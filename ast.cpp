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
        out << "\"stmts_\" : [";
        auto sep = "";
        for (ASTNode *stmt: stmts_) {
            out << sep;
            stmt->json(out, ctx);
            sep = ", ";
        }
        out << "]";
        json_close(out, ctx);
    }

    void Assign::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Assign", out, ctx);
        json_child("lexpr_", lexpr_, out, ctx);
        json_child("rexpr_", rexpr_, out, ctx, ' ');
        json_close(out, ctx);
     }

    void If::json(std::ostream& out, AST_print_context& ctx) {
        json_head("If", out, ctx);
        json_child("cond_", cond_, out, ctx);
        json_child("true_part_", ifpart_, out, ctx);
        json_child("else_part_", elsepart_, out, ctx, ' ');
        json_close(out, ctx);
    }

    void While::json(std::ostream& out, AST_print_context& ctx) {
        json_head("While", out, ctx);
        json_child("while_cond_", while_cond_, out, ctx);
        json_child("while_body_", while_body_, out, ctx);
        json_close(out, ctx);
    }

    void For::json(std::ostream& out, AST_print_context& ctx) {
        json_head("For", out, ctx);
        json_child("decl_", decl, out, ctx);
        json_child("cond_", cond, out, ctx);
        json_child("iter_", iter, out, ctx);
        json_child("body_", for_body, out, ctx);
        json_close(out, ctx);
    }

    void CompExp::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Comp Exp", out, ctx);
        json_child("ident", ident, out, ctx);
        json_child("operation", oper, out, ctx);
        json_child("val", val, out, ctx);
        json_close(out, ctx);
    }

    void Not::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Not", out, ctx);
        json_child("left_", left_, out, ctx);
        json_close(out, ctx);
    }

    void Print::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Print", out, ctx);
        json_child("left_", left_, out, ctx);
        json_close(out, ctx);
    }

    void AsBool::json(std::ostream& out, AST_print_context& ctx) {
        json_head("AsBool", out, ctx);
        json_child("left_", left_, out, ctx);
        json_close(out, ctx);
    }

    void Ident::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Ident", out, ctx);
        out << "\"text_\" : \"" << text_ << "\"";
        json_close(out, ctx);
    }

    void LeafNode::json(std::ostream& out, AST_print_context& ctx) {
        json_head(leaf_type, out, ctx);
        out << "\"value_\" : \"" << value_ << "\"";
        json_close(out, ctx);
    }

    void NullConst::json(std::ostream& out, AST_print_context& ctx) {
        json_head("NullConst", out, ctx);
        out << "null";
        json_close(out, ctx);
    }

    void BinOp::json(std::ostream& out, AST_print_context& ctx) {
        json_head(opsym, out, ctx);
        json_child("left_", left_, out, ctx);
        json_child("right_", right_, out, ctx, ' ');
        json_close(out, ctx);
    }

}