namespace pct
{

template <class TInputImage, class TOutputImage, class TFFTPrecision>
FDKDDConeBeamReconstructionFilter<TInputImage, TOutputImage, TFFTPrecision>::FDKDDConeBeamReconstructionFilter()
{
  this->SetNumberOfRequiredInputs(1);

  // Create each filter of the composite filter
  m_ExtractFilter = ExtractFilterType::New();
  m_WeightFilter = WeightFilterType::New();
  m_RampFilter = RampFilterType::New();
  m_BackProjectionFilter = BackProjectionFilterType::New();

  // Permanent internal connections
  m_WeightFilter->SetInput(m_ExtractFilter->GetOutput());
  m_RampFilter->SetInput(m_WeightFilter->GetOutput());
  m_BackProjectionFilter->SetProjectionStack(m_RampFilter->GetOutput());

  // Default parameters
  m_ExtractFilter->SetDirectionCollapseToSubmatrix();
  m_WeightFilter->InPlaceOn();
  m_BackProjectionFilter->InPlaceOn();
  m_BackProjectionFilter->SetTranspose(true);
}

template <class TInputImage, class TOutputImage, class TFFTPrecision>
void
FDKDDConeBeamReconstructionFilter<TInputImage, TOutputImage, TFFTPrecision>::GenerateInputRequestedRegion()
{
  typename Superclass::InputImagePointer inputPtr = const_cast<TInputImage *>(this->GetInput());
  if (!inputPtr)
    return;

  // SR: is this useful?
  m_BackProjectionFilter->SetInput(0, this->GetInput(0));
  m_ExtractFilter->SetInput(m_ProjectionStack);
  m_BackProjectionFilter->GetOutput()->SetRequestedRegion(this->GetOutput()->GetRequestedRegion());
  m_BackProjectionFilter->GetOutput()->PropagateRequestedRegion();
}

template <class TInputImage, class TOutputImage, class TFFTPrecision>
void
FDKDDConeBeamReconstructionFilter<TInputImage, TOutputImage, TFFTPrecision>::GenerateOutputInformation()
{
  const unsigned int Dimension = this->InputImageDimension;

  // We only set the first sub-stack at that point, the rest will be
  // requested in the GenerateData function
  typename ExtractFilterType::InputImageRegionType projRegion;
  m_ProjectionStack->UpdateOutputInformation();
  projRegion = m_ProjectionStack->GetLargestPossibleRegion();
  projRegion.SetSize(Dimension, 1);
  m_ExtractFilter->SetExtractionRegion(projRegion);

  // Run composite filter update
  m_BackProjectionFilter->SetInput(0, this->GetInput(0));
  m_ExtractFilter->SetInput(m_ProjectionStack);
  m_BackProjectionFilter->UpdateOutputInformation();

  // Update output information
  this->GetOutput()->SetOrigin(m_BackProjectionFilter->GetOutput()->GetOrigin());
  this->GetOutput()->SetSpacing(m_BackProjectionFilter->GetOutput()->GetSpacing());
  this->GetOutput()->SetDirection(m_BackProjectionFilter->GetOutput()->GetDirection());
  this->GetOutput()->SetLargestPossibleRegion(m_BackProjectionFilter->GetOutput()->GetLargestPossibleRegion());
}

template <class TInputImage, class TOutputImage, class TFFTPrecision>
void
FDKDDConeBeamReconstructionFilter<TInputImage, TOutputImage, TFFTPrecision>::GenerateData()
{
  const unsigned int Dimension = this->InputImageDimension;

  // The backprojection works on a small stack of projections, not the full stack
  typename ExtractFilterType::InputImageRegionType subsetRegion;
  subsetRegion = m_ProjectionStack->GetLargestPossibleRegion();
  unsigned int nProj = subsetRegion.GetSize(Dimension);

  for (unsigned int i = 0; i < nProj; i++)
  {
    // After the first bp update, we need to use its output as input.
    if (i)
    {
      typename TInputImage::Pointer pimg = m_BackProjectionFilter->GetOutput();
      pimg->DisconnectPipeline();
      m_BackProjectionFilter->SetInput(pimg);

      // Change projection subset
      subsetRegion.SetIndex(Dimension, i);
      subsetRegion.SetSize(Dimension, 1);
      m_ExtractFilter->SetExtractionRegion(subsetRegion);

      // This is required to reset the full pipeline
      m_BackProjectionFilter->GetOutput()->UpdateOutputInformation();
      m_BackProjectionFilter->GetOutput()->PropagateRequestedRegion();
    }

    m_PreFilterProbe.Start();
    m_WeightFilter->UpdateLargestPossibleRegion();
    m_PreFilterProbe.Stop();

    m_FilterProbe.Start();
    m_RampFilter->UpdateLargestPossibleRegion();
    m_FilterProbe.Stop();

    m_BackProjectionProbe.Start();
    m_BackProjectionFilter->Update();
    m_BackProjectionProbe.Stop();
  }
  Superclass::GraftOutput(m_BackProjectionFilter->GetOutput());
}

template <class TInputImage, class TOutputImage, class TFFTPrecision>
rtk::ThreeDCircularProjectionGeometry::Pointer
FDKDDConeBeamReconstructionFilter<TInputImage, TOutputImage, TFFTPrecision>::GetGeometry()
{
  return this->m_WeightFilter->GetGeometry();
}

template <class TInputImage, class TOutputImage, class TFFTPrecision>
void
FDKDDConeBeamReconstructionFilter<TInputImage, TOutputImage, TFFTPrecision>::SetGeometry(
  const rtk::ThreeDCircularProjectionGeometry::Pointer _arg)
{
  itkDebugMacro("setting GeometryPointer to " << _arg);
  if (this->GetGeometry() != _arg)
  {
    m_WeightFilter->SetGeometry(_arg);
    m_BackProjectionFilter->SetGeometry(_arg.GetPointer());
    this->Modified();
  }
}

template <class TInputImage, class TOutputImage, class TFFTPrecision>
void
FDKDDConeBeamReconstructionFilter<TInputImage, TOutputImage, TFFTPrecision>::PrintTiming(std::ostream & os) const
{
  os << "FDKDDConeBeamReconstructionFilter timing:" << std::endl;
  os << "  Prefilter operations: " << m_PreFilterProbe.GetTotal() << ' ' << m_PreFilterProbe.GetUnit() << std::endl;
  os << "  Ramp filter: " << m_FilterProbe.GetTotal() << ' ' << m_FilterProbe.GetUnit() << std::endl;
  os << "  Backprojection: " << m_BackProjectionProbe.GetTotal() << ' ' << m_BackProjectionProbe.GetUnit() << std::endl;
}

} // end namespace pct
