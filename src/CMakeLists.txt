set(PCT_SRCS
  itkMinimalStandardRandomVariateGenerator.cxx
  pctEnergyAdaptiveMLPFunction.cxx
  pctPolynomialMLPFunction.cxx
  pctSchulteMLPFunction.cxx
  )

itk_module_add_library(PCT ${PCT_SRCS})
