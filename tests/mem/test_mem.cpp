#include <iostream>

#include "../../MemoryKernel.hpp"

using namespace std;

/**************************************************
 *        Useful macros and init function
 **************************************************/

#define _INIT __attribute__((constructor(0)))
#define _TEST __attribute__((constructor(10)))

#define TEST_BEGIN() cout << "\n=== " << __func__ << " begin ===\n"
#define TEST_END() cout << "=== " << __func__ << " end ===\n"

// print last n warning lines from warning file
#define DUMP_WARN(n)                      \
  do {                                    \
    stringstream ss;                      \
    ss << "cat .nnl_warn | tail -n" << n; \
    std::system(ss.str().c_str());        \
  } while (0)

_INIT void init_tests() { static std::ios_base::Init _; }

/**************************************************
 *                  Tests Begin
 **************************************************/

_TEST void scope_and_objects_intro() {
  TEST_BEGIN();
  MemoryKernel mem;  // main memory object

  // MUST create at least one scope before
  // saving something into memory
  mem.enter_scope();  // scope 1

  // put some objects into memory
  mem.put_object(new MemObject(OBJECT_STRING, "t1_s1", "abc"));
  mem.put_object(new MemObject(OBJECT_STRING, "t1_s2", "cba"));

  cout << "2 variables in scope 1\n";
  mem.dump_mem();  // can be used to dump current memory (in debug purposes)

  mem.enter_scope();  // scope 2
  mem.put_object(new MemObject(OBJECT_STRING, "t1_s3", "123"));

  cout << "2 variables in scope 1, 1 variable in scope 2\n";
  mem.dump_mem();

  mem.exit_scope();  // scope 2

  mem.put_object(new MemObject(OBJECT_STRING, "t1_s4", "qwe"));

  cout << "3 variables in scope 1\n";
  mem.dump_mem();

  mem.exit_scope();  // scope 1

  // dump warnings (they are saved to file)
  DUMP_WARN(100);

  TEST_END();
}

_TEST void mem_object_intro() {
  TEST_BEGIN();
  MemoryKernel mem;
  mem.enter_scope();

  /**
   * MemoryObject have:
   * 1. type - string, number, bool or function
   *    Arrays and tuples are processed in special way,
   *    to use them just save each element in following format:
   *    ARR_NAME@ELEMENT_NAME (will be demostrated in separate example)
   * 2. name
   * 3. value (in string format)
   */

  MemObject *obj = new MemObject(OBJECT_NUMBER, "some_num", "100.3");

  cout << "type: " << obj->get_type() << "\n"  // just some number
       << "name: " << obj->get_name() << "\n"
       << "value: " << obj->get_value() << "\n";

  // we can try to convert values to other types
  // (can use stringstream or other mechanisms)
  stringstream ss;
  ss << obj->get_value();
  double d;
  ss >> d;

  cout << obj->get_name() << " + 15 = " << d + 15 << "\n";

  mem.put_object(obj);

  // can check if object exists in memory:

  // true
  cout << "some_num in memory: " << (mem.get_object("some_num") != nullptr)
       << "\n";

  // false
  cout << "some_func in memory: " << (mem.get_object("some_func") != nullptr)
       << "\n";

  // can increment number of references and get their number
  obj->ref_inc();
  obj->ref_inc();

  cout << "Number of references: " << obj->count_references() << "\n";  // 2

  // can set objects type and value
  obj->set_type(OBJECT_BOOL);
  obj->set_value("0");

  cout << "New type: " << obj->get_type() << "\n";
  cout << "New value: " << obj->get_value() << "\n";

  mem.exit_scope();
  TEST_END();
}

_TEST void references_intro() {
  TEST_BEGIN();
  /**
   * If number of references is equal to zero,
   * that means that variable was not used anywhere.
   * Due to this reason, interpreter dumps warning into
   * file `.nnl_warn`
   */

  MemoryKernel mem;
  mem.enter_scope();  // scope 1

  mem.put_object(new MemObject(OBJECT_STRING, "t2_s1", "abc"));
  mem.put_object(new MemObject(OBJECT_STRING, "t2_s2", "cba"));

  mem.put_object(new MemObject(OBJECT_STRING, "t2_s3", "cba"));

  // we can get object by name
  MemObject *obj = mem.get_object("t2_s3");

  // increment number of references to s3
  obj->ref_inc();

  // we can get number of references to object
  cout << "Number of references to t2_s3: " << obj->count_references() << "\n";

  mem.exit_scope();  // scope 1

  // There will be warnings only for s1 and s2
  // because s3 has at least one reference
  // (intentionally print 3 last lines to see that
  // previous message was from prevoius test and
  // there is no message about t2_s3)
  DUMP_WARN(3);

  TEST_END();
}

_TEST void reassign_object() {
  TEST_BEGIN();
  MemoryKernel mem;
  mem.enter_scope();  // scope 1

  // if the object already exists in memory,
  // it will be reassinged, even if it is created in other scope
  // (exception - function calls)

  mem.put_object(new MemObject(OBJECT_STRING, "x0", "zxc"));

  mem.enter_scope();  // scope 2

  mem.put_object(new MemObject(OBJECT_STRING, "x1", "123"));
  mem.put_object(new MemObject(OBJECT_STRING, "x0", "aaa"));

  cout << "1 variable in scope 1, 1 variable in scope 2\n"
       << "x0 is reassigned to \"aaa\"\n";
  mem.dump_mem();

  mem.exit_scope();  // scope 2
  mem.exit_scope();  // scope 1
  TEST_END();
}

_TEST void assign_array_element() {
  TEST_BEGIN();
  MemoryKernel mem;
  mem.enter_scope();

  // array element can be assigned in the same way as other variables
  // just use following format: ARRAY_NAME@ELEMENT_NAME

  // if array element is assigned in the same scope as
  // other its elements, there is no any difference with
  // assignment of primary variable

  mem.put_object(new MemObject(OBJECT_STRING, "a@1", "hello"));
  mem.put_object(new MemObject(OBJECT_NUMBER, "a@2", "100"));

  // nothing unusual, just array elements in the scope
  mem.dump_mem();

  // also there is a possibility to get all array elements
  cout << "List array elements: ";
  auto elements = mem.extract_array("a");
  for (MemObject *obj : elements) {
    cout << obj->get_name() << " ";
  }
  cout << "\n";

  mem.exit_scope();
  TEST_END();
}

_TEST void reassing_array_element() {
  TEST_BEGIN();
  MemoryKernel mem;
  mem.enter_scope();  // scope 1

  /**
   * When we push array element on the scope
   * which is deeper that initial array scope,
   * it anyway puts variable in its scope.
   */

  mem.put_object(new MemObject(OBJECT_STRING, "a@x", "q"));

  mem.enter_scope();  // scope 2

  // we create a@y in scope 2, but as array `a` already
  // exists, we put it to its initial scope
  mem.put_object(new MemObject(OBJECT_STRING, "a@y", "w"));

  // b@y just put to current scope, because array `b` did not exist
  mem.put_object(new MemObject(OBJECT_STRING, "b@y", "e"));

  cout << "a@x and a@y are in scope 1, b@y in scope 2\n";
  mem.dump_mem();

  mem.exit_scope();  // scope 2

  cout << "array `b` dropped, array `a` still exists\n";
  mem.dump_mem();

  mem.exit_scope();  // scope 1

  cout << "All variables are dropped\n";
  mem.dump_mem();

  TEST_END();
}

_TEST void drop_object() {
  TEST_BEGIN();
  MemoryKernel mem;
  mem.enter_scope();

  /**
   * Objects can be dropped manually.
   * It may be needed, when we override array, e.g:
   * a = [1, 2, 3]
   *
   * then
   * a = [3, 2, 1]
   *
   * Before reinitialization it is neccessary to
   * drop first defined elements.
   *
   * Primary objects also can be dropped.
   */

  mem.put_object(new MemObject(OBJECT_STRING, "uwu", "uWu"));
  mem.put_object(new MemObject(OBJECT_STRING, "wuw", "WuW"));

  mem.drop_object("uwu");

  cout << "Only `wuw` had left\n";
  mem.dump_mem();

  mem.exit_scope();
  TEST_END();
}

_TEST void functions_intro() {
  TEST_BEGIN();
  MemoryKernel mem;
  mem.enter_scope();

  /**
   * MemFunction is a subclass of MemObject
   * That means that function are objects as numbers or strings
   *
   * MemFunction contatins some additional information and
   * methods for function calls.
   */

  AST::Block *b =
      static_cast<AST::Block *>(malloc(1024));  // simulate some block of code

  MemFunction *f = new MemFunction("some_func", b, vector<string>{"x", "y"});

  cout << "Function " << f->get_name() << " with " << f->count_args()
       << " parameters\n";

  vector<string> args = f->get_arg_names();
  cout << "Following parameters are required: ";
  for (string param : args) cout << param << " ";
  cout << "\n";

  // Functions are ordinal objects in memory
  mem.put_object(f);
  mem.dump_mem();

  // check if element in memory is function
  mem.put_object(new MemObject(OBJECT_STRING, "not_func", "123"));

  cout << "`some_func` is function: "
       << (dynamic_cast<MemFunction *>(mem.get_object("some_func")) != 0)
       << "\n";  // true

  cout << "`not_func` is function: "
       << (dynamic_cast<MemFunction *>(mem.get_object("not_func")) != 0)
       << "\n";  // false

  mem.exit_scope();
  TEST_END();
}

_TEST void function_prepare_memory_before_call() {
  TEST_BEGIN();
  MemoryKernel mem;
  mem.enter_scope();

  AST::Block *b =
      static_cast<AST::Block *>(malloc(1024));  // simulate some block of code
  MemFunction *f = new MemFunction("some_func", b, vector<string>{"x", "y"});
  vector<string> arg_names = f->get_arg_names();
  vector<string> call_parameters = {
      "hello",  // this is for `x`
      "world"   // this is for `y`
  };

  vector<MemObject *> to_call;

  for (int i = 0; i < f->count_args(); ++i) {
    to_call.push_back(
        new MemObject(OBJECT_STRING, arg_names[i], call_parameters[i]));
  }

  mem.put_object(f);  // push function to memory itself

  // it is MANDATORY to open new scope before function call
  mem.enter_scope();  // function scope

  // pushes arguments to memory for function call
  f->prep_mem(mem, to_call);

  mem.dump_mem();

  mem.exit_scope();  // function scope

  cout << "Deleted function scope variables\n";
  mem.dump_mem();

  mem.exit_scope();
  TEST_END();
}

_TEST void pass_array_to_function() {
  TEST_BEGIN();
  MemoryKernel mem;
  mem.enter_scope();

  /**
   * It is possible to pass array to function
   * even if we pass multiple array elements to function call,
   * they will be considered as a single element
   * (e.g. a@1, a@2 and a@3 are considered as single element)
   *
   */

  AST::Block *b =
      static_cast<AST::Block *>(malloc(1024));  // simulate some block of code
  MemFunction *f = new MemFunction("some_func", b, vector<string>{"x", "y"});
  mem.put_object(f);

  vector<MemObject *> to_call = {
      new MemObject(OBJECT_STRING, "x@1", "a"),
      new MemObject(OBJECT_STRING, "x@2", "b"),
      new MemObject(OBJECT_STRING, "x@3", "c"),
      new MemObject(OBJECT_STRING, "y", "z"),
  };

  mem.enter_scope();  // function scope

  bool success = f->prep_mem(mem, to_call);

  // true
  cout << "Memory had been prepared: " << success << "\n";

  cout << "Array x and variable y are created\n";
  mem.dump_mem();

  mem.exit_scope();  // function scope

  cout << "Function scope had been cleaned\n";
  mem.dump_mem();

  mem.exit_scope();
  TEST_END();
}

_TEST void function_wrong_number_of_arguments() {
  TEST_BEGIN();
  MemoryKernel mem;
  mem.enter_scope();

  /**
   * If we provide wrong number of arguments, nothing will
   * be pushed to memory and `mem_prep` will return false
   */

  AST::Block *b =
      static_cast<AST::Block *>(malloc(1024));  // simulate some block of code
  MemFunction *f = new MemFunction("some_func", b, vector<string>{"x", "y"});
  mem.put_object(f);

  vector<MemObject *> to_call = {
      new MemObject(OBJECT_STRING, "x@1", "a"),
      new MemObject(OBJECT_STRING, "x@2", "b"),
      new MemObject(OBJECT_STRING, "x@3", "c"),
      new MemObject(OBJECT_STRING, "y", "z"),
      new MemObject(OBJECT_STRING, "not_needed", "z"),
  };

  mem.enter_scope();  // function scope

  bool success = f->prep_mem(mem, to_call);

  // false
  cout << "Memory had been prepared: " << success << "\n";

  mem.exit_scope();  // function scope

  mem.exit_scope();
  TEST_END();
}

/*
// Here is test template
_TEST void some_test() {
  TEST_BEGIN();
  MemoryKernel mem;
  mem.enter_scope();

  // Test code

  mem.exit_scope();
  TEST_END();
}
*/

int main() {
  // All tests have _TEST keyword before initialization,
  // they will be executed automatically
  return 0;
}