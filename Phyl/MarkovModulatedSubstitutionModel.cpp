//
// File: MarkovModulatedSubstitutionModel.cpp
// Created by: Julien Dutheil
// Created on: Sat Aug 05 08:21 2006
//

/*
Copyright or � or Copr. CNRS, (November 16, 2004)

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

#include "MarkovModulatedSubstitutionModel.h"

//From NumCalc:
#include <NumCalc/MatrixTools.h>
#include <NumCalc/VectorTools.h>
using namespace VectorFunctions;
using namespace VectorOperators;
#include <NumCalc/EigenValue.h>

/******************************************************************************/
	
void MarkovModulatedSubstitutionModel::updateMatrices()
{
  //_ratesGenerator and _rates must be initialized!

  _nbStates        = _model->getNumberOfStates();
  _nbRates         = _rates.nCols();
  _ratesGenerator  = MatrixTools::mult(_ratesExchangeability, MatrixTools::diag< RowMatrix<double> >(_ratesFreq));
  
  _generator       = MatrixTools::kroneckerMult(_rates, _model->getGenerator());
  MatrixTools::add(_generator, MatrixTools::kroneckerMult(_ratesGenerator, MatrixTools::getId< RowMatrix<double> >(_nbStates))      );
  _exchangeability = MatrixTools::kroneckerMult(MatrixTools::mult(_rates, MatrixTools::diag< RowMatrix<double> >(1./_ratesFreq)),_model->getExchangeabilityMatrix());
  MatrixTools::add(_exchangeability, MatrixTools::kroneckerMult(_ratesExchangeability, MatrixTools::diag< RowMatrix<double> >(1/_model->getFrequencies())));
  _freq            = VectorFunctions::kroneckerMult(_ratesFreq, _model->getFrequencies());
	if(_normalizeRateChanges)
  {
    // Normalization:
	  double scale = -scalar(MatrixTools::diag<RowMatrix<double>, double>(_generator), _freq);
    MatrixTools::scale(_generator, 1./scale);

    // Normalize exchangeability matrix too:
	  MatrixTools::scale(_exchangeability, 1./scale);
  }

  // Compute eigen values and vectors:
  _eigenValues.resize(_nbRates*_nbStates);
  _rightEigenVectors.resize(_nbStates*_nbRates, _nbStates*_nbRates);
  
  vector<double>    modelEigenValues       = _model->getEigenValues();
  RowMatrix<double> modelRightEigenVectors = _model->getColumnRightEigenVectors();
  for(unsigned int i = 0; i < _nbStates; i++)
  {
    RowMatrix<double> tmp = _rates;
    MatrixTools::scale(tmp, modelEigenValues[i]);
    MatrixTools::add(tmp, _ratesGenerator);
    EigenValue<double> ev(tmp);
	  vector<double>    values  = ev.getRealEigenValues();
	  RowMatrix<double> vectors = ev.getV();
    for(unsigned int j = 0; j < _nbRates; j++)
    {
      unsigned int c = i*_nbRates+j; //Current eigen value index.
      _eigenValues[c] = values[j];
      // Compute the Kronecker product of the jth vector and the ith modelRightEigenVector.
      for(unsigned int ii = 0; ii < _nbRates; ii++)
      {
        double vii = vectors(ii, j);
        for(unsigned int jj = 0; jj < _nbStates; jj++)
        {
          _rightEigenVectors(ii*_nbStates+jj, c) = vii * modelRightEigenVectors(jj, i);
        }
      }
    }
  }
  // Now compute left eigen vectors by inversion: 
  _leftEigenVectors = MatrixTools::inv(_rightEigenVectors);
}

/******************************************************************************/

RowMatrix<double> MarkovModulatedSubstitutionModel::getPij_t(double t) const
{
	if(t==0) return MatrixTools::getId< RowMatrix<double> >(_nbStates * _nbRates);
  return MatrixTools::mult(_rightEigenVectors, exp(_eigenValues*t), _leftEigenVectors);
}

RowMatrix<double> MarkovModulatedSubstitutionModel::getdPij_dt(double t) const
{
	return MatrixTools::mult(_rightEigenVectors, _eigenValues * exp(_eigenValues*t), _leftEigenVectors);
}

RowMatrix<double> MarkovModulatedSubstitutionModel::getd2Pij_dt2(double t) const
{
	return MatrixTools::mult(_rightEigenVectors, sqr(_eigenValues) * exp(_eigenValues*t), _leftEigenVectors);
}

/******************************************************************************/

double MarkovModulatedSubstitutionModel::getInitValue(int i, int state) const throw (BadIntException)
{
	if(i < 0 || i > (int)(_nbStates*_nbRates)) throw BadIntException(i, "AbstractSubstitutionModel::getInitValue");
	if(state < 0 || !_model->getAlphabet()->isIntInAlphabet(state)) throw BadIntException(state, "MarkovModulatedSubstitutionModel::getInitValue. Character " + _model->getAlphabet()->intToChar(state) + " is not allowed in model.");
	vector<int> states = _model->getAlphabet()->getAlias(state);
  int x = i % _nbStates;
	for(unsigned int j = 0; j < states.size(); j++) if(x == states[j]) return 1.;
	return 0.;
}

/******************************************************************************/
