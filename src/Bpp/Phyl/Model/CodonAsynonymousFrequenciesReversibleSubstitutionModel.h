//
// File: CodonAsynonymousFrequenciesReversibleSubstitutionModel.h
// Created by: Laurent Gueguen
// Created on: Tue Dec 24 11:03:53 2003
//

/*
   Copyright or © or Copr. CNRS, (November 16, 2004)

   This software is a computer program whose purpose is to provide classes
   for phylogenetic data analysis.

   This software is governed by the CeCILL  license under French law and
   abiding by the rules of distribution of free software.  You can  use,
   modify and/ or redistribute the software under the terms of the CeCILL
   license as circulated by CEA, CNRS and INRIA at the following URL
   "http://www.cecill.info".

   As a counterpart to the access to the source code and  rights to copy,
   modify and redistribute granted by the license, users are provided only
   with a limited warranty  and the software's author,  the holder of the
   economic rights,  and the successive licensors  have only  limited
   liability.

   In this respect, the user's attention is drawn to the risks associated
   with loading,  using,  modifying and/or developing or reproducing the
   software by the user in light of its specific status of free software,
   that may mean  that it is complicated to manipulate,  and  that  also
   therefore means  that it is reserved for developers  and  experienced
   professionals having in-depth computer knowledge. Users are therefore
   encouraged to load and test the software's suitability as regards their
   requirements in conditions enabling the security of their systems and/or
   data to be ensured and,  more generally, to use and operate it in the
   same conditions as regards security.

   The fact that you are presently reading this means that you have had
   knowledge of the CeCILL license and that you accept its terms.
 */

#ifndef _CODONASYNONYMOUSFREQUENCIESREVERSIBLESUBSTITUTIONMODEL_H_
#define _CODONASYNONYMOUSFREQUENCIESREVERSIBLESUBSTITUTIONMODEL_H_

#include "AbstractCodonFrequenciesReversibleSubstitutionModel.h"

// From SeqLib:
#include <Seq/CodonAlphabet.h>
#include <Seq/GeneticCode.h>
#include <Seq/AlphabetIndex2.h>

namespace bpp
{
/**
 * @brief Class for asynonymous substitution models on codons with
 * parameterized equilibrium frequencies and K80 basic model.
 * @author Laurent Guéguen
 *
 * Only substitutions with one letter changed are accepted. </p>
 *
 * The additional parameter to
 * AbstractCodonFrequenciesReversibleSubstitutionModel is the ratio of
 * nonsynonymous over synonymous substitutions.
 *
 * If a distance @f$d@f$ between amino-acids is defined, the ratio between
 * non-synonymous and synonymous substitutions rates is, if the codied
 * amino-acids are @f$x@f$ and @f$y@f$, @f$\beta*\exp(-\alpha.d(x,y))@f$ with
 * non-negative parameter @f$\alpha@f$ and positive parameter @f$\beta@f$.
 *
 * If such a distance is not defined, the ratio between non-synonymous
 * and synonymous substitutions rates is @f$\beta@f$ with positive
 * parameter @f$\beta@f$.
 */

class CodonAsynonymousFrequenciesReversibleSubstitutionModel :
  public AbstractCodonFrequenciesReversibleSubstitutionModel
{
private:
  const GeneticCode* geneticCode_;
  const AlphabetIndex2<double>* pdistance_;

public:
  /**
   * @brief Build a new CodonAsynonymousFrequenciesReversibleSubstitutionModel object
   * from three pointers to AbstractReversibleSubstitutionModels. NEW
   * AbstractReversibleSubstitutionModels are copied from the given ones.
   *
   * @param palph pointer to a GeneticCode
   * @param pfreq pointer to the FrequenciesSet* equilibrium frequencies
   * @param pdist optional pointer to the AlphabetIndex2<double> amino-acids distance object.
   */
  CodonAsynonymousFrequenciesReversibleSubstitutionModel(
    const GeneticCode* palph,
    FrequenciesSet* pfreq,
    const AlphabetIndex2<double>* pdist = 0) throw (Exception);

  CodonAsynonymousFrequenciesReversibleSubstitutionModel(
    const CodonAsynonymousFrequenciesReversibleSubstitutionModel& cfm) :
    AbstractCodonFrequenciesReversibleSubstitutionModel(cfm),
    geneticCode_(cfm.geneticCode_),
    pdistance_(cfm.pdistance_)
  {}

  CodonAsynonymousFrequenciesReversibleSubstitutionModel& operator=(
    const CodonAsynonymousFrequenciesReversibleSubstitutionModel& cfm)
  {
    AbstractCodonFrequenciesReversibleSubstitutionModel::operator=(cfm);
    geneticCode_ = cfm.geneticCode_;
    pdistance_   = cfm.pdistance_;
    return *this;
  }

  ~CodonAsynonymousFrequenciesReversibleSubstitutionModel() {}

  CodonAsynonymousFrequenciesReversibleSubstitutionModel* clone() const
  {
    return new CodonAsynonymousFrequenciesReversibleSubstitutionModel(*this);
  }

protected:
  void completeMatrices();

public:
  std::string getName() const;

  const GeneticCode* getGeneticCode() const { return geneticCode_; }
};

} // end of namespace bpp.

#endif
