//
// File: YpR.cpp
// Created by: Laurent Gueguen
// Created on: Thu August 2 2007
//

/*
   Copyright or � or Copr. CNRS, (November 16, 2004)
   This software is a computer program whose purpose is to provide
   classes for phylogenetic data analysis.

   This software is governed by the CeCILL license under French law and
   abiding by the rules of distribution of free software. You can use,
   modify and/ or redistribute the software under the terms of the CeCILL
   license as circulated by CEA, CNRS and INRIA at the following URL
   "http://www.cecill.info".

   As a counterpart to the access to the source code and rights to copy,
   modify and redistribute granted by the license, users are provided
   only with a limited warranty and the software's author, the holder of
   the economic rights, and the successive licensors have only limited
   liability.

   In this respect, the user's attention is drawn to the risks associated
   with loading, using, modifying and/or developing or reproducing the
   software by the user in light of its specific status of free software,
   that may mean that it is complicated to manipulate, and that also
   therefore means that it is reserved for developers and experienced
   professionals having in-depth computer knowledge. Users are therefore
   encouraged to load and test the software's suitability as regards
   their requirements in conditions enabling the security of their
   systems and/or data to be ensured and, more generally, to use and
   operate it in the same conditions as regards security.

   The fact that you are presently reading this means that you have had
   knowledge of the CeCILL license and that you accept its terms.
 */

#include "YpR.h"

// From the STL:
#include <cmath>

using namespace bpp;

#include <Bpp/Numeric/Matrix/MatrixTools.h>
#include <Bpp/Numeric/VectorTools.h>
#include <Bpp/Numeric/Matrix/EigenValue.h>

/******************************************************************************/

YpR::YpR(const RNY* alph, SubstitutionModel* const pm, const std::string& prefix) :
  AbstractParameterAliasable(prefix),
  AbstractSubstitutionModel(alph,prefix),
  _pmodel(pm->clone()),
  _nestedPrefix(pm->getNamespace())
{
   _pmodel->setNamespace(prefix + _nestedPrefix);
  addParameters_(_pmodel->getParameters());
}

YpR::YpR(const YpR& ypr, const std::string& prefix) :
  AbstractParameterAliasable(ypr),
  AbstractSubstitutionModel(ypr),
  _pmodel(ypr._pmodel->clone()),
  _nestedPrefix(ypr.getNestedPrefix())

{
   _pmodel->setNamespace(prefix + _nestedPrefix);
}

YpR::YpR(const YpR& ypr) :
  AbstractParameterAliasable(ypr),
  AbstractSubstitutionModel(ypr),
  _pmodel(ypr._pmodel->clone()),
  _nestedPrefix(ypr.getNestedPrefix())
{}

void YpR::updateMatrices()
{
  updateMatrices(0,0,0,0,0,0,0,0);
}


void YpR::updateMatrices(double CgT, double cGA,
                         double TgC, double tGA,
                         double CaT, double cAG,
                         double TaC, double tAC)
{
  check_model(_pmodel);

  // Generator:
  const Alphabet* alph = _pmodel->getAlphabet();
  std::vector<int> l(4);

  l[0] = alph->charToInt("A");
  l[1] = alph->charToInt("G");
  l[2] = alph->charToInt("C");
  l[3] = alph->charToInt("T");

  unsigned int i,j,i1,i2,i3,j1,j2,j3;

  std::vector<double> a(4);  // a[A], a[G], a[C], a[T]
  std::vector<double> b(4);  // b[A], b[G], b[C], b[T]

  for (i = 0; i < 2; i++)
  {
    a[i] = _pmodel->Qij(l[1 - i],l[i]);
    b[i] = _pmodel->Qij(l[3 - i],l[i]);
    a[i + 2] = _pmodel->Qij(l[3 - i],l[i + 2]);
    b[i + 2] = _pmodel->Qij(l[1 - i],l[i + 2]);
  }

  // M_1
  RowMatrix<double> M1(3,3);

  M1(0,0) = 0;
  M1(0,1) = b[2];
  M1(0,2) = b[3];
  M1(1,0) = b[0] + b[1];
  M1(1,1) = 0;
  M1(1,2) = a[3];
  M1(2,0) = b[0] + b[1];
  M1(2,1) = a[2];
  M1(2,2) = 0;

  // M_2
  RowMatrix<double> M2(4,4);

  M2(0,0) = 0;
  M2(0,1) = a[1];
  M2(0,2) = b[2];
  M2(0,3) = b[3];
  M2(1,0) = a[0];
  M2(1,1) = 0;
  M2(1,2) = b[2];
  M2(1,3) = b[3];
  M2(2,0) = b[0];
  M2(2,1) = b[1];
  M2(2,2) = 0;
  M2(2,3) = a[3];
  M2(3,0) = b[0];
  M2(3,1) = b[1];
  M2(3,2) = a[2];
  M2(3,3) = 0;

  // M_3
  RowMatrix<double> M3(3,3);

  M3(0,0) = 0;
  M3(0,1) = a[1];
  M3(0,2) = b[2] + b[3];
  M3(1,0) = a[0];
  M3(1,1) = 0;
  M3(1,2) = b[2] + b[3];
  M3(2,0) = b[0];
  M3(2,1) = b[1];
  M3(2,2) = 0;


  for (i1 = 0; i1 < 3; i1++)
  {
    for (i2 = 0; i2 < 4; i2++)
    {
      for (i3 = 0; i3 < 3; i3++)
      {
        i = 12 * i1 + 3 * i2 + i3;
        for (j1 = 0; j1 < 3; j1++)
        {
          for (j2 = 0; j2 < 4; j2++)
          {
            for (j3 = 0; j3 < 3; j3++)
            {
              j = 12 * j1 + 3 * j2 + j3;
              if ((i1 == j1) && (i2 == j2))
                generator_(i,j) = M3(i3,j3);
              else if ((i1 == j1) && (i3 == j3))
                generator_(i,j) = M2(i2,j2);
              else if ((i2 == j2) && (i3 == j3))
                generator_(i,j) = M1(i1,j1);
              else
                generator_(i,j) = 0;
            }
          }
        }
      }
    }
  }

  // Introduction des dependances

  for (i3 = 0; i3 < 3; i3++)
  {
    generator_(15 + i3,12 + i3) += cGA * a[0]; // CG -> CA
    generator_(12 * i3 + 7,12 * i3 + 6) += cGA * a[0];

    generator_(15 + i3,27 + i3) += CgT * a[3]; // CG -> TG
    generator_(12 * i3 + 7,12 * i3 + 10) += CgT * a[3];

    generator_(27 + i3,24 + i3) += tGA * a[0]; // TG -> TA
    generator_(12 * i3 + 10,12 * i3 + 9) += tGA * a[0];

    generator_(27 + i3,15 + i3) += TgC * a[2]; // TG -> CG
    generator_(12 * i3 + 10,12 * i3 + 7) += TgC * a[2];

    generator_(12 + i3,24 + i3) += CaT * a[3]; // CA -> TA
    generator_(12 * i3 + 6,12 * i3 + 9) += CaT * a[3];

    generator_(12 + i3,15 + i3) += cAG * a[1]; // CA -> CG
    generator_(12 * i3 + 6,12 * i3 + 7) += cAG * a[1];

    generator_(24 + i3,27 + i3) += tAC * a[1]; // TA -> TG
    generator_(12 * i3 + 9,12 * i3 + 10) += tAC * a[1];

    generator_(24 + i3,12 + i3) += TaC * a[2]; // TA -> CA
    generator_(12 * i3 + 9,12 * i3 + 6) += TaC * a[2];
  }

  double x;

  for (i = 0; i < 36; i++)
  {
    x = 0;
    for (j = 0; j < 36; j++)
    {
      if (j != i)
        x += generator_(i,j);
    }
    generator_(i,i) = -x;
  }

  // calcul spectral

  EigenValue<double> ev(generator_);
  eigenValues_ = ev.getRealEigenValues();
  
  rightEigenVectors_ = ev.getV();
  MatrixTools::inv(rightEigenVectors_,leftEigenVectors_);

  std::vector<double> vi = ev.getImagEigenValues();

  // frequence stationnaire

  x = 0;
  j = 0;
  while (j < 36)
  {
    if (abs(eigenValues_[j]) < 0.000001 && abs(vi[j]) < 0.000001)
    {
      eigenValues_[j]=0; //to avoid approximation problems in the future
      for (i = 0; i < 36; i++)
      {
        freq_[i] = leftEigenVectors_(j,i);
        x += freq_[i];
      }
      break;
    }
    j++;
  }

  for (i = 0; i < 36; i++)
  {
    freq_[i] /= x;
  }

  // mise a l'echelle

  x = 0;
  for (i1 = 0; i1 < 3; i1++)
  {
    for (i2 = 0; i2 < 4; i2++)
    {
      for (i3 = 0; i3 < 3; i3++)
      {
        i = 12 * i1 + 3 * i2 + i3;
        for (j2 = 0; j2 < 4; j2++)
        {
          if (j2 != i2)
          {
            j = 12 * i1 + 3 * j2 + i3;
            x += freq_[i] * generator_(i,j);
          }
        }
      }
    }
  }

  MatrixTools::scale(generator_,1 / x);

  for (i = 0; i < 36; i++)
  {
    eigenValues_[i] /= x;
  }
}


void YpR::check_model(SubstitutionModel* const pm) const
throw (Exception)
{
  if (!pm)
    throw Exception("No Model ");

  const Alphabet* alph = pm->getAlphabet();
  if (alph->getAlphabetType() != "DNA alphabet")
    throw Exception("Need a DNA model");

  std::vector<int> l(4);

  l[0] = alph->charToInt("A");
  l[1] = alph->charToInt("G");
  l[2] = alph->charToInt("C");
  l[3] = alph->charToInt("T");

  // Check that the model is good for YpR

  int i;

  for (i = 0; i < 2; i++)
  {
    if (pm->Qij(l[2],l[i]) != pm->Qij(l[3],l[i]))
      throw Exception("Not R/Y Model " + pm->getName());
  }
  for (i = 2; i < 4; i++)
  {
    if (pm->Qij(l[0],l[i]) != pm->Qij(l[1],l[i]))
      throw Exception("Not R/Y Model " + pm->getName());
  }
}

void YpR::setNamespace(const std::string& prefix)
{
   AbstractSubstitutionModel::setNamespace(prefix);
  // We also need to update the namespace of the nested model:
  _pmodel->setNamespace(prefix + _nestedPrefix);
}

// ///////////////////////////////////////////////
// ///////////////////////////////////////////////


/******************************************************************************/

YpR_Sym::YpR_Sym(const RNY* alph,
                 SubstitutionModel* const pm,
                 double CgT, double TgC,
                 double CaT, double TaC) : AbstractParameterAliasable("YpR_Sym."), YpR(alph, pm,"YpR_Sym.")
{
  addParameter_(Parameter("YpR_Sym.rCgT", CgT, &Parameter::R_PLUS));
  addParameter_(Parameter("YpR_Sym.rTgC", TgC, &Parameter::R_PLUS));
  addParameter_(Parameter("YpR_Sym.rCaT", CaT, &Parameter::R_PLUS));
  addParameter_(Parameter("YpR_Sym.rTaC", TaC, &Parameter::R_PLUS));

  updateMatrices();
}

void YpR_Sym::updateMatrices()
{
   double rCgT = getParameterValue("rCgT");
   double rTgC = getParameterValue("rTgC");
   double rCaT = getParameterValue("rCaT");
   double rTaC = getParameterValue("rTaC");

   YpR::updateMatrices(rCgT, rCgT, rTgC, rTgC, rCaT, rCaT, rTaC, rTaC);
}

YpR_Sym::YpR_Sym(const YpR_Sym& ypr) : AbstractParameterAliasable(ypr), YpR(ypr,"YpR_Sym.")
{}

/******************************************************************************/

std::string YpR_Sym::getName() const
{
  return "YpR_Sym " + _pmodel->getName();
}


// ///////////////////////////////////////////////
// ///////////////////////////////////////////////

/******************************************************************************/

YpR_Gen::YpR_Gen(const RNY* alph,
                 SubstitutionModel* const pm,
                 double CgT, double cGA,
                 double TgC, double tGA,
                 double CaT, double cAG,
                 double TaC, double tAG) : AbstractParameterAliasable("YpR_Gen."), YpR(alph, pm,"YpR_Gen.")
{
  addParameter_(Parameter("YpR_Gen.rCgT", CgT, &Parameter::R_PLUS));
  addParameter_(Parameter("YpR_Gen.rcGA", cGA, &Parameter::R_PLUS));
  addParameter_(Parameter("YpR_Gen.rTgC", TgC, &Parameter::R_PLUS));
  addParameter_(Parameter("YpR_Gen.rtGA", tGA, &Parameter::R_PLUS));
  addParameter_(Parameter("YpR_Gen.rCaT", CaT, &Parameter::R_PLUS));
  addParameter_(Parameter("YpR_Gen.rcAG", cAG, &Parameter::R_PLUS));
  addParameter_(Parameter("YpR_Gen.rTaC", TaC, &Parameter::R_PLUS));
  addParameter_(Parameter("YpR_Gen.rtAG", tAG, &Parameter::R_PLUS));

  updateMatrices();
}

void YpR_Gen::updateMatrices()
{
   double rCgT = getParameterValue("rCgT");
   double rcGA = getParameterValue("rcGA");
   double rTgC = getParameterValue("rTgC");
   double rtGA = getParameterValue("rtGA");
   double rCaT = getParameterValue("rCaT");
   double rcAG = getParameterValue("rcAG");
   double rTaC = getParameterValue("rTaC");
   double rtAG = getParameterValue("rtAG");

   YpR::updateMatrices(rCgT, rcGA, rTgC, rtGA, rCaT, rcAG, rTaC, rtAG);
}

YpR_Gen::YpR_Gen(const YpR_Gen& ypr) : AbstractParameterAliasable(ypr), YpR(ypr,"YpR_Gen.")
{
  updateMatrices();
}

/******************************************************************************/

std::string YpR_Gen::getName() const
{
  return "YpR_Gen " + _pmodel->getName();
}

