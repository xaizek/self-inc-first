set(LLVM_LINK_COMPONENTS support)
set(LLVM_USED_LIBS clangTooling clangBasic clangAST)

add_clang_executable(self-inc-first
                     src/self-inc-first.cpp
                     src/IncludeFinderAction.cpp
                     src/path_utils.cpp)
target_link_libraries(self-inc-first clangTooling clangBasic clangASTMatchers)
