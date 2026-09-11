# ============================================================
# Core Libraries
# ============================================================

add_library(Mail_lib STATIC mail.cc)
add_library(Sales_data_lib STATIC Sales_data.cc)
add_library(StrVec_lib STATIC StrVec.cc)
add_library(Basket_lib STATIC Basket.cc)
add_library(error_code_lib STATIC error_code.cc)
add_library(Query_lib STATIC Query.cc)
#add_library(rQuery_lib STATIC rQuery.cc)

#target_link_libraries(
#  TextQuery_lib
#  PRIVATE
#  StrVec_lib
#)

set(
  ALL_LIBS
  Mail_lib
  Sales_data_lib
  StrVec_lib
  Basket_lib
  error_code_lib
  Query_lib
  #rQuery_lib
)

foreach(lib IN LISTS ALL_LIBS)
  target_include_directories(
    ${lib}
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
  )

  target_link_libraries(
    ${lib}
    PRIVATE
    project_warnings
    project_sanitizers
  )
endforeach()

