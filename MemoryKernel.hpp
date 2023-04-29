#ifndef MEMORY_KERNEL_HPP
#define MEMORY_KERNEL_HPP

#include <string>
#include <vector>

#include "ast.hpp"

/* All classess prototypes */
class MemObject;
class MemFunction;
class MemoryKernel;
/* End prototypes */

/**
 * @brief Memory object type
 *
 * (arrays and tuples are processed separately)
 */
enum ObjectType : int {
  OBJECT_STRING = 0,
  OBJECT_NUMBER,
  OBJECT_BOOL,
  OBJECT_FUNC,
};

/**
 * @brief Objects that are stored in MemoryKernel
 * (can be changed if needed, MemoryKernel operates only with pointers)
 *
 */
class MemObject {
 private:
  ObjectType type;
  std::string name;
  std::string value;

  // number of times object was used by other objects
  unsigned int num_references;

 public:
  MemObject(ObjectType type, std::string name, std::string value);

  // show warning if object was not used
  ~MemObject();

  // getters
  ObjectType get_type() const;
  std::string get_name() const;
  std::string get_value() const;
  unsigned int count_references() const;

  // setters
  void set_type(ObjectType type);
  void set_value(std::string value);

  // increment number of references
  // (needed to show warning if object was not used)
  void ref_inc();
};

/**
 * @brief Function is special object which contains
 * useful metainformation about function object
 *
 */
class MemFunction : MemObject {
 private:
  // entry point for function
  AST::Block *entry_point;

  // names of arguments required by function
  // (required to prepare memory before function call)
  std::vector<std::string> arg_names;

 public:
  MemFunction(std::string name, AST::Block *entry_point,
              std::vector<std::string> arg_names);

  // Get entry point block for execution
  // (do not forget to call `prep_mem` before run!)
  AST::Block *get_entry_point() const;

  // Get number of arguments required by function
  unsigned int count_args();

  /**
   * @brief Prepare memory before function call
   * (correctly place args in memory,
   *  even arrays and tuples processed)
   *
   * @param mem Reference to memory to be prepared
   * @param args list of args
   * @return true if memory prepared successfully
   * @return false if failed to prepare memory (invalid number of args)
   */
  bool prep_mem(MemoryKernel &mem, std::vector<MemObject> args);
};

/**
 * @brief Memory Kernel is a core of memory management
 * in nonamelang interpreter. Among its responsibilities:
 *  1. Objects storage
 *  2. Controling variables visibility scope
 *  3. Optimizations suggestions if debug is enabled
 *     (e.g. warnings on unused variables, etc.)
 *  4. Functions management (check if number of arguments is correct, etc.)
 *  5. Additional array and tuple elements support
 *     (To store array or tuple it is needed to split it
 *      into primitives and store them separately with name in
 *      the following format: ARRAY_NAME@ELEMENT_NAME,
 *      e.g. if we declare `const a = [33, 22, 11]`, it should be
 *      saved in memory in 3 iterations: a@1=33, a@2=22, a@3=11.
 *      It is important to follow this convention as there are
 *      additional actions performed to array/tuple management)
 *
 */
class MemoryKernel {
 private:
  // each element of scopes is a vector of objects in current scope
  std::vector<std::vector<MemObject *>> scopes;

 public:
  MemoryKernel();

  /**
   * @brief Get the object by name
   *
   * (if array or tuple element is required,
   *  then use ARRAY_NAME@ELEMENT_NAME notation)
   *
   * @param name Objects name
   * @return Pointer to object or nullptr if it does not exist
   */
  MemObject *get_object(std::string name) const;

  /**
   * @brief Put object to memory
   *
   * @param name Object name
   * @param obj Object itself
   * @return true if object existed before
   * @return false if object did not exist before
   */
  bool put_object(std::string name, MemObject *obj);

  /**
   * @brief Manually delete object from memory
   *        (May be useful on inline array redefinition)
   *
   * @param name Name of object to be dropped
   * @return true If object dropped successfully 
   * @return false If object does not exist
   */
  bool drop_object(std::string name);

  /**
   * @brief Should be called on each new visibility scope entered
   * (It is needed to track memory for each visibility scope
   * and cleanup on scope exit)
   */
  void enter_scope();

  /**
   * @brief Should be called on each visibility scope exited
   * (It is needed to track memory for each visibility scope
   * and cleanup on scope exit)
   */
  void exit_scope();

  /**
   * @brief Dump current memory to standard output
   *        (Can be used in debug purposes)
   * 
   */
  void dump_mem() const;
};

#endif  // MEMORY_KERNEL_HPP