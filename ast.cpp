#include "ast.hpp"
#include <stdlib.h>

namespace AST {
    void ASTNode::json_indent(std::ostream& out, AST_print_context& ctx) {
        if (ctx.indent_ > 0) {
            out << std::endl;
        }
        for (int i=0; i < ctx.indent_; ++i) {
            out << "    ";
        }
    }

    void ASTNode::json_head(std::string node_kind, std::ostream& out, AST_print_context& ctx) {
        json_indent(out, ctx);
        out << "{ \"type\" : \"" << node_kind << "\"," ;
        ctx.indent();  // one level more for children
        return;
    }

    void ASTNode::json_close(std::ostream& out, AST_print_context& ctx) {
        out << "}";
        ctx.dedent();
    }

    void ASTNode::json_child(std::string field, ASTNode& child, std::ostream& out, AST_print_context& ctx, char sep) {
        json_indent(out, ctx);
        out << "\"" << field << "\" : ";
        child.json(out, ctx);
        out << sep;
    }


    void Block::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Block", out, ctx);
        out << "\"nodes\" : [";
        auto sep = "";
        for (ASTNode *stmt: nodes) {
            out << sep;
            stmt->json(out, ctx);
            sep = ", ";
        }
        out << "]";
        json_close(out, ctx);
    }

    void Assign::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Assign", out, ctx);
        json_child("mod", mod, out, ctx);
        json_child("name", name, out, ctx);
        json_child("value", value, out, ctx, ' ');
        json_close(out, ctx);
     }

    void If::json(std::ostream& out, AST_print_context& ctx) {
        json_head("If", out, ctx);
        json_child("condition", cond, out, ctx);
        json_child("true_block", true_block, out, ctx);
        json_child("else_block", else_block, out, ctx, ' ');
        json_close(out, ctx);
    }

    void While::json(std::ostream& out, AST_print_context& ctx) {
        json_head("While", out, ctx);
        json_child("while_condition", while_cond, out, ctx);
        json_child("while_block", while_block, out, ctx);
        json_close(out, ctx);
    }

    void For::json(std::ostream& out, AST_print_context& ctx) {
        json_head("For", out, ctx);
        out << "\"assigns\" : [";
        auto sep = "";
        for (ASTNode *stmt: nodes) {
            out << sep;
            stmt->json(out, ctx);
            sep = ", ";
        }
        out << "]";
        json_child("conditions", cond, out, ctx);
        json_child("iterations", iter, out, ctx);
        json_child("for_block", for_block, out, ctx);
        json_close(out, ctx);
    }

    void CompExp::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Comp Exp", out, ctx);
        json_child("ident", ident, out, ctx);
        json_child("operation", oper, out, ctx);
        json_child("val", val, out, ctx);
        json_close(out, ctx);
    }

    void FuncDecl::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Func Declaration", out, ctx);
        out << "\"params\" : [";
        auto sep = "";
        for (ASTNode *stmt: params) {
            out << sep;
            stmt->json(out, ctx);
            sep = ", ";
        }
        out << "]";
        json_child("func_body", funcBody, out, ctx);
        json_child("func_expr", expr, out, ctx);
        json_close(out, ctx);
    }

    void FuncCall::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Func Call", out, ctx);
        json_child("func ident", ident, out, ctx);
        out << "\"call params\" : [";
        auto sep = "";
        for (ASTNode *stmt: params) {
            out << sep;
            stmt->json(out, ctx);
            sep = ", ";
        }
        out << "]";
        json_close(out, ctx);
    }

    void ArrayDecl::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Array Decl", out, ctx);
        out << "\"array params\" : [";
        auto sep = "";
        for (ASTNode *stmt: params) {
            out << sep;
            stmt->json(out, ctx);
            sep = ", ";
        }
        out << "]";
        json_close(out, ctx);
    }

    void TupleDecl::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Tuple Decl", out, ctx);
        out << "\"tuple params\" : [";
        auto sep = "";
        for (ASTNode *stmt: params) {
            out << sep;
            stmt->json(out, ctx);
            sep = ", ";
        }
        out << "]";
        json_close(out, ctx);
    }

    void Not::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Not", out, ctx);
        json_child("left", left, out, ctx);
        json_close(out, ctx);
    }

    void Print::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Print", out, ctx);
        json_child("left", left, out, ctx);
        json_close(out, ctx);
    }

    void AssignMod::json(std::ostream& out, AST_print_context& ctx) {
        json_head("AssignMod", out, ctx);
        out << "\"assign mode\" : \"" << mod << "\"";
        json_close(out, ctx);
    }

    void LeafNode::json(std::ostream& out, AST_print_context& ctx) {
        json_head(leaf_type, out, ctx);
        out << "\"value\" : \"" << value << "\"";
        json_close(out, ctx);
    }

    void NullConst::json(std::ostream& out, AST_print_context& ctx) {
        json_head("NullConst", out, ctx);
        out << "null";
        json_close(out, ctx);
    }

    void BinOp::json(std::ostream& out, AST_print_context& ctx) {
        json_head(opsym, out, ctx);
        json_child("left", left_, out, ctx);
        json_child("right", right_, out, ctx, ' ');
        json_close(out, ctx);
    }

}