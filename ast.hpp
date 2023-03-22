#ifndef AST_HPP
#define AST_HPP
//
// Created by Michal Young on 9/12/18.

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <assert.h>

namespace AST {
    // Abstract syntax tree.  ASTNode is abstract base class for all other nodes.

    // Json conversion and pretty-printing can pass around a print context object
    // to keep track of indentation, and possibly other things.
    class AST_print_context {
    public:
        int indent_; // Number of spaces to place on left, after each newline
        AST_print_context() : indent_{0} {};
        void indent() { ++indent_; }
        void dedent() { --indent_; }
    };


    class ASTNode {
    public:
        /* Dump JSON representation */
        virtual void json(std::ostream& out, AST_print_context& ctx) = 0;

        std::string str() {  // String representation is JSON
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

    /* A block is a sequence of statements or expressions.
     * For simplicity we'll just make it a sequence of ASTNode,
     * and leave it to the parser to build valid structures.
     */
    class Block : public ASTNode {
        std::vector<ASTNode*> stmts_;
    public:
        explicit Block() : stmts_{std::vector<ASTNode*>()} {}
        void append(ASTNode* stmt) { stmts_.push_back(stmt); }
        void json(std::ostream& out, AST_print_context& ctx) override;
     };

    /* L_Expr nodes are AST nodes that can be evaluated for location.
     * Most can also be evaluated for value_.  An example of an L_Expr
     * is an identifier, which can appear on the left_ hand or right_ hand
     * side of an assignment.  For example, in x = y, x is evaluated for
     * location and y is evaluated for value_.
     *
     * For now, a location is just a name, because that's what we index
     * the symbol table with.  In a full compiler, locations can be
     * more complex, and typically in code generation we would have
     * LExpr evaluate to an address in a register.
     *
     * LExpr is abstract.  It's only concrete subclass for now is Ident,
     * but in a full OO language we would have LExprs that look like
     * a.b and a[2].
     */

    /* An assignment has an lvalue (location to be assigned to)
     * and an expression.  We evaluate the expression and place
     * the value_ in the variable.
     */

    class Assign : public ASTNode {
        ASTNode &lexpr_;
        ASTNode &rexpr_;
    public:
        Assign(ASTNode &lexpr, ASTNode &rexpr) :
           lexpr_{lexpr}, rexpr_{rexpr} {};
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    class If : public ASTNode {
        ASTNode &cond_; // The boolean expression to be evaluated
        Block &truepart_; // Execute this block if the condition is true
        Block &falsepart_; // Execute this block if the condition is false
    public:
        explicit If(ASTNode &cond, Block &truepart, Block &falsepart) :
            cond_{cond}, truepart_{truepart}, falsepart_{falsepart} { };
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    /* We need a node to represent interpretation of an r-expression
     * as a boolean.  While r-expressions have a gen_rvalue method,
     * we need a gen_branch method for r-expressions that are
     * interpreted as booleans.
     */
    class AsBool : public ASTNode {
        ASTNode &left_;
    public:
        explicit AsBool(ASTNode &left) : left_{left} {}
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    /* Identifiers like x and literals like 42 are the
     * leaves of the AST.  A literal can only be evaluated
     * for value_ (the 'eval' method), but an identifier
     * can also be evaluated for location (when we want to
     * store something in it).
     */
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

    class NullConst : public ASTNode {
    public:
        explicit NullConst() {}
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    // Virtual base class for +, -, *, /, etc
    class BinOp : public ASTNode {
        // each subclass must override the inherited
        // eval() method

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
        Div (ASTNode &l, ASTNode &r) :
                BinOp(std::string("Div"),  l, r) {};
    };

    // Boolean combinations and, or, not are short-circuit evaluated.
    // The BinOp superclass will be ok, be we need to add a
    // gen_branch method for each.
    class And : public BinOp {
    public:
        And (ASTNode &l, ASTNode &r) :
                BinOp(std::string("And"),  l, r) {};
    };

    class Or : public BinOp {
    public:
        Or (ASTNode &l, ASTNode &r) :
                BinOp(std::string("Or"),  l, r) {};
    };

    class Not : public ASTNode {
        ASTNode& left_;
    public:
        explicit Not(ASTNode &l) : left_{l} {}
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    class Print : public ASTNode {
        ASTNode& left_;
    public:
        explicit Print(ASTNode &l) : left_{l} {}
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    // For comparisons, we want to factor out a bit more of the common
    // code generation

    class Compare : public BinOp {
    protected:
        std::string c_compare_op_;
        Compare(std::string sym,  std::string op, ASTNode &l, ASTNode &r) :
            BinOp(sym, l, r), c_compare_op_{op} {};
    };

    class Less : public Compare {
    public:
        Less (ASTNode &l, ASTNode &r) :
            Compare("Less", "<",  l, r) {};
    };

    class AtMost : public Compare {
    public:
        AtMost (ASTNode &l, ASTNode &r) :
                Compare("AtMost", "<=",  l, r) {};
    };

    class AtLeast : public Compare {
    public:
        AtLeast (ASTNode &l, ASTNode &r) :
                Compare("AtLeast", ">=",  l, r) {};
    };

    class Greater : public Compare {
    public:
        Greater (ASTNode &l, ASTNode &r) :
                Compare("Less", "<", l, r) {};
    };

    class Equals : public Compare {
    public:
        Equals (ASTNode &l, ASTNode &r) :
                Compare("Equals", "==", l, r) {};
    };


}
#endif /* AST_HPP */