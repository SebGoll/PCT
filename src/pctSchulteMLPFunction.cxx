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

#include "pctSchulteMLPFunction.h"

namespace pct
{

SchulteMLPFunction
::SchulteMLPFunction()
{
  // We apply a change of origin, u0 is always 0
  m_u0=0.;

  // Construct the constant part of R0 and R1 (equations 11 and 14)
  m_R0(0,0) = 1.;
  m_R0(1,0) = 0.;
  m_R0(1,1) = 1.;
  m_R1 = m_R0;

  // Transpose
  m_R0T = m_R0.GetTranspose();
  m_R1T = m_R1.GetTranspose();

  // Construct the constant part of Sin and Sout (Eq. 14 & 15 in Krah 2018, PMB)
  // Needed only when tracker uncertainties are considered
  m_Sin(0,0) = 1.;
  m_Sin(1,0) = 0.;
  m_Sin(1,1) = 1.;
  m_Sout = m_Sin;

}

// Initialize terms needed to include tracker uncertainties
void
SchulteMLPFunction
::InitUncertain(const VectorType posIn, const VectorType posOut, const VectorType dirIn, const VectorType dirOut,
                double dEntry, double dExit, double TrackerResolution, double TrackerPairSpacing, double MaterialBudget)
{
  m_considerTrackerUncertainties = true;  // NK: maybe this should actually go into constructor
  m_u2 = posOut[2]-posIn[2];
  const double sigmaPSq = TrackerResolution * TrackerResolution;
  // Finish constructing Sin and Sout matrices (Eq. 14 & 15 in Krah 2018, PMB)
  m_Sin(0,1) = dEntry;
  m_Sout(0,1) = dExit;

  // Transpose
  m_SinT = m_Sin.GetTranspose();
  m_SoutT = m_Sout.GetTranspose();

  m_Sout_Inv = m_Sout;
  InverseMatrix(m_Sout_Inv);
  m_SoutT_Inv = m_SoutT;
  InverseMatrix(m_SoutT_Inv);

  m_SigmaIn(0,0) = 1;
  m_SigmaIn(0,1) = 1 / TrackerPairSpacing;
  m_SigmaIn(1,0) = m_SigmaIn(0,1);
  m_SigmaIn(1,1) = 2 / TrackerPairSpacing / TrackerPairSpacing;
  m_SigmaIn *= sigmaPSq;

  m_SigmaOut(0,0) = m_SigmaIn(0,0);
  m_SigmaOut(0,1) = -m_SigmaIn(0,1);
  m_SigmaOut(1,0) = -m_SigmaIn(1,0);
  m_SigmaOut(1,1) = m_SigmaIn(1,1);
  m_SigmaOut *= sigmaPSq;

  const double c = 13.6*CLHEP::MeV * 13.6*CLHEP::MeV / (36.1*CLHEP::cm);
  const double trackerThickness = MaterialBudget * 36.1*CLHEP::cm;
  m_SigmaIn(1,1) += Functor::SchulteMLP::IntegralForSigmaSqTheta::GetValue(trackerThickness) * c;
  // m_SigmaOut(1,1) += Functor::SchulteMLP::IntegralForSigmaSqTheta::GetValue(trackerThickness) * c;
  m_SigmaOut(1,1) += (Functor::SchulteMLP::IntegralForSigmaSqTheta::GetValue(m_u2 + trackerThickness) - Functor::SchulteMLP::IntegralForSigmaSqTheta::GetValue(m_u2)) * c;

  SchulteMLPFunction::Init(posIn, posOut, dirIn, dirOut);

}

// standard part of the Initialization
void
SchulteMLPFunction
::Init(const VectorType posIn, const VectorType posOut, const VectorType dirIn, const VectorType dirOut)
{
  m_uOrigin = posIn[2];

  m_u2 = posOut[2]-m_uOrigin;
  m_IntForSigmaSqTheta2  = Functor::SchulteMLP::IntegralForSigmaSqTheta ::GetValue(m_u2);
  m_IntForSigmaSqTTheta2 = Functor::SchulteMLP::IntegralForSigmaSqTTheta::GetValue(m_u2);
  m_IntForSigmaSqT2      = Functor::SchulteMLP::IntegralForSigmaSqT     ::GetValue(m_u2);

  // Parameters vectors
  m_x0[0] = posIn[0];
  m_x0[1] = std::atan(dirIn[0]);  //dirIn[2] is implicitely 1.
  m_x2[0] = posOut[0];
  m_x2[1] = std::atan(dirOut[0]); //dirOut[2] is implicitely 1.

  m_y0[0] = posIn[1];
  m_y0[1] = std::atan(dirIn[1]);  //dirIn[2] is implicitely 1.
  m_y2[0] = posOut[1];
  m_y2[1] = std::atan(dirOut[1]); //dirOut[2] is implicitely 1.
}

void
SchulteMLPFunction
::Evaluate( const double u, double &x, double &y, double &dx, double &dy )
{
#ifdef MLP_TIMING
  m_EvaluateProbe1.Start();
#endif
  const double u1 = u-m_uOrigin;

  // Finish constructing rotation matrices (equations 11 and 14)
  m_R0(0,1) = u1;
  m_R1(0,1) = m_u2-u1;
  m_R0T(1,0) = m_R0(0,1);
  m_R1T(1,0) = m_R1(0,1);

  itk::Matrix<double, 2, 2> R1T_Inv(m_R1T);
  InverseMatrix(R1T_Inv);
  itk::Matrix<double, 2, 2> R1_Inv(m_R1);
  InverseMatrix(R1_Inv);

  // Constants used in both integrals
  const double intForSigmaSqTheta1  = Functor::SchulteMLP::IntegralForSigmaSqTheta ::GetValue(u1);
  const double intForSigmaSqTTheta1 = Functor::SchulteMLP::IntegralForSigmaSqTTheta::GetValue(u1);
  const double intForSigmaSqT1      = Functor::SchulteMLP::IntegralForSigmaSqT     ::GetValue(u1);

  // Construct Sigma1 (equations 6-9)
  m_Sigma1(1,1) = intForSigmaSqTheta1/* - m_IntForSigmaSqTheta0*/;
  m_Sigma1(0,1) = u1 * m_Sigma1(1,1) - intForSigmaSqTTheta1/* + m_IntForSigmaSqTTheta0*/;
  m_Sigma1(1,0) = m_Sigma1(0,1);
  m_Sigma1(0,0) = u1 * ( 2*m_Sigma1(0,1) - u1*m_Sigma1(1,1) ) + intForSigmaSqT1/* - m_IntForSigmaSqT0*/;
  m_Sigma1 *= Functor::SchulteMLP::ConstantPartOfIntegrals::GetValue(m_u0,u1);

  double sigma1 = std::sqrt(m_Sigma2(1,1));

  // Construct Sigma2 (equations 15-18)
  m_Sigma2(1,1) = m_IntForSigmaSqTheta2 - intForSigmaSqTheta1;
  m_Sigma2(0,1) = m_u2 * m_Sigma2(1,1) - m_IntForSigmaSqTTheta2 + intForSigmaSqTTheta1;
  m_Sigma2(1,0) = m_Sigma2(0,1);
  m_Sigma2(0,0) = m_u2 * ( 2*m_Sigma2(0,1) - m_u2*m_Sigma2(1,1) ) + m_IntForSigmaSqT2 - intForSigmaSqT1;
  m_Sigma2 *= Functor::SchulteMLP::ConstantPartOfIntegrals::GetValue(u1,m_u2);

#ifdef MLP_TIMING
  m_EvaluateProbe2.Start();
#endif

  itk::Vector<double, 2> xMLP;
  itk::Vector<double, 2> yMLP;

  if(m_considerTrackerUncertainties)
  {
    itk::Matrix<double, 2, 2>  C1 = m_R0 * m_Sin * m_SigmaIn * m_SinT * m_R0T + m_Sigma1;
    itk::Matrix<double, 2, 2>  C2 = R1_Inv * m_Sout_Inv * m_SigmaOut * m_SoutT_Inv * R1T_Inv + R1_Inv * m_Sigma2 * R1T_Inv;
    itk::Matrix<double, 2, 2> C1plusC2(C1 + C2);
    InverseMatrix(C1plusC2);
    itk::Matrix<double, 2, 2> factorIn(C2 * C1plusC2 * m_R0);
    itk::Matrix<double, 2, 2> factorOut(C1 * C1plusC2 * R1_Inv);

    xMLP = factorIn * m_x0 + factorOut * m_x2;
    yMLP = factorIn * m_y0 + factorOut * m_y2;
  }
  else
  {
    // This version here is better than the previously implemented one
    // because it avoids inverting the matrices Sigma.
    // See comment in [Krah 2018, PMB]
    itk::Matrix<double, 2, 2> sum1(R1_Inv * m_Sigma2 + m_Sigma1 * m_R1T);
    InverseMatrix(sum1);
    itk::Matrix<double, 2, 2> sum2(m_R1 * m_Sigma1 + m_Sigma2 * R1T_Inv);
    InverseMatrix(sum2);

    itk::Matrix<double, 2, 2> part1(R1_Inv * m_Sigma2 * sum1 * m_R0);
    itk::Matrix<double, 2, 2> part2(m_Sigma1 * sum2);

    xMLP = part1 * m_x0 + part2 * m_x2;
    yMLP = part1 * m_y0 + part2 * m_y2;
  }

  x = xMLP[0];
  dx = xMLP[1];
  y = yMLP[0];
  dy = yMLP[1];

#ifdef MLP_TIMING
  m_EvaluateProbe1.Stop();
  m_EvaluateProbe2.Stop();
#endif
}

void
SchulteMLPFunction
::EvaluateError( const double u, itk::Matrix<double, 2, 2> &error )
{
  double x, y;
  double dx, dy;
  Evaluate(u,x,y,dx,dy);
  error = m_Sigma1 + m_R1T * m_Sigma2 * m_R1;
  InverseMatrix(error);
  error *= 2.;
}

#ifdef MLP_TIMING
void
SchulteMLPFunction
::PrintTiming(std::ostream& os)
{
  os << "SchulteMLPFunction timing:" << std::endl;
  os << "  EvaluateProbe1: " << m_EvaluateProbe1.GetTotal()
     << ' ' << m_EvaluateProbe1.GetUnit() << std::endl;
  os << "  EvaluateProbe2: " << m_EvaluateProbe2.GetTotal()
     << ' ' << m_EvaluateProbe2.GetUnit() << std::endl;
}
#endif

void
SchulteMLPFunction
::InverseMatrix(itk::Matrix<double, 2, 2> &mat)
{
  double det = 1. / ( mat(0,0)*mat(1,1) - mat(0,1)*mat(1,0) );
  std::swap( mat(0,0), mat(1,1) );
  mat(1,0) *= -1.;
  mat(0,1) *= -1.;
  mat *= det;
}

}
