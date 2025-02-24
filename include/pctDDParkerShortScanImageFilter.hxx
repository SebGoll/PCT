#ifndef __pctDDParkerShortScanImageFilter_txx
#define __pctDDParkerShortScanImageFilter_txx

#include <itkImageRegionIteratorWithIndex.h>
#include <itkMacro.h>

namespace pct
{

template <class TInputImage, class TOutputImage>
void
DDParkerShortScanImageFilter<TInputImage, TOutputImage>::DynamicThreadedGenerateData(
  const OutputImageRegionType & outputRegionForThread)
{
  // Get angular gaps and max gap
  std::vector<double> angularGaps = m_Geometry->GetAngularGapsWithNext(m_Geometry->GetGantryAngles());
  int                 nProj = angularGaps.size();
  int                 maxAngularGapPos = 0;
  for (int iProj = 1; iProj < nProj; iProj++)
    if (angularGaps[iProj] > angularGaps[maxAngularGapPos])
      maxAngularGapPos = iProj;

  // Input / ouput iterators
  itk::ImageRegionConstIterator<InputImageType> itIn(this->GetInput(), outputRegionForThread);
  itk::ImageRegionIterator<OutputImageType>     itOut(this->GetOutput(), outputRegionForThread);
  itIn.GoToBegin();
  itOut.GoToBegin();

  // Not a short scan if less than 20 degrees max gap, => nothing to do
  // FIXME: do nothing in parallel geometry, currently handled with a trick in the geometry object
  if (m_Geometry->GetSourceToDetectorDistances()[0] == 0. || angularGaps[maxAngularGapPos] < itk::Math::pi / 9)
  {
    if (this->GetInput() != this->GetOutput()) // If not in place, copy is
                                               // required
    {
      while (!itIn.IsAtEnd())
      {
        itOut.Set(itIn.Get());
        ++itIn;
        ++itOut;
      }
    }
    return;
  }

  // Weight image parameters
  typename WeightImageType::RegionType  region;
  typename WeightImageType::SpacingType spacing;
  typename WeightImageType::PointType   origin;
  region.SetSize(0, outputRegionForThread.GetSize(0));
  region.SetIndex(0, outputRegionForThread.GetIndex(0));
  spacing[0] = this->GetInput()->GetSpacing()[0];
  origin[0] = this->GetInput()->GetOrigin()[0];

  // Create one line of weights
  typename WeightImageType::Pointer weights = WeightImageType::New();
  weights->SetSpacing(spacing);
  weights->SetOrigin(origin);
  weights->SetRegions(region);
  weights->Allocate();
  typename itk::ImageRegionIteratorWithIndex<WeightImageType> itWeights(weights, weights->GetLargestPossibleRegion());

  const std::vector<double>                 rotationAngles = m_Geometry->GetGantryAngles();
  const std::multimap<double, unsigned int> sortedAngles = m_Geometry->GetSortedAngles(m_Geometry->GetGantryAngles());
  const double                              detectorWidth =
    this->GetInput()->GetSpacing()[0] * this->GetInput()->GetLargestPossibleRegion().GetSize()[0];


  // Compute delta between first and last angle where there is weighting required
  // First angle
  std::multimap<double, unsigned int>::const_iterator itFirstAngle;
  itFirstAngle = sortedAngles.find(rotationAngles[maxAngularGapPos]);
  itFirstAngle = (++itFirstAngle == sortedAngles.end()) ? sortedAngles.begin() : itFirstAngle;
  itFirstAngle = (++itFirstAngle == sortedAngles.end()) ? sortedAngles.begin() : itFirstAngle;
  const double firstAngle = itFirstAngle->first;
  // Last angle
  std::multimap<double, unsigned int>::const_iterator itLastAngle;
  itLastAngle = sortedAngles.find(rotationAngles[maxAngularGapPos]);
  itLastAngle = (itLastAngle == sortedAngles.begin()) ? --sortedAngles.end() : --itLastAngle;
  double lastAngle = itLastAngle->first;
  if (lastAngle < firstAngle)
    lastAngle += 2 * itk::Math::pi;
  // Delta
  double delta = 0.5 * (lastAngle - firstAngle - itk::Math::pi);
  delta = delta - 2 * itk::Math::pi * floor(delta / (2 * itk::Math::pi)); // between -2*PI and 2*PI

  double sox = m_Geometry->GetSourceOffsetsX()[itIn.GetIndex()[2]];
  double sid = m_Geometry->GetSourceToIsocenterDistances()[itIn.GetIndex()[2]];
  double invsid = 1. / sqrt(sid * sid + sox * sox);
  if (delta < atan(0.5 * detectorWidth * invsid))
    itkWarningMacro(<< "You do not have enough data for proper Parker weighting (short scan)"
                    << "Delta is " << delta * 180. / itk::Math::pi
                    << " degrees and should be more than half the beam angle, i.e. "
                    << atan(0.5 * detectorWidth * invsid) * 180. / itk::Math::pi << " degrees.");

  for (unsigned int p = 0; p < outputRegionForThread.GetSize(3); p++)
  {
    sox = m_Geometry->GetSourceOffsetsX()[itIn.GetIndex()[3]];
    sid = m_Geometry->GetSourceToIsocenterDistances()[itIn.GetIndex()[3]];
    invsid = 1. / sqrt(sid * sid + sox * sox);

    // Prepare weights for current slice (depends on ProjectionOffsetsX)
    typename WeightImageType::PointType point;
    weights->TransformIndexToPhysicalPoint(itWeights.GetIndex(), point);

    // Parker's article assumes that the scan starts at 0, convert projection
    // angle accordingly
    double beta = rotationAngles[itIn.GetIndex()[3]];
    beta = beta - firstAngle;
    if (beta < 0)
      beta += 2 * itk::Math::pi;

    itWeights.GoToBegin();
    while (!itWeights.IsAtEnd())
    {
      const double l = m_Geometry->ToUntiltedCoordinateAtIsocenter(itIn.GetIndex()[3], point[0]);
      double       alpha = atan(-1 * l * invsid);
      if (beta <= 2 * delta - 2 * alpha)
        itWeights.Set(2. * pow(sin((itk::Math::pi * beta) / (4 * (delta - alpha))), 2.));
      else if (beta <= itk::Math::pi - 2 * alpha)
        itWeights.Set(2.);
      else if (beta <= itk::Math::pi + 2 * delta)
        itWeights.Set(2. * pow(sin((itk::Math::pi * (itk::Math::pi + 2 * delta - beta)) / (4 * (delta + alpha))), 2.));
      else
        itWeights.Set(0.);

      ++itWeights;
      point[0] += spacing[0];
    }

    // Multiply each line of the current slice
    for (unsigned int k = 0; k < outputRegionForThread.GetSize(2); k++)
    {
      for (unsigned int j = 0; j < outputRegionForThread.GetSize(1); j++)
      {
        itWeights.GoToBegin();
        while (!itWeights.IsAtEnd())
        {
          itOut.Set(itIn.Get() * itWeights.Get());
          ++itWeights;
          ++itIn;
          ++itOut;
        }
      }
    }
  }
}

} // end namespace pct
#endif
