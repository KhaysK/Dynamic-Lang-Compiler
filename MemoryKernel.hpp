#ifndef MEMORY_KERNEL_HPP
#define MEMORY_KERNEL_HPP

#include <string>
#include <vector>

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
  OBJECT_NULL,
  OBJECT_ARRAY,
};

inline std::string ObjectTypeStr(ObjectType type) {
  static std::vector<std::string> types = {
    "string", "number", "bool", "func",
  };

  if (type < 0 || type >= types.size())
    return "undefined";
  return types[type];
}

/**
 * @brief Objects that are stored in MemoryKernel
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
  virtual ~MemObject();

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
class MemFunction : public MemObject {
 private:
  // entry point for function
  void *entry_point;

  // names of arguments required by function
  // (required to prepare memory before function call)
  std::vector<std::string> arg_names;

 public:
  MemFunction(std::string name, void *entry_point,
              std::vector<std::string> arg_names);

  // Get entry point block for execution
  // (do not forget to call `prep_mem` before run!)
  void *get_entry_point() const;

  // May be needed to fill arguments before function call
  std::vector<std::string> get_arg_names() const;

  // Get number of arguments required by function
  unsigned int count_args();

  /**
   * @brief Prepare memory before function call
   * (correctly place args in memory,
   *  even arrays and tuples processed)
   *
   * Note: needs enter_scope to be called first
   *
   * @param mem Reference to memory to be prepared
   * @param args list of args
   * @return true if memory prepared successfully
   * @return false if failed to prepare memory (invalid number of args
   *         or wrong parameters are passed)
   */
  bool prep_mem(MemoryKernel &mem, std::vector<MemObject *> args);
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
  friend MemFunction;

 private:
  // each element of scopes is a vector of objects in current scope
  std::vector<std::vector<MemObject *>> scopes;
  bool inside_func;

  /**
   * @brief Save primary (not array) element into memory
   *
   * @param obj Object to save
   * @return true if object did not exist before
   * @return false if object existed before
   */
  bool put_primary_element(MemObject *obj);

  /**
   * @brief Save array element into memory
   *
   * @param obj Object to save
   * @return true if object did not exist before
   * @return false if object existed before
   */
  bool put_array_element(MemObject *obj);

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
   * @param obj Object itself
   * @return true if object did not exist before
   * @return false if object existed before
   */
  bool put_object(MemObject *obj);

  /**
   * @brief Put object to global scope
   * 
   * @param obj Object itself
   * @return true if object did not exist before
   * @return false if object existed before
   */
  bool put_global(MemObject *obj);

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

  /**
   * @brief Extract array elements by array name
   *
   * @param name Name of array
   * @return List of array elements
   */
  std::vector<MemObject *> extract_array(std::string name);

  /**
   * @brief Mark memory that it is executed inside 
   *        function at the moment
   */
  void mark_inside_func();

  /**
   * @brief Mark memory that it is not executed inside 
   *        function at the moment
   */
  void unmark_inside_func();

  /**
   * @brief Check if memory is located inside
   *        function at the moment
   */
  bool is_inside_func() const;
};

#endif  // MEMORY_KERNEL_HPP