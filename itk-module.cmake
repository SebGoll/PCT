# the top-level README is used for describing this module, just
# re-used it for documentation here
set(DOCUMENTATION "")

# itk_module() defines the module dependencies in PCT
# PCT depends on ITKCommon
# The testing module in PCT depends on ITKTestKernel
# and ITKMetaIO(besides PCT and ITKCore)
# By convention those modules outside of ITK are not prefixed with
# ITK.

# define the dependencies of the include module and the tests
itk_module(PCT
  DEPENDS
    ITKCommon
    ITKStatistics
    RTK
  COMPILE_DEPENDS
    ITKImageSources
  TEST_DEPENDS
    ITKTestKernel
    ITKMetaIO
  DESCRIPTION
    "${DOCUMENTATION}"
  EXCLUDE_FROM_DEFAULT
  ENABLE_SHARED
)
