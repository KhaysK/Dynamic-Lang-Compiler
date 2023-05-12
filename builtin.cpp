#include "builtin.hpp"

#include <iostream>

#include "MemoryKernel.hpp"
#include "ast.hpp"

using namespace std;

class BuiltinTriplet {
 public:
  string name;
  BuiltinBlock* block;
  vector<string> args;

  BuiltinTriplet(string name, BuiltinBlock* block, vector<string> args)
      : name(name), block(block), args(args) {}
};

/**********************************************************************
 * Builtin functions implementation
 *********************************************************************/

MemObject* builtin_dump_mem(MemoryKernel& mem) {
  mem.dump_mem();
  return nullptr;
}

MemObject* builtin_for_each(MemoryKernel& mem) {
  MemFunction* func =
      dynamic_cast<MemFunction*>(mem.get_object("for_each_func"));

  if (!func) return nullptr;

  vector<string> args = func->get_arg_names();
  vector<MemObject*> arr = mem.extract_array("arr");

  for (int i = 0; i < arr.size(); ++i) {
    MemObject* elem = arr[i];
    MemObject* new_elem =
        new MemObject(elem->get_type(), args[1], elem->get_value());

    mem.enter_scope();
    mem.mark_inside_func();

    string elem_full_name = arr[i]->get_name();
    string elem_name = elem_full_name.substr(elem_full_name.find("@") + 1);

    func->prep_mem(mem, {
                            new MemObject(OBJECT_NUMBER, args[0], elem_name),
                            new_elem,
                        });

    static_cast<AST::Block*>(func->get_entry_point())->eval(mem);

    mem.unmark_inside_func();
    mem.exit_scope();
  }

  return nullptr;
}

/**********************************************************************
 * Builtin functions registrations
 *********************************************************************/

static vector<BuiltinTriplet> builtin_functions = {
    BuiltinTriplet("dump_mem", new BuiltinBlock(builtin_dump_mem), {}),
    BuiltinTriplet("for_each", new BuiltinBlock(builtin_for_each),
                   {"arr", "for_each_func"}),
};

/**********************************************************************
 * Initialize builting functions in memory instance (do not touch)
 *********************************************************************/

void BuiltinBlock::initialize_builtins(MemoryKernel& mem) {
  mem.enter_scope();
  for (auto b : builtin_functions) {
    mem.put_object(new MemFunction(b.name, b.block, b.args));
  }
}
