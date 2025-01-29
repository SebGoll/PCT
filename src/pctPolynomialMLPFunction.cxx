/*=========================================================================
 *
 *  Copyright RTK Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "pctPolynomialMLPFunction.h"

namespace pct
{

PolynomialMLPFunction ::PolynomialMLPFunction()
{
  // We operate a change of origin, u0 is always 0
  m_u0 = 0.;
  m_CanBeVectorised = true;
}

PolynomialMLPFunction ::PolynomialMLPFunction(const int polydeg)
{
  // We operate a change of origin, u0 is always 0
  PolynomialMLPFunction();
  m_PolynomialDegree = polydeg;
}

void
PolynomialMLPFunction ::SetPolynomialDegree(const int polydeg)
{
  m_PolynomialDegree = polydeg;
  m_PolynomialDegreePlusThree = m_PolynomialDegree + 3;

  switch (polydeg)
  {
    case 0:
      m_bm.reserve(Functor::PolynomialMLP::bm_0.size());
      std::copy(Functor::PolynomialMLP::bm_0.begin(), Functor::PolynomialMLP::bm_0.end(), std::back_inserter(m_bm));
      break;
    case 1:
      m_bm.reserve(Functor::PolynomialMLP::bm_1.size());
      std::copy(Functor::PolynomialMLP::bm_1.begin(), Functor::PolynomialMLP::bm_1.end(), std::back_inserter(m_bm));
      break;
    case 2:
      m_bm.reserve(Functor::PolynomialMLP::bm_2.size());
      std::copy(Functor::PolynomialMLP::bm_2.begin(), Functor::PolynomialMLP::bm_2.end(), std::back_inserter(m_bm));
      break;
    case 3:
      m_bm.reserve(Functor::PolynomialMLP::bm_3.size());
      std::copy(Functor::PolynomialMLP::bm_3.begin(), Functor::PolynomialMLP::bm_3.end(), std::back_inserter(m_bm));
      break;
    case 4:
      m_bm.reserve(Functor::PolynomialMLP::bm_4.size());
      std::copy(Functor::PolynomialMLP::bm_4.begin(), Functor::PolynomialMLP::bm_4.end(), std::back_inserter(m_bm));
      break;
    case 5:
      m_bm.reserve(Functor::PolynomialMLP::bm_5.size());
      std::copy(Functor::PolynomialMLP::bm_5.begin(), Functor::PolynomialMLP::bm_5.end(), std::back_inserter(m_bm));
      break;
    default:
      itkWarningMacro(<< "Allowed values for polydeg are 0-5. Received " << m_PolynomialDegree
                      << ". Using default (5).");
      std::copy(Functor::PolynomialMLP::bm_5.begin(), Functor::PolynomialMLP::bm_5.end(), std::back_inserter(m_bm));
      break;
  }
}

void
PolynomialMLPFunction ::Init(const VectorType posIn,
                             const VectorType posOut,
                             const VectorType dirIn,
                             const VectorType dirOut)
{
  m_uOrigin = posIn[2];
  m_u2 = posOut[2] - m_uOrigin;

  // Parameters vectors
  m_x0[0] = posIn[0];
#if ITK_VERSION_MAJOR <= 4
  m_x0[1] = vcl_atan(dirIn[0]); // dirIn[2] is implicitely 1.
#else
  m_x0[1] = std::atan(dirIn[0]); // dirIn[2] is implicitely 1.
#endif
  m_x2[0] = posOut[0];
#if ITK_VERSION_MAJOR <= 4
  m_x2[1] = vcl_atan(dirOut[0]); // dirOut[2] is implicitely 1.
#else
  m_x2[1] = std::atan(dirOut[0]); // dirOut[2] is implicitely 1.
#endif

  m_y0[0] = posIn[1];
#if ITK_VERSION_MAJOR <= 4
  m_y0[1] = vcl_atan(dirIn[1]); // dirIn[2] is implicitely 1.
#else
  m_y0[1] = std::atan(dirIn[1]); // dirIn[2] is implicitely 1.
#endif
  m_y2[0] = posOut[1];
#if ITK_VERSION_MAJOR <= 4
  m_y2[1] = vcl_atan(dirOut[1]); // dirOut[2] is implicitely 1.
#else
  m_y2[1] = std::atan(dirOut[1]); // dirOut[2] is implicitely 1.
#endif

  const double A = Functor::PolynomialMLP::FactorsABCD::GetA(m_u2, m_bm);
  const double B = Functor::PolynomialMLP::FactorsABCD::GetB(m_u2, m_bm);
  const double C = Functor::PolynomialMLP::FactorsABCD::GetC(m_u2, m_bm);
  const double D = Functor::PolynomialMLP::FactorsABCD::GetD(m_u2, m_bm);

  Functor::PolynomialMLP::CoefficientsC::GetValue(m_c_x, m_u2, m_x0, m_x2, A, B, C, D);
  Functor::PolynomialMLP::CoefficientsC::GetValue(m_c_y, m_u2, m_y0, m_y2, A, B, C, D);

  m_dm_x[0] = m_x0[0];
  m_dm_x[1] = m_x0[1];
  m_dm_x[2] = m_c_x[0] * m_bm[0] / 2;
  for (int i = 3; i != m_PolynomialDegree + 3; i++)
  {
    m_dm_x[i] = (m_c_x[0] * m_bm[i - 2] + m_c_x[1] * m_bm[i - 3]) / i / (i - 1);
  }
  m_dm_x[m_PolynomialDegree + 3] =
    m_c_x[1] * m_bm[m_PolynomialDegree] / (m_PolynomialDegree + 2) / (m_PolynomialDegree + 3);

  m_dm_y[0] = m_y0[0];
  m_dm_y[1] = m_y0[1];
  m_dm_y[2] = m_c_y[0] * m_bm[0] / 2;
  for (int i = 3; i != m_PolynomialDegree + 3; i++)
  {
    m_dm_y[i] = (m_c_y[0] * m_bm[i - 2] + m_c_y[1] * m_bm[i - 3]) / i / (i - 1);
  }
  m_dm_y[m_PolynomialDegree + 3] =
    m_c_y[1] * m_bm[m_PolynomialDegree] / (m_PolynomialDegree + 2) / (m_PolynomialDegree + 3);
}

// vectorised version
void
PolynomialMLPFunction ::Evaluate(std::vector<double> u, std::vector<double> & x, std::vector<double> & y)
{
  for (auto & element : u)
    element -= m_uOrigin;

  std::fill(x.begin(), x.end(), 0.);
  std::fill(y.begin(), y.end(), 0.);

#ifdef MLP_TIMING
  m_EvaluateProbe1.Start();
#endif

  for (int i = 0; i != m_PolynomialDegreePlusThree; i++)
  {
    for (auto & element : x)
      element += m_dm_x[m_PolynomialDegreePlusThree - i];
    std::transform(x.begin(), x.end(), u.begin(), x.begin(), std::multiplies<double>());

    for (auto & element : y)
      element += m_dm_y[m_PolynomialDegreePlusThree - i];
    std::transform(y.begin(), y.end(), u.begin(), y.begin(), std::multiplies<double>());
  }

  for (auto & element : x)
    element += m_dm_x[0];
  for (auto & element : y)
    element += m_dm_y[0];


#ifdef MLP_TIMING
  m_EvaluateProbe1.Stop();
#endif
}


void
PolynomialMLPFunction ::EvaluateError(const double u, itk::Matrix<double, 2, 2> & error)
{
  itkGenericExceptionMacro("The method PolynomialMLPFunction::EvaluateError is not implemented at the moment");
}

#ifdef MLP_TIMING
void
PolynomialMLPFunction ::PrintTiming(std::ostream & os)
{
  os << "PolynomialMLPFunction timing:" << std::endl;
  os << "  EvaluateProbe1: " << m_EvaluateProbe1.GetTotal() << ' ' << m_EvaluateProbe1.GetUnit() << std::endl;
  // os << "  EvaluateProbe2: " << m_EvaluateProbe2.GetTotal()
  //    << ' ' << m_EvaluateProbe2.GetUnit() << std::endl;
}
#endif


} // namespace pct
