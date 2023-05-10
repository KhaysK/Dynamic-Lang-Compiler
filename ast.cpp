#include "ast.hpp"
#include <stdlib.h>

namespace AST {
    MemObject *ASTNode::eval(MemoryKernel &mem) {
        return new MemObject(OBJECT_NULL, "", "null");
    }

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
        // if we try to change object which does not exist,
        // then panic and exit
        if (mod.getMod() == "assign" && !mem.get_object(this->name)) {
            std::cout << "Invalid reference to '" << this->name
                      << "': variable does not exist\n";
            exit(1);
        }

        MemObject* _eval = value.eval(mem);
        
        if (_eval->get_type() != OBJECT_FUNC) {
            mem.put_object(new MemObject(_eval->get_type(), this->name, _eval->get_value()));
        } else {
            MemFunction *_eval_f = dynamic_cast<MemFunction*>(_eval);
            MemFunction *to_put_f = new MemFunction(this->name,
                                                    _eval_f->get_entry_point(),
                                                    _eval_f->get_arg_names());
            mem.put_object(to_put_f);
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

        input_obj = new MemObject(t, name, input);
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

        // number + number = number
        if (left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_NUMBER) {
            double first, second;
            std::stringstream _l(left->get_value());
            std::stringstream _r(right->get_value());
            _l >> first;
            _r >> second;
            return new MemObject(OBJECT_NUMBER, "", std::to_string(first + second));
        }

        // string + string = string
        else if (left->get_type() == OBJECT_STRING && right->get_type() == OBJECT_STRING) {
            return new MemObject(OBJECT_STRING, "", left->get_value() + right->get_value());
        }

        // bool + bool = bool
        else if (left->get_type() == OBJECT_BOOL && right->get_type() == OBJECT_BOOL) {
            if (left->get_value() == "true" || right->get_value() == "true") {
                return new MemObject(OBJECT_BOOL, "", "true");
            }
            return new MemObject(OBJECT_BOOL, "", "false");
        }

        // null + null = null
        else if (left->get_type() == OBJECT_NULL && right->get_type() == OBJECT_NULL) {
            return new MemObject(OBJECT_NULL, "", "null");
        }

        // number + string = string
        else if (left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_STRING) {
            return new MemObject(OBJECT_STRING, "", left->get_value() + right->get_value());
        }
        else if (left->get_type() == OBJECT_STRING && right->get_type() == OBJECT_NUMBER) {
            return new MemObject(OBJECT_STRING, "", left->get_value() + right->get_value());
        }

        // number + bool = num + 0/1
        else if (left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_BOOL) {
            double first, second;
            std::stringstream _l(left->get_value());
            std::string _bool = "0";
            if (right->get_value() == "true") _bool = "1";
            std::stringstream _r(_bool);
            _l >> first;
            _r >> second;
            return new MemObject(OBJECT_NUMBER, "", std::to_string(first + second));
        }
        else if (left->get_type() == OBJECT_BOOL && right->get_type() == OBJECT_NUMBER) {
            double first, second;
            std::stringstream _l(right->get_value());
            std::string _bool = "0";
            if (left->get_value() == "true") _bool = "1";
            std::stringstream _r(_bool);
            _l >> first;
            _r >> second;
            return new MemObject(OBJECT_NUMBER, "", std::to_string(first + second));
        }

        // num + null = num
        else if (left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_NULL) {
            return new MemObject(OBJECT_NUMBER, "", left->get_value());
        }
        else if (left->get_type() == OBJECT_NULL && right->get_type() == OBJECT_NUMBER) {
            return new MemObject(OBJECT_NUMBER, "", right->get_value());
        }

        // string + bool = string
        else if (left->get_type() == OBJECT_STRING && right->get_type() == OBJECT_BOOL) {
            return new MemObject(OBJECT_STRING, "", left->get_value() + right->get_value());
        }
        else if (left->get_type() == OBJECT_BOOL && right->get_type() == OBJECT_STRING) {
            return new MemObject(OBJECT_STRING, "", left->get_value() + right->get_value());
        }

        // string + null = string
        else if (left->get_type() == OBJECT_STRING && right->get_type() == OBJECT_NULL) {
            return new MemObject(OBJECT_STRING, "", left->get_value());
        }
        else if (left->get_type() == OBJECT_NULL && right->get_type() == OBJECT_STRING) {
            return new MemObject(OBJECT_STRING, "", right->get_value());
        }

        // bool + null = bool
        else if (left->get_type() == OBJECT_BOOL && right->get_type() == OBJECT_NULL) {
            return new MemObject(OBJECT_BOOL, "", left->get_value());
        }
        else if (left->get_type() == OBJECT_NULL && right->get_type() == OBJECT_BOOL) {
            return new MemObject(OBJECT_BOOL, "", right->get_value());
        }

        // FIXME: throw error
        else {
            return new MemObject(OBJECT_NULL, "", "null");
        }
    }

    MemObject* Minus::eval(MemoryKernel& mem) {
        MemObject* left = left_.eval(mem);
        MemObject* right = right_.eval(mem);

        // number - number = number
        if (left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_NUMBER) {
            double first, second;
            std::stringstream _l(left->get_value());
            std::stringstream _r(right->get_value());
            _l >> first;
            _r >> second;
            return new MemObject(OBJECT_NUMBER, "", std::to_string(first - second));
        }

        // bool - bool = 0/1 - 0/1
        else if (left->get_type() == OBJECT_BOOL && right->get_type() == OBJECT_BOOL) {
            double first, second;
            std::string _bool_l = "0";
            if (left->get_value() == "true") _bool_l = "1";
            std::stringstream _l(_bool_l);
            std::string _bool_r = "0";
            if (right->get_value() == "true") _bool_r = "1";
            std::stringstream _r(_bool_r);
            _l >> first;
            _r >> second;
            return new MemObject(OBJECT_NUMBER, "", std::to_string(first - second));
        }

        // number - bool = num - 0/1
        else if (left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_BOOL) {
            double first, second;
            std::stringstream _l(left->get_value());
            std::string _bool = "0";
            if (right->get_value() == "true") _bool = "1";
            std::stringstream _r(_bool);
            _l >> first;
            _r >> second;
            return new MemObject(OBJECT_NUMBER, "", std::to_string(first - second));
        }
        else if (left->get_type() == OBJECT_BOOL && right->get_type() == OBJECT_NUMBER) {
            double first, second;
            std::stringstream _l(right->get_value());
            std::string _bool = "0";
            if (left->get_value() == "true") _bool = "1";
            std::stringstream _r(_bool);
            _l >> first;
            _r >> second;
            return new MemObject(OBJECT_NUMBER, "", std::to_string(first - second));
        }

        else {
            return new MemObject(OBJECT_NULL, "", "null");
        }
    }

    MemObject* Times::eval(MemoryKernel& mem){
        MemObject* left = left_.eval(mem);
        MemObject* right = right_.eval(mem);

        // number * number = number
        if (left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_NUMBER) {
            double first, second;
            std::stringstream _l(left->get_value());
            std::stringstream _r(right->get_value());
            _l >> first;
            _r >> second;
            return new MemObject(OBJECT_NUMBER, "", std::to_string(first * second));
        }

        // bool * bool = bool
        else if (left->get_type() == OBJECT_BOOL && right->get_type() == OBJECT_BOOL) {
            if (left->get_value() == "false" || right->get_value() == "false") {
                return new MemObject(OBJECT_BOOL, "", "false");
            }
            return new MemObject(OBJECT_NUMBER, "", "true");
        }

        // number * bool = num * 0/1
        else if (left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_BOOL) {
            double first, second;
            std::stringstream _l(left->get_value());
            std::string _bool = "0";
            if (right->get_value() == "true") _bool = "1";
            std::stringstream _r(_bool);
            _l >> first;
            _r >> second;
            return new MemObject(OBJECT_NUMBER, "", std::to_string(first * second));
        }
        else if (left->get_type() == OBJECT_BOOL && right->get_type() == OBJECT_NUMBER) {
            double first, second;
            std::stringstream _l(right->get_value());
            std::string _bool = "0";
            if (left->get_value() == "true") _bool = "1";
            std::stringstream _r(_bool);
            _l >> first;
            _r >> second;
            return new MemObject(OBJECT_NUMBER, "", std::to_string(first * second));
        }

        else {
            return new MemObject(OBJECT_NULL, "", "null");
        }
    }

    MemObject* Div::eval(MemoryKernel& mem){
        MemObject* left = left_.eval(mem);
        MemObject* right = right_.eval(mem);

        // number / number = number
        if (left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_NUMBER) {
            if (right->get_value() == "0") {
                // TODO: throw error
                return new MemObject(OBJECT_NULL, "", "null");
            }
            double first, second;
            std::stringstream _l(left->get_value());
            std::stringstream _r(right->get_value());
            _l >> first;
            _r >> second;
            return new MemObject(OBJECT_NUMBER, "", std::to_string(first / second));
        }
        
        // bool / bool = 0/1 / 0/1
        else if (left->get_type() == OBJECT_BOOL && right->get_type() == OBJECT_BOOL) {
            double first, second;
            std::string _bool_l = "0";
            if (left->get_value() == "true") _bool_l = "1";
            std::stringstream _l(_bool_l);
            std::string _bool_r = "0";
            if (right->get_value() == "true") _bool_r = "1";
            std::stringstream _r(_bool_r);
            if (_bool_r == "0") {
                // TODO: throw error
                return new MemObject(OBJECT_NULL, "", "null");
            }
            _l >> first;
            _r >> second;
            return new MemObject(OBJECT_NUMBER, "", std::to_string(first - second));
        }
        
        // number / bool = num / 0/1
        else if (left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_BOOL) {
            double first, second;
            std::stringstream _l(left->get_value());
            std::string _bool = "0";
            if (right->get_value() == "true") _bool = "1";
            if (_bool == "0") {
                // TODO: throw error
                return new MemObject(OBJECT_NULL, "", "null");
            }
            std::stringstream _r(_bool);
            _l >> first;
            _r >> second;
            return new MemObject(OBJECT_NUMBER, "", std::to_string(first - second));
        }
        else if (left->get_type() == OBJECT_BOOL && right->get_type() == OBJECT_NUMBER) {
            if (right->get_value() == "0") {
                // TODO: throw error
                return new MemObject(OBJECT_NULL, "", "null");
            }
            double first, second;
            std::stringstream _l(right->get_value());
            std::string _bool = "0";
            if (left->get_value() == "true") _bool = "1";
            std::stringstream _r(_bool);
            _l >> first;
            _r >> second;
            return new MemObject(OBJECT_NUMBER, "", std::to_string(first - second));
        }

        else {
            return new MemObject(OBJECT_NULL, "", "null");
        }
    };

    MemObject* Equals::eval(MemoryKernel& mem) {
        MemObject* left = left_.eval(mem);
        MemObject* right = right_.eval(mem);

        bool eq = false;

        if (left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_NUMBER){
            double _left, _right;
            std::stringstream _l, _r;
            _l << left->get_value();
            _r << right->get_value();

            _l >> _left;
            _r >> _right;
            eq = (_left == _right);
        } else if (left->get_value() == right->get_value()) {
            eq = true;
        }

        return new MemObject(OBJECT_BOOL, "", eq ? "true": "false");
    }

    MemObject* Not::eval(MemoryKernel& mem) {
        MemObject* _left = left.eval(mem);

        if (_left->get_value() == "true") {
            return new MemObject(OBJECT_BOOL, "", "false");
        }

        return new MemObject(OBJECT_BOOL, "", "true");
    }

    MemObject* Not_Equals::eval(MemoryKernel& mem) {
        MemObject* left = left_.eval(mem);
        MemObject* right = right_.eval(mem);

        MemObject* _equals = (new Equals(left_, right_))->eval(mem);

        if (_equals->get_value() == "true") {
            return new MemObject(OBJECT_BOOL, "", "false");
        }

        return new MemObject(OBJECT_BOOL, "", "true");
    }

    MemObject* And::eval(MemoryKernel& mem) {
        MemObject* left = left_.eval(mem);
        MemObject* right = right_.eval(mem);

        // bool and bool
        if (left->get_type() == OBJECT_BOOL && left->get_type() == OBJECT_BOOL) {
            return (new Times(left_, right_))->eval(mem);
        }
        // TODO:  Add number support 
        else {
            return new MemObject(OBJECT_BOOL, "", "false");
        }
    }

    MemObject* Or::eval(MemoryKernel& mem) {
        MemObject* left = left_.eval(mem);
        MemObject* right = right_.eval(mem);

        // bool and bool
        if (left->get_type() == OBJECT_BOOL && left->get_type() == OBJECT_BOOL) {
            return (new Plus(left_, right_))->eval(mem);
        }
        // TODO:  Add number support 
        else {
            return new MemObject(OBJECT_BOOL, "", "false");
        }
    }

    MemObject* Less::eval(MemoryKernel& mem) {
        MemObject* left = left_.eval(mem);
        MemObject* right = right_.eval(mem);

        // строки
        if (left->get_type() == OBJECT_STRING || right->get_type() == OBJECT_STRING) {
            if (left->get_value().compare(right->get_value()) < 0) {
                return new MemObject(OBJECT_BOOL, "", "true");
            }
        }
        // числа
        else if (left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_NUMBER) {
            MemObject* _subtract = (new Minus(left_, right_))->eval(mem);

            double _sub_value;
            std::stringstream _l(_subtract->get_value());
            _l >> _sub_value;
            if (_sub_value < 0) {
                return new MemObject(OBJECT_BOOL, "", "true");
            }
        }

        return new MemObject(OBJECT_BOOL, "", "false");
    }

    MemObject* Less_E::eval(MemoryKernel& mem) {
        MemObject* left = left_.eval(mem);
        MemObject* right = right_.eval(mem);

        Less _less = Less(left_, right_);
        Equals _equals = Equals(left_, right_);

        return (new Plus(_less, _equals))->eval(mem);
    }

    MemObject* Greater::eval(MemoryKernel& mem) {
        MemObject* left = left_.eval(mem);
        MemObject* right = right_.eval(mem);

        // строки
        if (left->get_type() == OBJECT_STRING || right->get_type() == OBJECT_STRING) {
            if (left->get_value().compare(right->get_value()) > 0) {
                return new MemObject(OBJECT_BOOL, "", "true");
            }
        }
        // числа
        if (left->get_type() == OBJECT_NUMBER && right->get_type() == OBJECT_NUMBER) {
            MemObject* _subtract = (new Minus(left_, right_))->eval(mem);

            double _sub_value;
            std::stringstream _l(_subtract->get_value());
            _l >> _sub_value;
            if (_sub_value > 0) {
                return new MemObject(OBJECT_BOOL, "", "true");
            }
        }

        return new MemObject(OBJECT_BOOL, "", "false");
    }

    MemObject* Greater_E::eval(MemoryKernel& mem) {
        MemObject* left = left_.eval(mem);
        MemObject* right = right_.eval(mem);

        Greater _greater = Greater(left_, right_);
        Equals _equals = Equals(left_, right_);

        return (new Plus(_greater, _equals))->eval(mem);
    }

    MemObject* While::eval(MemoryKernel& mem) {
        while (true) {
            MemObject* res = while_cond.eval(mem);

            if (res->get_type() == OBJECT_BOOL && res->get_value() == "false") break;
            else if (res->get_type() == OBJECT_NUMBER && res->get_value() == "0") break;
            else if (res->get_type() == OBJECT_NULL) break;
            while_block.eval(mem); 
        }
        return new MemObject(OBJECT_NULL, "", "null");
        
    }

    MemObject* FuncDecl::eval(MemoryKernel& mem) {
        if (mem.is_inside_func()){
            std::cout << "Can not declare function inside function\n";
            exit(1);
        }
        
        std::vector<std::string> args;
        for (auto p: this->params)
            args.push_back(p->eval(mem)->get_value());
        
        return new MemFunction("", &this->funcBody, args);
    }

    MemObject* FuncCall::eval(MemoryKernel& mem) {
        
        MemObject *obj = ident.eval(mem);
        MemFunction *func = dynamic_cast<MemFunction*>(obj);

        // mem.dump_mem();
        mem.enter_scope();
        mem.mark_inside_func();

        std::vector<MemObject*> to_call;
        std::vector<std::string> arg_names = func->get_arg_names();

        // This should be modified when arrays are implemented
        if (params.size() != arg_names.size()) {
            std::cout << func->get_name() << ": Invalid arguments. Aborting.\n";
            exit(1);
        }

        for (int i = 0; i < params.size(); ++i) {
            ASTNode *node = params[i];
            MemObject* eval_res = node->eval(mem);
            to_call.push_back(new MemObject(eval_res->get_type(), arg_names[i], eval_res->get_value()));
        }

        if (!func->prep_mem(mem, to_call)) {
            std::cout << func->get_name() << ": Invalid arguments. Aborting.\n";
            exit(1);
        }

        static_cast<Block*>(func->get_entry_point())->eval(mem);
        mem.exit_scope();
        mem.unmark_inside_func();

        return new MemObject(OBJECT_NULL, "", "null");

    }

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

    void Return::json(std::ostream& out, AST_print_context& ctx) {
        json_head("Return", out, ctx);
        json_child("return expr", expr, out, ctx);
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