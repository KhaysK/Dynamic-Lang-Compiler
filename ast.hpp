#ifndef AST_HPP
#define AST_HPP
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <assert.h>

namespace AST {
    class AST_print_context {
    public:
        int indent_;
        AST_print_context() : indent_{0} {};
        void indent() { ++indent_; }
        void dedent() { --indent_; }
    };


    class ASTNode {
    public:
        virtual void json(std::ostream& out, AST_print_context& ctx) = 0;

        std::string str() {
            std::stringstream ss;
            AST_print_context ctx;
            json(ss, ctx);
            return ss.str();
        }
    protected:
        void json_indent(std::ostream& out, AST_print_context& ctx);
        void json_head(std::string node_kind, std::ostream& out, AST_print_context& ctx);
        void json_close(std::ostream& out, AST_print_context& ctx);
        void json_child(std::string field, ASTNode& child, std::ostream& out, AST_print_context& ctx, char sep=',');
    };

    class Block : public ASTNode {
        std::vector<ASTNode*> stmts_;
    public:
        explicit Block() : stmts_{std::vector<ASTNode*>()} {}
        void append(ASTNode* stmt) { stmts_.push_back(stmt); }
        void json(std::ostream& out, AST_print_context& ctx) override;
     };


    class Assign : public ASTNode {
        ASTNode &lexpr_;
        ASTNode &rexpr_;
    public:
        Assign(ASTNode &lexpr, ASTNode &rexpr) :
           lexpr_{lexpr}, rexpr_{rexpr} {};
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    class If : public ASTNode {
        ASTNode &cond_;
        Block &ifpart_; 
        Block &elsepart_;
    public:
        explicit If(ASTNode &cond, Block &ifpart, Block &elsepart) :
            cond_{cond}, ifpart_{ifpart}, elsepart_{elsepart} { };
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    class AsBool : public ASTNode {
        ASTNode &left_;
    public:
        explicit AsBool(ASTNode &left) : left_{left} {}
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    class Print : public ASTNode {
        ASTNode& left_;
    public:
        explicit Print(ASTNode &l) : left_{l} {}
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    class Ident : public ASTNode {
        std::string text_;
    public:
        explicit Ident(std::string txt) : text_{txt} {}
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    // Leaf nodes
    class LeafNode : public ASTNode {
    protected:
        std::string leaf_type;
        std::string value_;
        LeafNode(std::string l_t, std::string v) :
                leaf_type{l_t}, value_{v} {};
    public:
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    class NumberConst : public LeafNode {
    public:
        NumberConst(std::string v) : 
            LeafNode(std::string("Number"), v) {};
    };

    class StringConst : public LeafNode {
    public:
        StringConst(std::string v) :
            LeafNode(std::string("String"), v) {};
    };

    class BoolConst : public LeafNode {
    public:
        BoolConst(std::string v) :
            LeafNode(std::string("Bool"), v) {};
    };

    class VarType : public LeafNode {
    public:
        VarType(std::string v) :
            LeafNode(std::string("VarType"), v) {};
    };

    class OpType : public LeafNode {
    public:
        OpType(std::string v) :
            LeafNode(std::string("Op. type"), v) {};
    };

    class NullConst : public ASTNode {
    public:
        explicit NullConst() {}
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    // Bin Operations
    class BinOp : public ASTNode {
    protected:
        std::string opsym;
        ASTNode &left_;
        ASTNode &right_;
        BinOp(std::string sym, ASTNode &l, ASTNode &r) :
                opsym{sym}, left_{l}, right_{r} {};
    public:
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    class Read : public BinOp {
    public:
        Read(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Read"),  l, r) {};
    };

    class IsOp : public BinOp {
    public:
        IsOp(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Is"),  l, r) {};
    };

    class Plus : public BinOp {
    public:
        Plus(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Plus"),  l, r) {};
    };

    class Minus : public BinOp {
    public:
        Minus(ASTNode &l, ASTNode &r) :
            BinOp(std::string("Minus"),  l, r) {};
    };

    class Times : public BinOp {
    public:
        Times(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Times"),  l, r) {};
    };

    class Div : public BinOp {
    public:
        Div(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Div"),  l, r) {};
    };

    // Condition expressions
    class And : public BinOp {
    public:
        And(ASTNode &l, ASTNode &r) :
                BinOp(std::string("And"),  l, r) {};
    };

    class Or : public BinOp {
    public:
        Or(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Or"),  l, r) {};
    };

    class Not : public ASTNode {
        ASTNode& left_;
    public:
        explicit Not(ASTNode &l) : left_{l} {}
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    // Comparing 
    class Compare : public BinOp {
    protected:
        std::string c_compare_op_;
        Compare(std::string sym,  std::string op, ASTNode &l, ASTNode &r) :
            BinOp(sym, l, r), c_compare_op_{op} {};
    };

    class Less : public Compare {
    public:
        Less(ASTNode &l, ASTNode &r) :
            Compare("Less", "<",  l, r) {};
    };

    class Less_E : public Compare {
    public:
        Less_E(ASTNode &l, ASTNode &r) :
                Compare("Less_E", "<=",  l, r) {};
    };

    class Greater_E : public Compare {
    public:
        Greater_E(ASTNode &l, ASTNode &r) :
                Compare("Greater_E", ">=",  l, r) {};
    };

    class Greater : public Compare {
    public:
        Greater(ASTNode &l, ASTNode &r) :
                Compare("Greater", ">", l, r) {};
    };

    class Equals : public Compare {
    public:
        Equals(ASTNode &l, ASTNode &r) :
                Compare("Equals", "==", l, r) {};
    };

    class Not_Equals : public Compare {
    public:
        Not_Equals(ASTNode &l, ASTNode &r) :
                Compare("Not Equals", "!=", l, r) {};
    };

    
    // Loops

    class While : public ASTNode {
        ASTNode &while_cond_;
        Block &while_body_;
    public:
        explicit While(ASTNode &cond, Block &body) :
            while_cond_{cond}, while_body_{body} {};
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    class For : public ASTNode {
        ASTNode &decl;
        ASTNode &cond;
        ASTNode &iter;
        Block &for_body;
    public:
        explicit For(ASTNode &d, ASTNode &c, ASTNode &i, Block &b) :
            decl{d}, cond{c}, iter{i}, for_body{b} {};
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    class CompExp: public ASTNode {
        ASTNode &ident;
        ASTNode &oper;
        ASTNode &val;
    public:
        explicit CompExp(ASTNode &i, ASTNode &o, ASTNode &v) :
            ident{i}, oper{o}, val{v} {};
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    // Functions

    class FuncDecl: public ASTNode {
        Block &params;
        Block &funcBody;
        ASTNode &expr;
    public:
        explicit FuncDecl(Block &func_params, Block &func_body, ASTNode &func_expr) :
            params{func_params}, funcBody{func_body}, expr{func_expr} {};
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    class FuncCall: public ASTNode {
        ASTNode &ident;
        Block &params;
    public:
        explicit FuncCall(ASTNode &func_ident, Block &func_params) :
            ident(func_ident), params{func_params} {};
        void json(std::ostream& out, AST_print_context& ctx) override;
    };


    // Arrays

    class ArrayEl : public BinOp {
    public:
        ArrayEl(ASTNode &l, ASTNode &r) :
                BinOp(std::string("ArrElem"),  l, r) {};
    };

    class ArrayAssign : public BinOp {
    public:
        ArrayAssign(ASTNode &l, ASTNode &r) :
                BinOp(std::string("ArrAssign"),  l, r) {};
    };

    class ArrayDecl : public ASTNode {
        Block &params;
    public:
        explicit ArrayDecl(Block &array_params) :
            params{array_params} {};
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    // Tuples

    class TupleEl : public BinOp {
    public:
        TupleEl(ASTNode &l, ASTNode &r) :
                BinOp(std::string("TuplElem"),  l, r) {};
    };

    class TupleDecl : public ASTNode {
        Block &params;
    public:
        explicit TupleDecl(Block &tuple_params) :
            params{tuple_params} {};
        void json(std::ostream& out, AST_print_context& ctx) override; 
    };
}
#endif /* AST_HPP */