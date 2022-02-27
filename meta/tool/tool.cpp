#include "tool.hpp"
#include "clang-c/Index.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#define NAMEOF(cursor) clang_getCString(clang_getCursorSpelling(cursor))
#define KINDOF(cursor)                                                         \
  clang_getCString(clang_getCursorKindSpelling(clang_getCursorKind(cursor)))

std::ostream &operator<<(std::ostream &stream, const CXString &str) {
  stream << clang_getCString(str);
  clang_disposeString(str);
  return stream;
}

CXCursor system_c{};

CXChildVisitResult getMethods(CXCursor cursor, CXCursor parent,
                              CXClientData clientData) {
  if (strcmp(NAMEOF(cursor), NAMEOF(system_c)) == 0) {
    return CXChildVisit_Recurse;
  }
  if (strcmp(KINDOF(cursor), "CXXMethod") == 0) {
    std::cout << "  // Found Method: " << NAMEOF(cursor) << '\n';
  }
  return CXChildVisit_Continue;
}

CXChildVisitResult getFields(CXCursor cursor, CXCursor parent,
                             CXClientData clientData) {
  if (strcmp(NAMEOF(cursor), NAMEOF(system_c)) == 0) {
    return CXChildVisit_Recurse;
  }
  if (strcmp(KINDOF(cursor), "FieldDecl") == 0) {
    std::cout << "  // Found Field: " << NAMEOF(cursor) << '\n';
  }
  return CXChildVisit_Continue;
}

CXChildVisitResult findSystem(CXCursor cursor, CXCursor parent,
                              CXClientData clientData) {

  if (strcmp(NAMEOF(parent), "main") == 0) {
    std::cout << "// Inside main\n";
    return CXChildVisit_Recurse;
  } else {
    if (strcmp(NAMEOF(cursor), "Generate") == 0) {
      std::cout << "// Found Generate " << KINDOF(cursor) << '\n';
      return CXChildVisit_Recurse;
    }
  }
  if (strcmp(KINDOF(cursor), "TypeRef") == 0) {
    std::cout << "// System is referenced as: "
              << clang_getCursorSpelling(cursor) << '\n';
    CXCursor actual_system = clang_getCursorDefinition(cursor);
    memcpy(&system_c, &actual_system, sizeof(CXCursor));
    return CXChildVisit_Break;
  }
  return CXChildVisit_Recurse;
}

int main(int const argc, char const *argv[]) {
  if (argc < 2) {
    std::cerr << "Wrong number of arguments!\n";
    std::exit(-1);
  }

  std::string const input_filename{argv[1]};
  std::cout << "template<class T>\nstruct Generate{\n  int operator()() "
               "{\n\t\treturn"
               " 0;\n\t} \n};\n";

  CXIndex index = clang_createIndex(0, 0);

  CXTranslationUnit unit =
      clang_parseTranslationUnit(index, input_filename.c_str(), nullptr, 0,
                                 nullptr, 0, CXTranslationUnit_None);
  if (unit == nullptr) {
    std::cerr << "Unable to parse translation unit of " << input_filename
              << ". \nQuitting.\n";
    std::exit(-1);
  }
  CXCursor cursor = clang_getTranslationUnitCursor(unit);

  // find generator
  clang_visitChildren(cursor, findSystem, nullptr);
  std::cout << "// System: " << clang_getCursorSpelling(system_c) << '\n';

  clang_visitChildren(cursor, getMethods, nullptr);
  clang_visitChildren(cursor, getFields, nullptr);

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
  return 0;
}
