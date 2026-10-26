# ============================================================
# 1. Add Executables (실행 파일 타겟 등록)
# ============================================================
add_executable(Query_app Query-main.cc)
add_executable(Quote_app Quote-main.cc)
add_executable(BuilderPattern_app builder_pattern-main.cc)
add_executable(TestTemplate_app TestTemplate-main.cc)
add_executable(TestDisasm_app TestDisasm-main.cc)

# ============================================================
# 2. Link Libraries (개별 라이브러리 및 헤더 경로 연결)
# ============================================================
target_link_libraries(Query_app PRIVATE Query_lib)
target_link_libraries(Quote_app PRIVATE Quote_headers Basket_lib)
target_link_libraries(TestTemplate_app PRIVATE MyTemplate_lib)
target_link_libraries(TestDisasm_app PRIVATE Disasm_headers)

# 빌더 패턴 앱이 컴파일러 인덱싱에서 누락되어 꼬이는 것을 방지하기 위해 임시 헤더 연결
target_link_libraries(BuilderPattern_app PRIVATE MyTemplate_lib) 

# ===========================================================
# 3. Common Settings (C++23 표준 및 공통 플래그 일괄 주입)
# ===========================================================
set(
  ALL_EXES
    Query_app
    Quote_app
    BuilderPattern_app
    TestTemplate_app
    TestDisasm_app
)

foreach(exe IN LISTS ALL_EXES)
  target_compile_features(${exe} PRIVATE cxx_std_23)
  
  target_link_libraries(
    ${exe} 
    PRIVATE 
      project_warnings 
      project_sanitizers
  )
endforeach()
