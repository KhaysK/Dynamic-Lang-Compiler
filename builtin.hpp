#ifndef __BUILTIN_HPP
#define __BUILTIN_HPP

#include "MemoryKernel.hpp"
#include "ast.hpp"
#include "string"
#include "vector"

using namespace std;

typedef MemObject* (*builtin_exec_t)(MemoryKernel& mem);

class BuiltinBlock : public AST::Block {
  builtin_exec_t exec;

 public:
  BuiltinBlock(builtin_exec_t exec) : exec(exec) {}
  MemObject* eval(MemoryKernel& mem) override { return exec(mem); }

  static void initialize_builtins(MemoryKernel& mem);
};

#endif /* __BUILTIN_HPP */