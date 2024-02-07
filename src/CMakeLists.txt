add_executable (ap
  AP.cpp
  CodeGen.cpp
  Lexer.cpp
  Parser.cpp
  Sema.cpp
  )
target_link_libraries(ap PRIVATE ${llvm_libs})
