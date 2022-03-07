#include "tool.hpp"
#include "clang-c/Index.h"
#include <algorithm>
#include <cstring>
#include <tuple>

// TODO: Consolidate get_methods and get_fields visitors on a template

#define StringIsSame(left, right) strcmp(left, right) == 0

Scanner::Scanner(std::string_view filename) {
  CXIndex index = clang_createIndex(0, 0);
  CXTranslationUnit unit = clang_parseTranslationUnit(
      index, filename.data(), nullptr, 0, nullptr, 0, CXTranslationUnit_None);
  if (unit == nullptr) {
    throw "Failed parse File";
  }
  auto cursor = clang_getTranslationUnitCursor(unit);
  memcpy(&this->m_root, &cursor, sizeof(CXCursor));
  memcpy(&this->m_index, &index, sizeof(CXIndex));
  memcpy(&this->m_unit, &unit, sizeof(CXTranslationUnit));

  this->get_target();
  this->get_methods();
  this->get_fields();
}

Scanner::~Scanner() {
  clang_disposeTranslationUnit(this->m_unit);
  clang_disposeIndex(this->m_index);
}

std::string Scanner::get_name(CXCursor const cursor) {
  return std::string{clang_getCString(clang_getCursorSpelling(cursor))};
}

CursorKind Scanner::get_kind(CXCursor const cursor) {
  char const *const kindName = clang_getCString(
      clang_getCursorKindSpelling(clang_getCursorKind(cursor)));
  if (StringIsSame(kindName, "TypeRef")) {
    return CursorKind::IsTypeRef;
  } else if (StringIsSame(kindName, "ClassDecl")) {
    return CursorKind::IsClass;
  } else if (StringIsSame(kindName, "StructDecl")) {
    return CursorKind::IsStruct;
  } else if (StringIsSame(kindName, "CXXMethod")) {
    return CursorKind::IsMethod;
  } else if (StringIsSame(kindName, "FieldDecl")) {
    return CursorKind::IsField;
  }
  return CursorKind::IsUknown;
}

std::vector<std::string> Scanner::get_methods() const {
  std::vector<std::string> methods{};

  auto target_name = Scanner::get_name(m_target);
  auto visitor =
      [/*must be empty*/](CXCursor cursor, CXCursor parent,
                          CXClientData clientData) -> CXChildVisitResult {
    auto pData = static_cast<
        std::tuple<std::string *,             // pointer to target_name string
                   std::vector<std::string> * // pointer to methods vector
                   > * /* parameter is passed as void pointer */>(clientData);

    auto [target_name, methods] = *pData;
    // make sure we are inside the target
    auto cursor_name = Scanner::get_name(cursor);
    if (cursor_name == *target_name) {
      return CXChildVisit_Recurse;
    }

    // match only methods
    auto cursor_kind = Scanner::get_kind(cursor);
    if (cursor_kind == CursorKind::IsMethod) {
      // add method name to vector
      methods->emplace_back(cursor_name);
      std::cout << "// method: " << cursor_name << '\n';
      return CXChildVisit_Continue;
    }
    return CXChildVisit_Continue;
  };

  auto param = std::make_tuple(&target_name, &methods);
  /* Call to visitChildren will modify the cursor, so we'll use a copy */
  CXCursor start{};
  memcpy(&start, &this->m_target, sizeof(CXCursor));

  clang_visitChildren(start, visitor, &param);

  return methods;
}

std::vector<std::string> Scanner::get_fields() const {
  std::vector<std::string> fields{};

  auto target_name = Scanner::get_name(m_target);
  auto visitor =
      [/*must be empty*/](CXCursor cursor, CXCursor parent,
                          CXClientData clientData) -> CXChildVisitResult {
    auto pData = static_cast<
        std::tuple<std::string *,             // pointer to target_name string
                   std::vector<std::string> * // pointer to methods vector
                   > * /* parameter is passed as void pointer */>(clientData);

    auto [target_name, fields] = *pData;
    // make sure we are inside the target
    auto cursor_name = Scanner::get_name(cursor);
    if (cursor_name == *target_name) {
      return CXChildVisit_Recurse;
    }

    // match only methods
    auto cursor_kind = Scanner::get_kind(cursor);
    if (cursor_kind == CursorKind::IsField) {
      // add method name to vector
      fields->emplace_back(cursor_name);
      std::cout << "// field: " << cursor_name << '\n';
      return CXChildVisit_Continue;
    }
    return CXChildVisit_Continue;
  };

  auto param = std::make_tuple(&target_name, &fields);
  /* Call to visitChildren will modify the cursor, so we'll use a copy */
  CXCursor start{};
  memcpy(&start, &this->m_target, sizeof(CXCursor));

  clang_visitChildren(start, visitor, &param);

  return fields;
}

void Scanner::get_target() {
  auto visitor = [](CXCursor cursor, CXCursor parent,
                    CXClientData clientData) -> CXChildVisitResult {
    auto target = static_cast<CXCursor *>(clientData);

    if (Scanner::get_name(parent) == "main") {
      return CXChildVisit_Recurse;
    } else if (Scanner::get_name(cursor) == "Generate") {
      return CXChildVisit_Recurse; // next is the template parameter
    }
    // We expect that now the cursor is pointing to a TypeRef to the actual
    // target
    if (Scanner::get_kind(cursor) == CursorKind::IsTypeRef) {
      // De-reference the type
      CXCursor actual_system = clang_getCursorDefinition(cursor);
      // copy cursor pointing to type to m_target
      memcpy(target, &actual_system, sizeof(CXCursor));
      return CXChildVisit_Break;
    }
    return CXChildVisit_Recurse;
  };

  CXCursor start{};
  memcpy(&start, &this->m_root, sizeof(CXCursor));

  clang_visitChildren(start, visitor, &m_target);
}

int main(int const argc, char const *argv[]) {
  if (argc < 2) {
    std::cerr << "Wrong number of arguments!\n";
    std::exit(-1);
  }
  std::cout << "template<class T>\nstruct Generate{\n  int operator()() "
               "{\n\t\treturn"
               " 0;\n\t} \n};\n";

  std::string const input_filename{argv[1]};

  Scanner scanner{input_filename};
  return 0;
}
