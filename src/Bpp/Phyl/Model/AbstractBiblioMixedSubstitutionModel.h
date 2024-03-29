//
// File: AbstractBiblioMixedSubstitutionModel.h
// Created by: Laurent Gueguen
// Created on: lundi 18 juillet 2011, à 15h 17
//

/*
  Copyright or © or Copr. Bio++ Development Team, (November 16, 2004)

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

#ifndef _ABSTRACTBIBLIOMIXEDSUBSTITUTIONMODEL_H_
#define _ABSTRACTBIBLIOMIXEDSUBSTITUTIONMODEL_H_

#include "AbstractBiblioSubstitutionModel.h"
#include "MixedSubstitutionModel.h"

namespace bpp
{
/**
 * @brief Abstract class for mixture models based on the bibliography.
 * @author Laurent Guéguen
 */

  class AbstractBiblioMixedSubstitutionModel :
	public virtual Clonable,
    public virtual MixedSubstitutionModel,
    public AbstractBiblioSubstitutionModel
  {
  protected:
    std::unique_ptr<MixedSubstitutionModel> pmixmodel_;
    
  public:
    AbstractBiblioMixedSubstitutionModel(const std::string& prefix);

    AbstractBiblioMixedSubstitutionModel(const AbstractBiblioMixedSubstitutionModel& model);

    AbstractBiblioMixedSubstitutionModel& operator=(const AbstractBiblioMixedSubstitutionModel& model);

    virtual ~AbstractBiblioMixedSubstitutionModel();

    virtual AbstractBiblioMixedSubstitutionModel* clone() const
    {
      return new AbstractBiblioMixedSubstitutionModel(*this);
    }
    

  public:
    /*
     *@brief Returns the submodel from the mixture.
     *
     */
  
    const SubstitutionModel* getNModel(size_t i) const
    {
      return getMixedModel().getNModel(i);
    }

    SubstitutionModel* getNModel(size_t i)
    {
      return getMixedModel().getNModel(i);
    }

    /**
     * @brief Returns the  probability of a specific model from the mixture
     */
    double getNProbability(size_t i) const
    {
      return getMixedModel().getNProbability(i);
    }

    /**
     * @brief Returns the vector of the probabilities of the
     * submodels of the mixture.
     *
     */

    const std::vector<double>& getProbabilities() const
    {
      return getMixedModel().getProbabilities();
    }

    /**
     * @brief Sets the probabilities of the submodels of the mixture.
     *
     */
    void setNProbability(size_t i, double prob)
    {
      getMixedModel().setNProbability(i, prob);
    }

    /**
     * @brief Returns the number of submodels
     *
     */
    size_t getNumberOfModels() const
    {
      return getMixedModel().getNumberOfModels();
    }

    /**
     * @brief sets the rates of the submodels.
     *
     **/
    void setVRates(const Vdouble& vd)
    {
      getMixedModel().setVRates(vd);
    }

    /**
     * @brief normalizes the rates of the submodels.
     *
     **/
    void normalizeVRates()
    {
      getMixedModel().normalizeVRates();
    }

    /**
     * @brief Returns the vector of all the rates of the mixture
     */

    const std::vector<double>& getVRates() const
    {
      return getMixedModel().getVRates();
    }

    /**
     * @brief Returns the rate of a specific model from the mixture
     */
    double getNRate(size_t i) const
    {
      return getMixedModel().getNRate(i);
    }

    /**
     * @brief retrieve a pointer to the submodel with the given name.
     *
     * Return Null if not found.
     *
     */
  
    const SubstitutionModel* getSubModelWithName(const std::string& name) const
    {
      return getMixedModel().getSubModelWithName(name);
    }
  
    /*
     *@brief Returns the vector of numbers of the submodels in the
     *mixture that match a description.
     *
     */
    Vint getSubmodelNumbers(const std::string& desc) const;

    const SubstitutionModel& getSubstitutionModel() const { return *pmixmodel_.get(); }

    const MixedSubstitutionModel& getMixedModel() const { return *pmixmodel_.get(); }
    
  protected:
    SubstitutionModel& getSubstitutionModel()
    {
      return *pmixmodel_.get();
    }
    
    MixedSubstitutionModel& getMixedModel() { return *pmixmodel_.get(); }

    const FrequenciesSet* getFrequenciesSet() const {return pmixmodel_->getNModel(1)->getFrequenciesSet();}
  };

} // end of namespace bpp.

#endif  // _AbstractBiblioMixedSubstitutionModel_H_

