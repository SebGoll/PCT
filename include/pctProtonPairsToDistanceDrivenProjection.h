#ifndef __pctProtonPairsToDistanceDrivenProjection_h
#define __pctProtonPairsToDistanceDrivenProjection_h

#include "rtkConfiguration.h"
#include "pctBetheBlochFunctor.h"

#include <rtkQuadricShape.h>
#include <itkInPlaceImageFilter.h>
#include <mutex>

namespace pct
{

template <class TInputImage, class TOutputImage>
class ITK_EXPORT ProtonPairsToDistanceDrivenProjection :
  public itk::InPlaceImageFilter<TInputImage,TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef ProtonPairsToDistanceDrivenProjection             Self;
  typedef itk::InPlaceImageFilter<TInputImage,TOutputImage> Superclass;
  typedef itk::SmartPointer<Self>                           Pointer;
  typedef itk::SmartPointer<const Self>                     ConstPointer;

  typedef itk::Vector<float, 3>                             ProtonPairsPixelType;
  typedef itk::Image<ProtonPairsPixelType,2>                ProtonPairsImageType;
  typedef ProtonPairsImageType::Pointer                     ProtonPairsImagePointer;

  typedef itk::Image<unsigned int, 3>                       CountImageType;
  typedef CountImageType::Pointer                           CountImagePointer;

  typedef itk::Image<float, 3>                              AngleImageType;
  typedef AngleImageType::Pointer                           AngleImagePointer;

  typedef TOutputImage                                      OutputImageType;
  typedef typename OutputImageType::Pointer                 OutputImagePointer;
  typedef typename OutputImageType::RegionType              OutputImageRegionType;

  typedef rtk::QuadricShape                                 RQIType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(ProtonPairsToDistanceDrivenProjection, itk::InPlaceImageFilter);

  /** Get/Set image of proton pairs. */
  itkGetMacro(ProtonPairsFileName, std::string);
  itkSetMacro(ProtonPairsFileName, std::string);

  /** Get/Set the source position. */
  itkGetMacro(SourceDistance, double);
  itkSetMacro(SourceDistance, double);

  /** Get/Set the most likely path type. Can be "schulte" or "polynomial" */
  itkGetMacro(MostLikelyPathType, std::string);
  itkSetMacro(MostLikelyPathType, std::string);

  itkGetMacro(MostLikelyPathTrackerUncertainties, bool);
  itkSetMacro(MostLikelyPathTrackerUncertainties, bool);

  itkGetMacro(MostLikelyPathPolynomialDegree, int);
  itkSetMacro(MostLikelyPathPolynomialDegree, int);

  itkGetMacro(TrackerResolution, double);
  itkSetMacro(TrackerResolution, double);
  itkGetMacro(TrackerPairSpacing, double);
  itkSetMacro(TrackerPairSpacing, double);
  itkGetMacro(MaterialBudget, double);
  itkSetMacro(MaterialBudget, double);

  /** Get/Set the boundaries of the object. */
  itkGetMacro(QuadricIn, RQIType::Pointer);
  itkSetMacro(QuadricIn, RQIType::Pointer);
  itkGetMacro(QuadricOut, RQIType::Pointer);
  itkSetMacro(QuadricOut, RQIType::Pointer);

  /** Get/Set the count of proton pairs per pixel. */
  itkGetMacro(Count, CountImagePointer);

 /** Get/Set the angle of proton pairs per pixel. */
  itkGetMacro(Angle, AngleImagePointer);

 /** Get/Set the angle of proton pairs per pixel. */
  itkGetMacro(SquaredOutput, OutputImagePointer);

  /** Get/Set the ionization potential used in the Bethe-Bloch equation. */
  itkGetMacro(IonizationPotential, double);
  itkSetMacro(IonizationPotential, double);

  /** Get the beam energy. */
  itkGetMacro(BeamEnergy, double);
  itkSetMacro(BeamEnergy, double);

  /** Convert the projection data to line integrals after pre-processing.
  ** Default is off. */
  itkSetMacro(Robust, bool);
  itkGetConstMacro(Robust, bool);
  itkBooleanMacro(Robust);

  /** Do the scatter projections (see Quinones et al, PMB, 2016)
  ** Default is off. */
  itkSetMacro(ComputeScattering, bool);
  itkGetConstMacro(ComputeScattering, bool);
  itkBooleanMacro(ComputeScattering);

  /** Compute WEPL variance per pixel in the projections
  ** Default is off. */
  itkSetMacro(ComputeNoise, bool);
  itkGetConstMacro(ComputeNoise, bool);
  itkBooleanMacro(ComputeNoise);

protected:
  ProtonPairsToDistanceDrivenProjection();
  virtual ~ProtonPairsToDistanceDrivenProjection() {}

  virtual void
  BeforeThreadedGenerateData() override;
  virtual void
  ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread, rtk::ThreadIdType threadId) override;
  virtual void
  AfterThreadedGenerateData() override;

  /** The two inputs should not be in the same space so there is nothing
   * to verify. */
  virtual void
  VerifyInputInformation() const override
  {}

private:
  ProtonPairsToDistanceDrivenProjection(const Self&); //purposely not implemented
  void operator=(const Self&);            //purposely not implemented

  std::string m_ProtonPairsFileName;
  double m_SourceDistance;
  std::string m_MostLikelyPathType;
  int m_MostLikelyPathPolynomialDegree;

  double m_BeamEnergy;
  bool m_VariableBeamEnergy=false;

  // MLP considering tracker uncertainties
  bool m_MostLikelyPathTrackerUncertainties;
  double m_TrackerResolution;
  double m_TrackerPairSpacing;
  double m_MaterialBudget;

  /** Count event in each thread */
  CountImagePointer m_Count;
  std::vector<CountImagePointer> m_Counts;

  AngleImagePointer                 m_Angle;
  std::vector<AngleImagePointer>    m_Angles;
  std::vector< std::vector<float> > m_AnglesVectors;
  std::mutex                        m_AnglesVectorsMutex;

  AngleImagePointer m_AngleSq;
  std::vector<AngleImagePointer> m_AnglesSq;

  OutputImagePointer m_SquaredOutput;
  std::vector<OutputImagePointer> m_SquaredOutputs; // NK: squared WEPL for noise projections


  /** Create one output per thread */
  std::vector<OutputImagePointer> m_Outputs;
  // std::vector<OutputImagePointer> m_AngleOutputs; // Note NK: check these declarations. Are these members really used?
  // std::vector<OutputImagePointer> m_AngleSqOutputs; // ... probably only m_Angles and m_AngleSq, declared above, are used.

  /** The two quadric functions defining the object support. */
  RQIType::Pointer m_QuadricIn;
  RQIType::Pointer m_QuadricOut;

  /** Ionization potential used in the Bethe Bloch equation */
  double m_IonizationPotential;

  /** The functor to convert energy loss to attenuation */
  Functor::IntegratedBetheBlochProtonStoppingPowerInverse<float, double> *m_ConvFunc;

  ProtonPairsImageType::Pointer m_ProtonPairs;
  bool                          m_Robust;
  bool                          m_ComputeScattering;
  bool                          m_ComputeNoise;
};

} // end namespace pct

#ifndef ITK_MANUAL_INSTANTIATION
#include "pctProtonPairsToDistanceDrivenProjection.hxx"
#endif

#endif
