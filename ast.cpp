#include "ast.hpp"
#include <stdlib.h>

namespace AST {
    MemObject* NullConst::eval(MemoryKernel& mem){
        return new MemObject(OBJECT_NULL, "", "null");
    }

    MemObject* NumberConst::eval(MemoryKernel& mem){
        return new MemObject(OBJECT_NUMBER, "", value);
    }

    MemObject* StringConst::eval(MemoryKernel& mem){
        return new MemObject(OBJECT_STRING, "", value);
    }

    MemObject* BoolConst::eval(MemoryKernel& mem){
        return new MemObject(OBJECT_BOOL, "", value);
    }

    MemObject* Ident::eval(MemoryKernel& mem){
        return mem.get_object(value);
    }

    MemObject* VarType::eval(MemoryKernel& mem){
        
        if(value == "string")
            return new MemObject(OBJECT_STRING, "", "");
        else if(value == "bool")
            return new MemObject(OBJECT_BOOL, "", "");
        else if(value == "number")
            return new MemObject(OBJECT_NUMBER, "", "");
        
        return new MemObject(OBJECT_NULL, "", "");
    }

    MemObject* Assign::eval(MemoryKernel& mem){
        if(mod.getMod() != "assign"){
            
            if (dynamic_cast<FuncDecl*>(&value)) {
                if (mem.is_inside_func()){
                    std::cout << "Can not declare function inside function\n";
                    exit(1);
                }
                
                FuncDecl *fd = dynamic_cast<FuncDecl*>(&value);
                std::vector<std::string> args;
                for (auto p: fd->params)
                    args.push_back(dynamic_cast<Ident*>(p)->eval(mem)->get_name());
                
                MemFunction *func = new MemFunction(name, &fd->funcBody, args);
                mem.put_object(func);
            } else{
                MemObject* assign_value = value.eval(mem);
                mem.put_object(new MemObject(assign_value->get_type(), name, assign_value->get_value()));
            }
        }else{

            if (dynamic_cast<FuncDecl*>(&value)) {
                FuncDecl *fd = dynamic_cast<FuncDecl*>(&value);
                std::vector<std::string> args;
                for (auto p: fd->params)
                    args.push_back(dynamic_cast<Ident*>(p)->eval(mem)->get_name());
                
                MemFunction *func = new MemFunction(name, &fd->funcBody, args);
                mem.put_object(func);
            } else {
                MemObject* assign_value = value.eval(mem);
                mem.get_object(name)->set_value(assign_value->get_value());
                mem.get_object(name)->set_type(assign_value->get_type());
            }
        }
        return new MemObject(OBJECT_NULL, "", "null");
    }

    MemObject* Block::eval(MemoryKernel& mem) {
        mem.enter_scope();
        for(ASTNode* node: nodes){
            node->eval(mem);
            // mem.dump_mem();
        }
        mem.exit_scope();
        return new MemObject(OBJECT_NULL, "", "null");
    }

    MemObject* If::eval(MemoryKernel& mem) {
        
        MemObject* if_cond = cond.eval(mem);
        if(if_cond->get_type() == OBJECT_BOOL && if_cond->get_value() == "false"){

            return else_block.eval(mem);
            
        }else if(if_cond->get_type() == OBJECT_NUMBER && if_cond->get_value() == "0"){
            
            return else_block.eval(mem);

        }else if(if_cond->get_type() == OBJECT_NULL){

            return else_block.eval(mem);
        }

        return true_block.eval(mem);
    }

    MemObject* Print::eval(MemoryKernel& mem) {
        std::cout<<left.eval(mem)->get_value()<<"\n";
        return new MemObject(OBJECT_NULL, "", "null");
    }

    MemObject* Read::eval(MemoryKernel& mem){
        std::string input;
        std::cin>>input;
        ObjectType t = OBJECT_STRING;
        MemObject* input_obj;

        if(type.eval(mem)->get_type() == OBJECT_NUMBER)
            t = OBJECT_NUMBER;

        input_obj = new MemObject(t, "", input);
        mem.put_object(input_obj);
        
        return input_obj;
    }

    MemObject* IsOp::eval(MemoryKernel& mem){
        MemObject* var = left_.eval(mem);
        MemObject* type = right_.eval(mem);
        
        if(var->get_type() == OBJECT_NUMBER && type->get_type() == OBJECT_NUMBER){
            return new MemObject(OBJECT_BOOL, "", "true");
        } else if (var->get_type() == OBJECT_BOOL && type->get_type() == OBJECT_BOOL){
            return new MemObject(OBJECT_BOOL, "", "true");
        } else if (var->get_type() == OBJECT_STRING && type->get_type() == OBJECT_STRING){
            return new MemObject(OBJECT_BOOL, "", "true");
        } else if (var->get_type() == OBJECT_NULL && type->get_type() == OBJECT_NULL){
            return new MemObject(OBJECT_BOOL, "", "true");
        }
        return new MemObject(OBJECT_BOOL, "", "false");
    }

    MemObject* Plus::eval(MemoryKernel& mem) {
        MemObject* left = left_.eval(mem);
        MemObject* right = right_.eval(mem);

        if(left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_NUMBER){

            int result = std::stoi(left->get_value()) + std::stoi(right->get_value());
            return new MemObject(OBJECT_NUMBER, "", std::to_string(result));

        }else if(left->get_type() == OBJECT_BOOL && right->get_type() == OBJECT_BOOL){
            
            if(left->get_value() == "true" || right->get_value() == "true")
                return new MemObject(OBJECT_BOOL, "", "true"); 
            else 
                return new MemObject(OBJECT_BOOL, "", "false"); 
            
        }else if(left->get_type() == OBJECT_BOOL && right->get_type() == OBJECT_NUMBER ||
                left->get_type() == OBJECT_BOOL && right->get_type() == OBJECT_NUMBER ){
            
            return new MemObject(OBJECT_NULL, "", "null"); 
        }

        return new MemObject(OBJECT_STRING, "", left->get_value() + right->get_value()); 
    }

    MemObject* Minus::eval(MemoryKernel& mem) {
        MemObject* left = left_.eval(mem);
        MemObject* right = right_.eval(mem);

        if(left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_NUMBER){

            int result = std::stoi(left->get_value()) - std::stoi(right->get_value());
            return new MemObject(OBJECT_NUMBER, "", std::to_string(result));
        }
        
        return new MemObject(OBJECT_NULL, "", "null"); 
    }

    MemObject* Times::eval(MemoryKernel& mem){
        MemObject* left = left_.eval(mem);
        MemObject* right = right_.eval(mem);

        if(left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_NUMBER){

            int result = std::stoi(left->get_value()) * std::stoi(right->get_value());
            return new MemObject(OBJECT_NUMBER, "", std::to_string(result));
        }
        
        return new MemObject(OBJECT_NULL, "", "null"); 
    }

    MemObject* Div::eval(MemoryKernel& mem){
        MemObject* left = left_.eval(mem);
        MemObject* right = right_.eval(mem);

        if(left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_NUMBER){

            int result = std::stoi(left->get_value()) / std::stoi(right->get_value());
            return new MemObject(OBJECT_NUMBER, "", std::to_string(result));
        }
        
        return new MemObject(OBJECT_NULL, "", "null"); 
    };

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
        out << "\"name\" : \"" << name << "\"";
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

    void Read::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Read", out, ctx);
        json_child("VarType", type, out, ctx);
        out << "\"name\" : \"" << name << "\"";
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