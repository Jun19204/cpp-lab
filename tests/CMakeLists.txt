include(GoogleTest)

# Add test targets here.

add_executable(disasm-rv32i_test Test_disasm-rv32i.cc)
target_link_libraries(disasm-rv32i_test PRIVATE RV32I_lib)



set(
  ALL_TESTS
    disasm-rv32i_test
)

foreach(test IN LISTS ALL_TESTS)
  target_compile_features(${test} PRIVATE cxx_std_23)
  target_link_libraries(
    ${test}
    PRIVATE
      project_warnings
      project_sanitizers
      GTest::gtest_main
  )
  gtest_discover_tests(
    ${test}
  )
endforeach()
