//
// File: DataFlow.h
// Authors:
//   Francois Gindraud (2017)
// Created: 2017-04-18
// Last modified: 2017-04-18
//

/*
  Copyright or © or Copr. Bio++ Development Team, (November 16, 2004)

  This software is a computer program whose purpose is to provide classes
  for phylogenetic data analysis.

  This software is governed by the CeCILL license under French law and
  abiding by the rules of distribution of free software. You can use,
  modify and/ or redistribute the software under the terms of the CeCILL
  license as circulated by CEA, CNRS and INRIA at the following URL
  "http://www.cecill.info".

  As a counterpart to the access to the source code and rights to copy,
  modify and redistribute granted by the license, users are provided only
  with a limited warranty and the software's author, the holder of the
  economic rights, and the successive licensors have only limited
  liability.

  In this respect, the user's attention is drawn to the risks associated
  with loading, using, modifying and/or developing or reproducing the
  software by the user in light of its specific status of free software,
  that may mean that it is complicated to manipulate, and that also
  therefore means that it is reserved for developers and experienced
  professionals having in-depth computer knowledge. Users are therefore
  encouraged to load and test the software's suitability as regards their
  requirements in conditions enabling the security of their systems and/or
  data to be ensured and, more generally, to use and operate it in the
  same conditions as regards security.

  The fact that you are presently reading this means that you have had
  knowledge of the CeCILL license and that you accept its terms.
*/

#ifndef BPP_DATAFLOW_H
#define BPP_DATAFLOW_H

#include <cassert>
#include <cstddef>
#include <memory>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

#include <map> // TODO rm ?

/** @file Defines the basic types of data flow nodes.
 */
namespace bpp {
/// Debug: return a readable name for a C++ type descriptor (from typeid operator).
std::string prettyTypeName (const std::type_info & type_info);

namespace dataflow {
	class Node;
	template <typename T> class Value;
	class Context;

	/// Node instances are always manipulated as shared pointers: provide a short alias.
	using NodeRef = std::shared_ptr<Node>;

	/// Alias for a dependency vector (of NodeRef).
	using NodeRefVec = std::vector<NodeRef>;

	/// Shared pointer alias for Value<T>.
	template <typename T> using ValueRef = std::shared_ptr<Value<T>>;

	/// Numerical properties for DF Node.
	enum class NumericalProperty {
		Constant,         // Is a constant value
		ConstantZero,     // Is a zero value for its type
		ConstantOne,      // Is a one value for its type
		ConstantIdentity, // Is identity (for matrices mostly ; double(1.) is also considered identity).
	};

	/// @name Error functions (generate a message and throw exceptions).
	///@{
	[[noreturn]] void failureComputeWasCalled (const std::type_info & nodeType);
	[[noreturn]] void failureNodeConversion (const std::type_info & handleType, const Node & node);
	[[noreturn]] void failureDependencyNumberMismatch (const std::type_info & contextNodeType,
	                                                   std::size_t expectedSize,
	                                                   std::size_t givenSize);
	[[noreturn]] void failureEmptyDependency (const std::type_info & contextNodeType,
	                                          std::size_t depIndex);
	[[noreturn]] void failureDependencyTypeMismatch (const std::type_info & contextNodeType,
	                                                 std::size_t depIndex,
	                                                 const std::type_info & expectedType,
	                                                 const std::type_info & givenNodeType);
	///@}

	/** @brief Base dataflow Node class.
	 *
	 * All data flow nodes inherit from this class.
	 * Instances of this class must be used through std::shared_ptr<T>.
	 *
	 * A base node represents something that can be computed from its dependencies.
	 * The computation is defined by the compute() method in derived classes.
	 *
	 * The base class stores a boolean indicating if the current value is valid.
	 * This boolean is used to cache intermediate results in the graph, and avoid recomputings.
	 * When data flow graph leaves change, they invalidate (transitively) dependent nodes.
	 * This means setting the valid flag to false, to trigger a recomputation at next access.
	 *
	 * Each node has dependencies: other nodes providing data it needs to compute.
	 * These dependencies are stored as NodeRef (shared_ptr<Node>).
	 * Thus, multiple nodes can depend on the same value, to shared intermediate results.
	 * The list of dependencies is FIXED at creation, and cannot be changed.
	 * Changing the meaning of a node (dependencies, config) is done by creating a different node.
	 *
	 * Nodes also store pointers to dependent nodes.
	 * Dependent nodes are nodes which have a direct dependency to the current node.
	 * This is used to propagate invalidations to all transitive dependent nodes of a leaf.
	 * Dependent nodes are stored as raw pointers (no ownership).
	 *
	 * Base nodes thus store the data flow graph topology, in terms of pointer to base Node.
	 * This is useful for code reuse: no templates in topology walks code.
	 * It is useful for debugging: can print the whole graph structure.
	 * Accessing the derived classes (for compute) can be done efficiently with static_cast.
	 *
	 * Two main invariants are maintained at all times by this class.
	 * 1: If a node is valid, all its transitive dependencies are valid.
	 * 2: If a node is invalid, all transitively dependent nodes are invalid.
	 *
	 * In addition, two properties are implemented for performance:
	 * 1: During computation of a node dependencies, do not recompute valid nodes.
	 * 2: Only invalidate transitively dependent node, not others (still valid).
	 *
	 * Specific features are present in the base class as virtual functions.
	 * This include derivation (numerical values), debug, etc.
	 * These features have no-op of failure defaults which can be overriden in derived classes.
	 */
	class Node : public std::enable_shared_from_this<Node> {
	public:
		Node () = default;
		Node (const Node &) = delete;
		Node (Node &&) = delete;
		Node & operator= (const Node &) = delete;
		Node & operator= (Node &&) = delete;
		virtual ~Node (); // Deregisters from dependencies

		// Sets dependencies + register
		Node (const NodeRefVec & dependenciesArg);
		Node (NodeRefVec && dependenciesArg);

		// Accessors
		bool isValid () const noexcept { return isValid_; }
		const NodeRefVec & dependencies () const noexcept { return dependencyNodes_; }
		const std::vector<Node *> & dependentNodes () const noexcept { return dependentNodes_; }
		std::size_t nbDependencies () const noexcept { return dependencyNodes_.size (); }
		const NodeRef & dependency (std::size_t i) const noexcept { return dependencyNodes_[i]; }

		/// Node pretty name (default = type name).
		virtual std::string description () const;

		/// Node debug info (default = ""): user defined detailed info for DF graph debug.
		virtual std::string debugInfo () const;

		/** @brief Test if the node has the given numerical property.
		 *
		 * This is an optional indication only, used for optimisations.
		 * If unsure, leave it to always false (the default implementation).
		 * This should be non recursive, to ensure a constant time check.
		 */
		virtual bool hasNumericalProperty (NumericalProperty prop) const;

		/** @brief Returns a node computing d(this_node_expression)/d(node_expression).
		 *
		 * The expression represented by 'node' is considered as a variable.
		 * Event if 'node' is a constant value node, d(node)/d(node) == 1.
		 * The derivatiive of a matrix is the matrix of the derivatives.
		 * Derivation is undefined by default, and this function will throw en exception.
		 * Implementations will usually recursively derive sub-expressions and combine them.
		 */
		virtual NodeRef derive (Context & c, const Node & node);

		/// Check if derivation is transitively defined (FIXME semantics).
		virtual bool isDerivable (const Node & node) const;

		// Rebuild the node with different dependencies
		virtual NodeRef rebuild (NodeRefVec && deps) const;

		/** @brief Compute this node value, recomputing dependencies (transitively) as needed.
		 *
		 * Not thread safe !
		 */
		void computeRecursively ();

	protected:
		/** @brief Computation implementation.
		 *
		 * This functions is defined by derived classes.
		 * It should compute the new node value from dependency node values.
		 * When called, dependency node are guaranteed to have valid values.
		 *
		 * This function is private to prevent use for invalid dependencies.
		 * Higher level functions like computeRecursively() call it while ensuring dependency validity.
		 *
		 * Compute has access to dependencies as a NodeRefVec (base Node classes only).
		 * The recommended usage is to check dependency types at Node construction.
		 * Then use static_cast to access derived classes efficiently from the NodeRefVec.
		 * Several helper functions in DataFlowInternal.h simplify this.
		 * See DataFlowNumeric.h for examples.
		 */
		virtual void compute () = 0;

		/** @brief Invalidate (transitively) dependent nodes from this one.
		 *
		 * Not thread safe !
		 */
		void invalidateRecursively () noexcept;

		void makeInvalid () noexcept { isValid_ = false; }
		void makeValid () noexcept { isValid_ = true; }

	private:
		void registerNode (Node * n);
		void unregisterNode (const Node * n);

		NodeRefVec dependencyNodes_{};         // Nodes that we depend on.
		std::vector<Node *> dependentNodes_{}; // Nodes that depend on us.
		bool isValid_{false};
	};

	/// Convert a node ref with type check.
	template <typename T, typename U>
	std::shared_ptr<T> convertRef (const std::shared_ptr<U> & from) {
		auto p = std::dynamic_pointer_cast<T> (from);
		if (!p)
			failureNodeConversion (typeid (T), *from);
		return p;
	}

	/// Flags to control dot output of dataflow graphs.
	enum class DotOptions {
		None = 0,
		DetailedNodeInfo = 1 << 0,
		FollowUpwardLinks = 1 << 1,
		ShowDependencyIndex = 1 << 2,
	};
	DotOptions operator| (DotOptions a, DotOptions b);
	bool operator& (DotOptions a, DotOptions b);

	/// Write dataflow graph starting at nodes to output stream.
	void writeGraphToDot (std::ostream & os, const std::vector<const Node *> & nodes, DotOptions opt);

	/// Write dataflow graph starting at nodes to file at filename (shortcut).
	void writeGraphToDot (const std::string & filename, const std::vector<const Node *> & nodes,
	                      DotOptions opt);

	/* Free functions.
	 */
	NodeRef rebuildWithSubstitution (const NodeRef & node,
	                                 const std::map<const Node *, NodeRef> & substitutions);

	bool isTransitivelyDependentOn (const Node & searchedDependency, const Node & node);

	/** Context for dataflow node construction.
	 *
	 * TODO later, use to implement Cache/merging strategies
	 */
	class Context {
	public:
		Context () = default;
	};

	/** @brief Abstract Node storing a value of type T.
	 *
	 * Represents a DataFlow node containing a T value, but still abstract (no compute()).
	 * This intermediate class is used everywhere to indicate a typed dependency to a T value.
	 *
	 * For performance, the T value is stored directly in the node.
	 * Access to the value are made without any virtual call.
	 * Forward declared types cannot be used easily due to that.
	 * For documentation on how to manipulate Eigen forward declared types, see LinearAlgebraFwd.h.
	 *
	 * Access can be raw: no recomputations are done, the valid flag should be checked beforehand.
	 * getValue provides an access with recomputation if needed.
	 *
	 * The Value<T> constructor forwards dependencies to the base Node, and other arguments to the
	 * T value constructor.
	 *
	 * Value<T> stores a target dimension property, only useful for types which have Dimensions.
	 * It is used to document what size should the result have.
	 * It starts with a default constructed value.
	 */
	template <typename T> class Value : public Node {
	public:
		// Init deps
		template <typename... Args>
		Value (const NodeRefVec & deps, Args &&... args)
		    : Node (deps), value_ (std::forward<Args> (args)...) {}
		template <typename... Args>
		Value (NodeRefVec && deps, Args &&... args)
		    : Node (std::move (deps)), value_ (std::forward<Args> (args)...) {}

		/** @brief Access value, recompute if needed.
		 *
		 * Recompute the value if it is not up to date.
		 * Then access it as const.
		 * Recomputation is single threaded and not thread safe.
		 */
		const T & getValue () {
			this->computeRecursively ();
			return accessValueConst ();
		}

		/** @brief Raw value access (const).
		 *
		 * Value is not guaranteed to be valid (no recomputation).
		 */
		const T & accessValueConst () const noexcept { return value_; }

		/// Derive and cast result as Value<T> (most nodes derive to the same value type).
		ValueRef<T> deriveAsValue (Context & c, const Node & node) {
			return convertRef<Value<T>> (this->derive (c, node));
		}

	protected:
		/// Raw value access (mutable). Should only be used by subclasses to implement compute().
		T & accessValueMutable () noexcept { return value_; }

	private:
		T value_;
	};

	/// Access value of Node as a Value<T> (unchecked cast).
	template <typename T> const T & accessValueConstCast (const Node & node) {
		assert (dynamic_cast<const Value<T> *> (&node) != nullptr);      // Check type in debug mode
		return static_cast<const Value<T> &> (node).accessValueConst (); // Fast cast access
	}

	/// @name Basic dependency check primitives
	///@{
	/// Checks the size of a dependency vector, throws if mismatch.
	void checkDependencyVectorSize (const std::type_info & contextNodeType, const NodeRefVec & deps,
	                                std::size_t expectedSize);

	/// Checks that all dependencies are not null, throws if not.
	void checkDependenciesNotNull (const std::type_info & contextNodeType, const NodeRefVec & deps);

	/// Checks that deps[index] is a T node, throws if not.
	template <typename T>
	void checkNthDependencyIs (const std::type_info & contextNodeType, const NodeRefVec & deps,
	                           std::size_t index) {
		const auto & dep = *deps[index];
		if (dynamic_cast<const T *> (&dep) == nullptr) {
			failureDependencyTypeMismatch (contextNodeType, index, typeid (T), typeid (dep));
		}
	}

	/// Checks that deps[index] is a Value<T> node, throws if not.
	template <typename T>
	void checkNthDependencyIsValue (const std::type_info & contextNodeType, const NodeRefVec & deps,
	                                std::size_t index) {
		checkNthDependencyIs<Value<T>> (contextNodeType, deps, index);
	}

	/// Check that deps[start, end[ contains Value<T> nodes, throws if not
	template <typename T>
	void checkDependencyRangeIsValue (const std::type_info & contextNodeType, const NodeRefVec & deps,
	                                  std::size_t start, std::size_t end) {
		for (std::size_t i = start; i < end; ++i) {
			checkNthDependencyIsValue<T> (contextNodeType, deps, i);
		}
	}
	///@}
} // namespace dataflow
} // namespace bpp

#endif // BPP_NEWPHYL_DATAFLOW_H