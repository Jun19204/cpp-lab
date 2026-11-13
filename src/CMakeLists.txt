# ============================================================
# Core Libraries
# ============================================================

add_library(Mail_lib STATIC mail.cc)
add_library(Sales_data_lib STATIC Sales_data.cc)
add_library(StrVec_lib STATIC StrVec.cc)
add_library(Basket_lib STATIC Basket.cc)
add_library(error_code_lib STATIC error_code.cc)
add_library(Query_lib STATIC Query.cc)
add_library(MyString_lib STATIC MyString.cc)
add_library(Decoder-rv32i_lib STATIC rv32i.cc)
add_library(Parser-elf_parser_lib STATIC elf_parser.cc)

# ============================================================
# Header-only Libraries
# ============================================================

set(
  ALL_HERDERS
    MyTemplate_lib
    Quote_headers
)

foreach(header IN LISTS ALL_HERDERS)
  add_library(${header} INTERFACE)
  target_include_directories(
    ${header}
    INTERFACE
      "${PROJECT_SOURCE_DIR}/include"
  )
  target_compile_features(
    ${header}
    INTERFACE
      cxx_std_23
  )
  target_link_libraries(
    ${header}
    INTERFACE
      project_warnings
      project_sanitizers
  )
endforeach()


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
    MyString_lib
    Decoder-rv32i_lib
    Parser-elf_parser_lib
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

