# ============================================================
# Core Libraries
# ============================================================

add_library(Mail_lib STATIC mail.cc)
add_library(Sales_data_lib STATIC Sales_data.cc)
add_library(StrVec_lib STATIC StrVec.cc)
add_library(Basket_lib STATIC Basket.cc)
add_library(error_code_lib STATIC error_code.cc)
add_library(Query_lib STATIC Query.cc)
# add_library(rQuery_lib STATIC rQuery.cc)

# ============================================================
# Header-only Libraries
# ============================================================

add_library(MyTemplate_lib INTERFACE)
target_include_directories(
  MyTemplate_lib
  INTERFACE
    "${PROJECT_SOURCE_DIR}/include"
)
target_compile_features(
  MyTemplate_lib
  INTERFACE
    cxx_std_23
)

add_library(Quote_headers INTERFACE)
target_include_directories(
  Quote_headers
  INTERFACE
    "${PROJECT_SOURCE_DIR}/include"
)
target_compile_features(
  Quote_headers
  INTERFACE
    cxx_std_23
)

# ============================================================
# Common Static Library Settings
# ============================================================

set(
  ALL_LIBS
    Mail_lib
    Sales_data_lib
    StrVec_lib
    Basket_lib
    error_code_lib
    Query_lib
    # rQuery_lib
)

foreach(lib IN LISTS ALL_LIBS)
  target_include_directories(
    ${lib}
    PUBLIC
      "${PROJECT_SOURCE_DIR}/include"
  )
  target_compile_features(
    ${lib}
    PUBLIC
      cxx_std_23
  )
  target_link_libraries(
    ${lib}
    PRIVATE
      project_warnings
      project_sanitizers
  )
endforeach()

