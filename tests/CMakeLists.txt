include(GoogleTest)

# Add test targets here.

add_executable(Disasm_test Test_Disasm.cc)
target_link_libraries(Disasm_test PRIVATE Disasm_headers)



set(
  ALL_TESTS
    Disasm_test
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
