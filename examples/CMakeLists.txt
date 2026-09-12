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
  #rQuery_lib
  project_warnings
  project_sanitizers
)

add_executable(
  Quote_app
  Quote-main.cc
)
target_include_directories( # Quote.h, Bulk_quote.h, Disc_quote.h가 header-only이기 때문
  Quote_app
  PRIVATE
  "${PROJECT_SOURCE_DIR}/include"
)
target_link_libraries(
  Quote_app
  PRIVATE
  Basket_lib
  project_warnings
  project_sanitizers
)

add_executable(
  BuilderPattern_app
  builder_pattern-main.cc
)

