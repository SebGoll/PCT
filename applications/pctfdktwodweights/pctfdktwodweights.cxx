#include "pctfdktwodweights_ggo.h"
#include "rtkGgoFunctions.h"

#include <rtkThreeDCircularProjectionGeometryXMLFile.h>
#include <rtkProjectionsReader.h>
#include "pctFDKDDWeightProjectionFilter.h"

#include <itkImageFileWriter.h>
#include <itkRegularExpressionSeriesFileNames.h>

int main(int argc, char * argv[])
{
  GGO(pctfdktwodweights, args_info);

  typedef float OutputPixelType;
  const unsigned int Dimension = 3;

  // Generate file names
  itk::RegularExpressionSeriesFileNames::Pointer names = itk::RegularExpressionSeriesFileNames::New();
  names->SetDirectory(args_info.path_arg);
  names->SetNumericSort(false);
  names->SetRegularExpression(args_info.regexp_arg);
  names->SetSubMatch(0);

  if(args_info.verbose_flag)
    std::cout << "Regular expression matches "
              << names->GetFileNames().size()
              << " file(s)..."
              << std::endl;

  // Projections reader
  typedef itk::Image< OutputPixelType, Dimension+1 > ProjectionImageType;
  typedef rtk::ProjectionsReader< ProjectionImageType > ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileNames( names->GetFileNames() );
  TRY_AND_EXIT_ON_ITK_EXCEPTION( reader->GenerateOutputInformation() );

  // Geometry
  if(args_info.verbose_flag)
    std::cout << "Reading geometry information from "
              << args_info.geometry_arg
              << "..."
              << std::endl;
  rtk::ThreeDCircularProjectionGeometryXMLFileReader::Pointer geometryReader;
  geometryReader = rtk::ThreeDCircularProjectionGeometryXMLFileReader::New();
  geometryReader->SetFilename(args_info.geometry_arg);
  TRY_AND_EXIT_ON_ITK_EXCEPTION( geometryReader->GenerateOutputInformation() )

  // Weights filter
  typedef pct::FDKDDWeightProjectionFilter< ProjectionImageType > WeightType;
  WeightType::Pointer wf = WeightType::New();
  wf->SetInput( reader->GetOutput() );
  wf->SetGeometry( geometryReader->GetOutputObject() );
  wf->InPlaceOff();

  // Write
  typedef itk::ImageFileWriter<  ProjectionImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( args_info.output_arg );
  writer->SetInput( wf->GetOutput() );
  writer->SetNumberOfStreamDivisions( args_info.divisions_arg );
  TRY_AND_EXIT_ON_ITK_EXCEPTION( writer->Update() )

  return EXIT_SUCCESS;
}
