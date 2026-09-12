# ============================================================
# Examples(executable)
# ============================================================

add_executable(
  Query_app
    Query-main.cc
)

target_link_libraries(
  Query_app
  PRIVATE
    Query_lib
    # rQuery_lib
    project_warnings
    project_sanitizers
)

add_executable(
  Quote_app
    Quote-main.cc
)

target_link_libraries(
  Quote_app
  PRIVATE
    Quote_headers
    Basket_lib
    project_warnings
    project_sanitizers
)

add_executable(
  BuilderPattern_app
    builder_pattern-main.cc
)

