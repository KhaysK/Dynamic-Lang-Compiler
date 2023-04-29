#include "MemoryKernel.hpp"

#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "ast.hpp"

/**************************************************
 *           Local Functions Prototypes
 **************************************************/

/**
 * @brief Checks if name satisfies array element pattern
 *        (ARRAY_NAME@ARRAY_ELEMENT)
 *
 * @param name Name of element to check
 * @return true If matches array element pattern
 * @return false If does not match array element pattern
 */
static bool is_array_element(std::string name);

/**
 * @brief Extract array name from pattern
 *        (e.g. from arr@123 extracts arr)
 *
 * Warning: `is_array_element` should be called first
 *
 * @param name Name of element to be extracted
 * @return extracted name of array
 */
static std::string extract_array_name(std::string name);

/**
 * @brief Extract array as list from memory
 *
 * @param name Name of array to be extracted
 * @return vector of pairs, where key is name of element
 *         without array prefix and value is pointer to MemObject
 */
static std::vector<std::pair<std::string, MemObject *>> extract_array(
    std::string name);

/**************************************************
 *           MemObject Implementation
 **************************************************/

MemObject::MemObject(ObjectType type, std::string name, std::string value)
    : type(type), name(name), value(value), num_references(0){};

MemObject::~MemObject() {
  if (!count_references()) {
    // TODO: implement warning
  }
}

ObjectType MemObject::get_type() const { return this->type; }

std::string MemObject::get_name() const { return this->name; }

std::string MemObject::get_value() const { return this->value; }

unsigned int MemObject::count_references() const {
  return this->num_references;
}

void MemObject::set_type(ObjectType type) { this->type = type; }

void MemObject::set_value(std::string value) { this->value = value; }

void MemObject::ref_inc() { this->num_references++; }

/**************************************************
 *          MemFunction Implementation
 **************************************************/

MemFunction::MemFunction(std::string name, AST::Block *entry_point,
                         std::vector<std::string> arg_names)
    : MemObject(OBJECT_FUNC, name, "(func)"),
      entry_point(entry_point),
      arg_names(arg_names) {}

AST::Block *MemFunction::get_entry_point() const { return this->entry_point; }

unsigned int MemFunction::count_args() { return this->arg_names.size(); }

bool MemFunction::prep_mem(MemoryKernel &mem, std::vector<MemObject> args) {
  // TODO: prepare memory before function call
  return false;
}

/**************************************************
 *           MemoryKernel Implementation
 **************************************************/

MemoryKernel::MemoryKernel() {
  this->scopes = std::vector<std::vector<MemObject *>>();
}

MemObject *MemoryKernel::get_object(std::string name) const {
  for (auto &scope : this->scopes) {
    for (MemObject *object : scope) {
      if (object->get_name() == name) return object;
    }
  }

  return nullptr;
}

bool MemoryKernel::put_object(std::string name, MemObject *obj) {
  // TODO: Implement
  return false;
}

bool MemoryKernel::drop_object(std::string name) {
  MemObject *obj = get_object(name);
  if (!obj) return false;

  for (auto &scope : this->scopes) {
    for (int i = 0; i < scope.size(); ++i) {
      if (scope[i] == obj) scope.erase(scope.begin() + i);
    }
  }

  return true;
}

void MemoryKernel::enter_scope() {
  scopes.push_back(std::vector<MemObject *>());
}

void MemoryKernel::exit_scope() { scopes.pop_back(); }

void MemoryKernel::dump_mem() const {
  std::cout << "{\n";
  int depth = 1;
  for (auto &scope : this->scopes) {
    for (MemObject *obj : scope) {
      for (int i = 0; i < depth; ++i) std::cout << " ";
      std::cout << obj->get_name() << " = " << obj->get_value() << "\n";
      ++depth;
    }
  }
  std::cout << "}\n";
}

/**************************************************
 *         Local Functions Implementation
 **************************************************/

static bool is_array_element(std::string name) {
  static std::regex pattern("..*@..*");
  return std::regex_match(name, pattern);
}

static std::string extract_array_name(std::string name) {
  std::stringstream ss;

  for (auto &c : name) {
    if (c == '@')
      return ss.str();
    else
      ss << c;
  }

  return "";
}

static std::vector<std::pair<std::string, MemObject *>> extract_array(
    std::string name) {
  // TODO: Implement
  return std::vector<std::pair<std::string, MemObject *>>();
}