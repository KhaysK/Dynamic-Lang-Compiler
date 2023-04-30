#include "MemoryKernel.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <set>
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

/**************************************************
 *           MemObject Implementation
 **************************************************/

MemObject::MemObject(ObjectType type, std::string name, std::string value)
    : type(type), name(name), value(value), num_references(0){};

MemObject::~MemObject() {
  static std::fstream out;

  if (!out.is_open()) out.open(".nnl_warn", std::ios_base::out);

  if (!count_references()) {
    out << "Warning: variable " << this->get_name()
        << " was not used anywhere. You can remove it.\n";
    out.flush();
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

std::vector<std::string> MemFunction::get_arg_names() const {
  return std::vector<std::string>(this->arg_names);
}

unsigned int MemFunction::count_args() { return this->arg_names.size(); }

bool MemFunction::prep_mem(MemoryKernel &mem, std::vector<MemObject *> args) {
  /**
   * There might array elements be
   * passed, and all array element should be considered
   * as single parameter
   */
  std::set<std::string> all_elements;
  std::set<std::string> vars;
  std::set<std::string> args_set(this->arg_names.begin(),
                                 this->arg_names.end());

  std::set<std::string> arr_appeared;

  for (auto &arg : args) {
    std::string name = arg->get_name();

    // return error if same element appeared multiple times
    if (all_elements.find(name) != all_elements.end()) return false;
    all_elements.insert(name);

    if (is_array_element(arg->get_name())) {
      name = extract_array_name(arg->get_name());
      arr_appeared.insert(name);
    }

    vars.insert(name);

    // if wrong parameters are passed, then function call should abort
    if (args_set.find(name) != args_set.end())
      args_set.erase(name);
    else if (arr_appeared.find(name) == arr_appeared.end())
      return false;
  }

  if (this->count_args() != vars.size() || args_set.size() != 0) return false;

  /**
   * Should save object to memory directly
   * overpassing `put_object` method because
   * it is required to 'shadow' global variables
   * with same names as parameters if they exist
   */

  // after all checks are passed, just push args to current scope
  for (MemObject *obj : args) mem.scopes[mem.scopes.size() - 1].push_back(obj);

  return true;
}

/**************************************************
 *           MemoryKernel Implementation
 **************************************************/

bool MemoryKernel::put_primary_element(MemObject *obj) {
  MemObject *_obj = get_object(obj->get_name());

  if (_obj) {
    *_obj = *obj;
    return false;
  }

  scopes[scopes.size() - 1].push_back(obj);
  return true;
}

bool MemoryKernel::put_array_element(MemObject *obj) {
  /**
   * First, check out if the array exists, then
   * if not - place element in the current scope.
   * Otherwise place element in the scope of array
   */

  std::string arr_name = extract_array_name(obj->get_name());
  for (int k = this->scopes.size() - 1; k >= 0; --k) {
    auto &scope = scopes[k];
    for (int i = scope.size() - 1; i >= 0; --i) {
      if (is_array_element(scope[i]->get_name()) &&
          extract_array_name(scope[i]->get_name()) == arr_name) {
        scope.push_back(obj);
        return false;
      }
    }
  }

  this->scopes[this->scopes.size() - 1].push_back(obj);

  return true;
}

MemoryKernel::MemoryKernel() {
  this->scopes = std::vector<std::vector<MemObject *>>();
}

MemObject *MemoryKernel::get_object(std::string name) const {
  /**
   * It is neccessary to traverse memory in inversed
   * order (from the deepest scope to the oldest)
   * because function calls shadows global variables
   * if their name is the same as the name of some
   * variable. After function has exited and scope
   * had been cleaned, the global variable should
   * become available again.
   */
  for (int k = this->scopes.size() - 1; k >= 0; --k) {
    auto &scope = scopes[k];
    for (int i = scope.size() - 1; i >= 0; --i) {
      if (scope[i]->get_name() == name) return scope[i];
    }
  }

  return nullptr;
}

bool MemoryKernel::put_object(MemObject *obj) {
  if (is_array_element(obj->get_name()))
    return MemoryKernel::put_array_element(obj);
  else
    return MemoryKernel::put_primary_element(obj);
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

void MemoryKernel::exit_scope() {
  int n = scopes.size() - 1;
  for (int i = scopes[n].size() - 1; i >= 0; --i) delete scopes[n][i];
  scopes.pop_back();
}

void MemoryKernel::dump_mem() const {
  std::cout << "{\n";
  int depth = 1;
  for (auto &scope : this->scopes) {
    for (MemObject *obj : scope) {
      for (int i = 0; i < depth; ++i) std::cout << "  ";
      std::cout << obj->get_name() << " = " << obj->get_value() << "\n";
    }
    ++depth;
  }
  std::cout << "}\n";
}

std::vector<MemObject *> MemoryKernel::extract_array(std::string name) {
  std::stringstream ss;
  ss << "^" << name << "@..*";

  // pattern is "^NAME@..*"
  std::regex pattern(ss.str());

  std::vector<MemObject *> arr;
  for (auto &scope : this->scopes) {
    for (MemObject *obj : scope) {
      if (std::regex_match(obj->get_name(), pattern)) arr.push_back(obj);
    }
  }

  return arr;
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