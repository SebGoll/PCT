itk_module_test()

set(PCTTests
  itkMinimalStandardRandomVariateGeneratorTest.cxx
  itkMyFilterTest.cxx
  )

CreateTestDriver(PCT "${PCT-Test_LIBRARIES}" "${PCTTests}")

itk_add_test(NAME itkMinimalStandardRandomVariateGeneratorTest
  COMMAND PCTTestDriver itkMinimalStandardRandomVariateGeneratorTest
  )

itk_add_test(NAME itkMyFilterTest
  COMMAND PCTTestDriver
    --compare
    DATA{Baseline/itkMyFilterTestOutput.mha}
    ${ITK_TEST_OUTPUT_DIR}/itkMyFilterTestOutput.mha
  itkMyFilterTest
    ${ITK_TEST_OUTPUT_DIR}/itkMyFilterTestOutput.mha
  )
