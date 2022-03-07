#include "clang-c/Index.h"
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

enum class CursorKind {
  IsTypeRef,
  IsStruct,
  IsClass,
  IsField,
  IsMethod,
  IsUknown = 99
};
class Scanner {
private:
  /* AST Cursor pointing to translation unit root node */
  CXCursor m_root{};
  CXIndex m_index{};
  CXTranslationUnit m_unit{};

  /* AST Cursor pointing to the class passed to Generator<target> */
  CXCursor m_target{};

  /* Helper to get Node name from cursor */
  static std::string get_name(CXCursor const cursor);

  /* Helper to get Node kind aka C++ base type Strct, Class, Field */
  static CursorKind get_kind(CXCursor const cursor);

  /* Examines target cursor to find it's node's methods */
  std::vector<std::string> get_methods() const;

  /* Examines target cursor to find it's node's fields (member variables) */
  std::vector<std::string> get_fields() const;

  /* Finds target from root node */
  void get_target();

public:
  /* Extracts the relevant information from cursor of root node of AST */
  Scanner(std::string_view filename);
  ~Scanner();
};
