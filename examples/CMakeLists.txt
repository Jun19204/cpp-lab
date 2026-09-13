# ============================================================
# Add Executable of Examples
# ============================================================

add_executable(Query_app Query-main.cc)
add_executable(Quote_app Quote-main.cc)
add_executable(BuilderPattern_app builder_pattern-main.cc)

# ============================================================
# Link Libararies
# ============================================================

target_link_libraries(
  Query_app
  PRIVATE 
    Query_lib
)
target_link_libraries(
  Quote_app
  PRIVATE
    Quote_headers
    Basket_lib
)

# ===========================================================
# Common Settings
# ===========================================================

set(
  ALL_EXES
    Query_app
    Quote_app
    BuilderPattern_app
)
foreach(exe IN LISTS ALL_EXES)
  target_compile_features(
    ${exe}
    PUBLIC
      cxx_std_23
  )
  target_link_libraries(
    ${exe}
    PRIVATE
      project_warnings
      project_sanitizers
  )
endforeach()

