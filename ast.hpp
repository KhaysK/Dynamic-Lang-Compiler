#ifndef AST_HPP
#define AST_HPP
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <unordered_map>
#include "MemoryKernel.hpp"

namespace AST {
    class AST_print_context {
    public:
        int indent_;
        AST_print_context() : indent_{0} {};
        void indent() { ++indent_; }
        void dedent() { --indent_; }
    };

    class eval_context {
    public:
        MemoryKernel mem;
        explicit eval_context() { }
    };

    /**
     * Кирпичик / база / основа
     * Элемент аст дерева, родитель всех нодов
     * 
     * Не объявляет внутри себя никакие переменные
     * Имеет только базовые функции для json аутпута
    */
    class ASTNode {
    public:
        virtual void json(std::ostream& out, AST_print_context& ctx) = 0;
        virtual std::string eval(eval_context& ctx) = 0;
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

    /**
     * Режим присваивания
     * 
     * Создание: 'var' или 'const'
     * Изменение значения: 'assign'
     * 
     * Имеет публичный геттер и сеттер
    **/
    class AssignMod : public ASTNode {
        std::string mod;
    public:
        explicit AssignMod(std::string txt) : mod{txt} {}
        void json(std::ostream& out, AST_print_context& ctx) override;
        void setMod(std::string new_mode) { mod = new_mode; }
        std::string getMod() { return mod; }
    };

    /**
     * Присваивание
     * 
     * mod: режим присваивания, (AssignMode)
     * name: имя переменной, (Ident)
     * value: значение переменной (Expression / Function Declaration)
     * 
     * имеет сеттер для mod
    */
    class Assign : public ASTNode {
        AssignMod &mod;
        ASTNode &name;
        ASTNode &value;
    public:
        Assign(AssignMod &mod, ASTNode &lexpr, ASTNode &rexpr) :
           mod{mod}, name{lexpr}, value{rexpr} {};
        void set(AssignMod& mod_) {
            mod.setMod(mod_.getMod());
        }
        void json(std::ostream& out, AST_print_context& ctx) override;
        std::string eval(eval_context& ctx){
            if(mod.getMod() != "assign"){
                if(dynamic_cast<StringConst*>(&value))
                    ctx.mem.put_object(new MemObject(OBJECT_STRING, name.eval(ctx), value.eval(ctx)));
                else if (dynamic_cast<NumberConst*>(&value))
                    ctx.mem.put_object(new MemObject(OBJECT_NUMBER, name.eval(ctx), value.eval(ctx)));
                else if (dynamic_cast<BoolConst*>(&value))
                    ctx.mem.put_object(new MemObject(OBJECT_BOOL, name.eval(ctx), value.eval(ctx)));
                else if (dynamic_cast<NullConst*>(&value))
                    ctx.mem.put_object(new MemObject(OBJECT_NULL, name.eval(ctx), value.eval(ctx)));
            }else if(mod.getMod() == "assign"){
                if(dynamic_cast<StringConst*>(&value)){
                    ctx.mem.get_object(name.eval(ctx))->set_value(value.eval(ctx));
                    ctx.mem.get_object(name.eval(ctx))->set_type(OBJECT_STRING);
                }
                else if (dynamic_cast<NumberConst*>(&value)){
                    ctx.mem.get_object(name.eval(ctx))->set_value(value.eval(ctx));
                    ctx.mem.get_object(name.eval(ctx))->set_type(OBJECT_NUMBER);
                }
                else if (dynamic_cast<BoolConst*>(&value)){
                    ctx.mem.get_object(name.eval(ctx))->set_value(value.eval(ctx));
                    ctx.mem.get_object(name.eval(ctx))->set_type(OBJECT_BOOL);
                }
            }
            return "Invalid operation";
        }
    };


    /**
     * Подразумевает область видимости переменных в коде
     * 
     * Представлен в виде массива (вектора) нодов
    */
    class Block : public ASTNode {
        std::vector<ASTNode*> nodes;
    public:
        explicit Block() : nodes{std::vector<ASTNode*>()} {}

        /**
         * Используется для assign
         * 
         * Пример: var x = 5, a = 10;
         * Это хранится как класс Block, только для удоства из-за массива
         * Но при добавлении в настоящий родительский Блок (область видимости),
         * нужно добавить по отдельности каждый assign, а не как один массив
        */
        void flat(Block* block) {
            for (auto &i : block->nodes) {
                nodes.push_back(i);
            }
        }
        /**
         * Испольуется для assign
         * 
         * Пример const x = 5, a = 10;
         * Изначально, из-за рекурсивного подхода бизона, код увидит const
         * только в самом конце
         * До этого уже будут созданы ноды для x = 5 и a = 10
         * Для удобства при создании ноды Assign автоматически присваиваем
         * режим 'assign', а потом если есть var или const пробегаемся по каждому
         * и проставляем правильный режим
        */
        void distribute(ASTNode* mod) {
            for (ASTNode *i: nodes) {
                Assign* assign = dynamic_cast<Assign*>(i);
                assign->set(*dynamic_cast<AssignMod*>(mod));
            }
        }
        /**
         * Добавляет новую ноду
        */
        void append(ASTNode* node) { nodes.push_back(node); }
        std::vector<ASTNode*> getNodes() { return nodes; }
        void json(std::ostream& out, AST_print_context& ctx) override;
        std::string eval(eval_context& ctx){
            ctx.mem.enter_scope();
            for(ASTNode* node: nodes){
                node->eval(ctx);
            }
            ctx.mem.exit_scope();
            return "Done";
        }
     };

    /**
     * Условное ветвление
     * 
     * сond: само условное выражение
     * true_block: блок, когда выражение - истинна
     * else_block: блок, когда выражение - ложь
    */
    class If : public ASTNode {
        ASTNode &cond;
        Block &true_block; 
        Block &else_block;
    public:
        explicit If(ASTNode &cond, Block &ifpart, Block &elsepart) :
            cond{cond}, true_block{ifpart}, else_block{elsepart} { };
        void json(std::ostream& out, AST_print_context& ctx) override;
        std::string eval(eval_context& ctx){
            if(dynamic_cast<BoolConst*>(&cond)){
                if (cond.eval(ctx) == "true")
                    return true_block.eval(ctx);
                else_block.eval(ctx);
            }else if(dynamic_cast<NumberConst*>(&cond)){
                if (cond.eval(ctx) != "0")
                    return true_block.eval(ctx);
                else_block.eval(ctx);
            }else if(dynamic_cast<StringConst*>(&cond)){
                if (cond.eval(ctx).length() > 0)
                    return true_block.eval(ctx);
                else_block.eval(ctx);
            }else {

            }
        }
    };

    /**
     * Принтуем какое-то выражение
    */
    class Print : public ASTNode {
        ASTNode& left;
    public:
        explicit Print(ASTNode &l) : left{l} {}
        void json(std::ostream& out, AST_print_context& ctx) override;
        std::string eval(eval_context& ctx){
            printf("%s\n", left.eval(ctx));
            return left.eval(ctx); // Returns printed value but it's useless anyway 
        }
    };

    // Leaf nodes

    /**
     * "Интерфейс" для классов с одним string value
     * Например Number, String, Bool
     * 
     * Передается имя класса (только для отображения и обычно уже
     * указан в конструкторе дочернего класса) и само значение
     * Почему такое используем: одинаковый тип значения + отображение
    */
    class LeafNode : public ASTNode {
    protected:
        std::string leaf_type;
        std::string value;
        LeafNode(std::string l_t, std::string v) :
                leaf_type{l_t}, value{v} {};
    public:
        void json(std::ostream& out, AST_print_context& ctx) override;
        std::string eval(eval_context& ctx){
            return value;
        }
        std::string get_type(){
            return leaf_type;
        }
    };

    /**
     * Число
     * 
     * Хранится в виде строки
    */
    class NumberConst : public LeafNode {
    public:
        NumberConst(std::string v) : 
            LeafNode(std::string("Number"), v) {};  
    };

    /**
     * Строка
    */
    class StringConst : public LeafNode {
    public:
        StringConst(std::string v) :
            LeafNode(std::string("String"), v) {};
    };

    /**
     * Булин
     * 
     * Хранится в виде строки
    */
    class BoolConst : public LeafNode {
    public:
        BoolConst(std::string v) :
            LeafNode(std::string("Bool"), v) {};
    };

    /**
     * Идентификатор (название переменной)
    */
    class Ident : public LeafNode {
    public:
        explicit Ident(std::string txt) :
            LeafNode(std::string("Ident"), txt) {};
        std::string eval(eval_context& ctx){
            return ctx.mem.get_object(value)->get_value();
        }
    };

    /**
     * Тип переменной
     * Например: "Int", "Real", "String"
     * 
     * Точно используется для ноды Read и IsOp
    */
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

    /**
     * Null
     * 
     * Не хранит абсолютно никакой информации
     * Реалньо null
    */
    class NullConst : public ASTNode {
    public:
        explicit NullConst() {}
        void json(std::ostream& out, AST_print_context& ctx) override;
        std::string get_type(){
            return "null";
        }
    };

    // Bin Operations

    /**
     * "Интерфейс" для нодов с разветвлением на 2 дочерние ноды
     * 
     * Например: мат. операции (+, -, *, /)
     * 
     * Передается имя класса (только для отображения и обычно уже
     * указан в конструкторе дочернего класса) и 2 ноды
    */
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

    /**
     * Считываем информацию с юзера
     * 
     * left: Тип считываемых данных (VarType)
     * right: Имя переменной (Ident)
     * 
     * Возможно, создать класс для каждого readInt, readReal, readSrting
     * удобнее, в таком случае call me)
    */
    class Read : public BinOp {
    public:
        Read(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Read"),  l, r) {};
    };

    /**
     * Проверка на тип переменной
     * 
     * left: Ident      TODO: Добавить поддержку любого expression слева
     * right: VarType
    */
    class IsOp : public BinOp {
    public:
        IsOp(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Is"),  l, r) {};
        std::string eval(eval_context& ctx) {
            MemObject* var = ctx.mem.get_object(dynamic_cast<Ident*>(&left_)->eval(ctx));
            if(var->get_type() == OBJECT_NUMBER && right_.eval(ctx) == "number"){
                return "true";
            } else if (var->get_type() == OBJECT_BOOL && right_.eval(ctx) == "bool"){
                return "true";
            } else if (var->get_type() == OBJECT_STRING && right_.eval(ctx) == "string"){
                return "true";
            } else if (var->get_type() == OBJECT_NULL && right_.eval(ctx) == "null"){
                return "true";
            }
            return "false";
        }; 
    };

    /**
     * Плюс
    */
    class Plus : public BinOp {
    public:
        Plus(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Plus"),  l, r) {};
        std::string eval(eval_context& ctx) {
            if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<NumberConst*>(&right_)){

                int result = std::stoi(left_.eval(ctx)) + std::stoi(right_.eval(ctx));
                return std::to_string(result);

            }else if(dynamic_cast<Ident*>(&left_) && dynamic_cast<Ident*>(&right_)){

                Ident* left = dynamic_cast<Ident*>(&left_);
                Ident* right = dynamic_cast<Ident*>(&right_);
                if(left->get_type() == "Number" && right->get_type() == "Number"){
                    int result = std::stoi(left_.eval(ctx)) + std::stoi(right_.eval(ctx));
                    return std::to_string(result);
                }

            }else if(dynamic_cast<Ident*>(&left_) && dynamic_cast<NumberConst*>(&right_)){

                Ident* left = dynamic_cast<Ident*>(&left_);
                if(left->get_type() == "Number"){
                    int result = std::stoi(left_.eval(ctx)) + std::stoi(right_.eval(ctx));
                    return std::to_string(result);
                }

            }else if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<Ident*>(&right_)){

                Ident* right = dynamic_cast<Ident*>(&right_);
                if(right->get_type() == "Number"){
                    int result = std::stoi(left_.eval(ctx)) + std::stoi(right_.eval(ctx));
                    return std::to_string(result);
                }

            } else if (dynamic_cast<BoolConst*>(&left_) || dynamic_cast<BoolConst*>(&right_) ||
                        dynamic_cast<Ident*>(&left_)->get_type() == "Bool" || 
                        dynamic_cast<Ident*>(&right_)->get_type() == "Bool"){

                return "Invalid operation";

            }
            return left_.eval(ctx) + right_.eval(ctx);
        }; 
    };

    /**
     * Минус
    */
    class Minus : public BinOp {
    public:
        Minus(ASTNode &l, ASTNode &r) :
            BinOp(std::string("Minus"),  l, r) {};
        std::string eval(eval_context& ctx) {
            if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<NumberConst*>(&right_)){

                int result = std::stoi(left_.eval(ctx)) - std::stoi(right_.eval(ctx));
                return std::to_string(result);

            }else if(dynamic_cast<Ident*>(&left_) && dynamic_cast<Ident*>(&right_)){

                Ident* left = dynamic_cast<Ident*>(&left_);
                Ident* right = dynamic_cast<Ident*>(&right_);
                if(left->get_type() == "Number" && right->get_type() == "Number"){
                    int result = std::stoi(left_.eval(ctx)) - std::stoi(right_.eval(ctx));
                    return std::to_string(result);
                }

            }else if(dynamic_cast<Ident*>(&left_) && dynamic_cast<NumberConst*>(&right_)){

                Ident* left = dynamic_cast<Ident*>(&left_);
                if(left->get_type() == "Number"){
                    int result = std::stoi(left_.eval(ctx)) - std::stoi(right_.eval(ctx));
                    return std::to_string(result);
                }

            }else if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<Ident*>(&right_)){

                Ident* right = dynamic_cast<Ident*>(&right_);
                if(right->get_type() == "Number"){
                    int result = std::stoi(left_.eval(ctx)) - std::stoi(right_.eval(ctx));
                    return std::to_string(result);
                }

            }
            return "Invalid operation";
        };
    };

    /**
     * Умножение
     * || 
                    dynamic_cast<Ident*>(&left_) && dynamic_cast<NumberConst*>(&left_) ||
                    dynamic_cast<NumberConst*>(&left_) && dynamic_cast<Ident*>(&left_)){
    */
    class Times : public BinOp {
    public:
        Times(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Times"),  l, r) {};
        std::string eval(eval_context& ctx) {
            if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<NumberConst*>(&right_)){
                
                int result = std::stoi(left_.eval(ctx)) * std::stoi(right_.eval(ctx));
                return std::to_string(result);

            }else if(dynamic_cast<Ident*>(&left_) && dynamic_cast<Ident*>(&right_)){

                Ident* left = dynamic_cast<Ident*>(&left_);
                Ident* right = dynamic_cast<Ident*>(&right_);
                if(left->get_type() == "Number" && right->get_type() == "Number"){
                    int result = std::stoi(left_.eval(ctx)) * std::stoi(right_.eval(ctx));
                    return std::to_string(result);
                }

            }else if(dynamic_cast<Ident*>(&left_) && dynamic_cast<NumberConst*>(&right_)){

                Ident* left = dynamic_cast<Ident*>(&left_);
                if(left->get_type() == "Number"){
                    int result = std::stoi(left_.eval(ctx)) * std::stoi(right_.eval(ctx));
                    return std::to_string(result);
                }

            }else if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<Ident*>(&right_)){

                Ident* right = dynamic_cast<Ident*>(&right_);
                if(right->get_type() == "Number"){
                    int result = std::stoi(left_.eval(ctx)) * std::stoi(right_.eval(ctx));
                    return std::to_string(result);
                }

            }
            return "Invalid operation";
        };
    };

    /**
     * Деление
    */
    class Div : public BinOp {
    public:
        Div(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Div"),  l, r) {};
        std::string eval(eval_context& ctx) {
            if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<NumberConst*>(&right_)){
                
                int result = std::stoi(left_.eval(ctx)) * std::stoi(right_.eval(ctx));
                return std::to_string(result);

            }else if(dynamic_cast<Ident*>(&left_) && dynamic_cast<Ident*>(&right_)){

                Ident* left = dynamic_cast<Ident*>(&left_);
                Ident* right = dynamic_cast<Ident*>(&right_);
                if(left->get_type() == "Number" && right->get_type() == "Number"){
                    int result = std::stoi(left_.eval(ctx)) / std::stoi(right_.eval(ctx));
                    return std::to_string(result);
                }

            }else if(dynamic_cast<Ident*>(&left_) && dynamic_cast<NumberConst*>(&right_)){

                Ident* left = dynamic_cast<Ident*>(&left_);
                if(left->get_type() == "Number"){
                    int result = std::stoi(left_.eval(ctx)) / std::stoi(right_.eval(ctx));
                    return std::to_string(result);
                }

            }else if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<Ident*>(&right_)){

                Ident* right = dynamic_cast<Ident*>(&right_);
                if(right->get_type() == "Number"){
                    int result = std::stoi(left_.eval(ctx)) / std::stoi(right_.eval(ctx));
                    return std::to_string(result);
                }

            }
            return "Invalid operation";
        };
    };

    // Condition expressions

    /**
     * Логическое И
    */
    class And : public BinOp {
    public:
        And(ASTNode &l, ASTNode &r) :
                BinOp(std::string("And"),  l, r) {};
        std::string eval(eval_context& ctx) {
            if(dynamic_cast<BoolConst*>(&left_) && dynamic_cast<BoolConst*>(&right_)){
                if (left_.eval(ctx) == "true" && right_.eval(ctx) == "true")
                    return "true";
                else return "false";
            }else if(dynamic_cast<Ident*>(&left_) && dynamic_cast<Ident*>(&right_)){

                Ident* left = dynamic_cast<Ident*>(&left_);
                Ident* right = dynamic_cast<Ident*>(&right_);
                if(left->get_type() == "Bool" && right->get_type() == "Bool"){
                    if (left_.eval(ctx) == "true" && right_.eval(ctx) == "true")
                        return "true";
                    else return "false";
                }else if(left->get_type() == "Number" && right->get_type() == "Number"){
                    if (!(left_.eval(ctx) == "0") && !(right_.eval(ctx) == "0"))
                        return "true";
                    else return "false";
                }

            }else if(dynamic_cast<Ident*>(&left_) && dynamic_cast<BoolConst*>(&right_)){

                Ident* left = dynamic_cast<Ident*>(&left_);
                if(left->get_type() == "Bool"){
                    if (left_.eval(ctx) == "true" && right_.eval(ctx) == "true")
                        return "true";
                    else return "false";
                }

            }else if(dynamic_cast<BoolConst*>(&left_) && dynamic_cast<Ident*>(&right_)){

                Ident* right = dynamic_cast<Ident*>(&right_);
                if(right->get_type() == "Bool"){
                    if (left_.eval(ctx) == "true" && right_.eval(ctx) == "true")
                        return "true";
                    else return "false";
                }

            }else if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<NumberConst*>(&right_)){
                
                if (!(left_.eval(ctx) == "0") && !(right_.eval(ctx) == "0"))
                    return "true";
                else return "false";

            }else if(dynamic_cast<Ident*>(&left_) && dynamic_cast<NumberConst*>(&right_)){

                Ident* left = dynamic_cast<Ident*>(&left_);
                if(left->get_type() == "Number"){
                    if (!(left_.eval(ctx) == "0") && !(right_.eval(ctx) == "0"))
                        return "true";
                    else return "false";
                }

            }else if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<Ident*>(&right_)){

                Ident* right = dynamic_cast<Ident*>(&right_);
                if(right->get_type() == "Number"){
                    if (!(left_.eval(ctx) == "0") && !(right_.eval(ctx) == "0"))
                        return "true";
                    else return "false";
                }

            }
            return "true";
        };
    };

    /**
     * Логическое ИЛИ
    */
    class Or : public BinOp {
    public:
        Or(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Or"),  l, r) {};
        std::string eval(eval_context& ctx) {
            if(dynamic_cast<BoolConst*>(&left_) && dynamic_cast<BoolConst*>(&right_)){
                if (left_.eval(ctx) == "true" || right_.eval(ctx) == "true")
                    return "true";
                else return "false";
            }else if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<NumberConst*>(&right_)){
                if (!(left_.eval(ctx) == "0") || !(right_.eval(ctx) == "0"))
                    return "true";
                else return "false";
            }else if(dynamic_cast<StringConst*>(&left_) && dynamic_cast<StringConst*>(&right_)){
                if (left_.eval(ctx).length() > 0 || right_.eval(ctx).length() > 0)
                    return "true";
                else return "false";
            }else {
                
            }
            return "Invalid operation";
        };
    };

    /**
     * Логическое Отрицание
    */
    class Not : public ASTNode {
        ASTNode& left;
    public:
        explicit Not(ASTNode &l) : left{l} {}
        void json(std::ostream& out, AST_print_context& ctx) override;
        std::string eval(eval_context& ctx) {
            if(dynamic_cast<BoolConst*>(&left)){
                if (left.eval(ctx) == "true")
                    return "false";
                else return "true";
            }else if(dynamic_cast<NumberConst*>(&left)){
                if (left.eval(ctx) == "0")
                    return "true";
                else return "false";
            }else if(dynamic_cast<StringConst*>(&left)){
                if (left.eval(ctx).length() > 0)
                    return "false";
                else return "true";
            }else {

            }
            return "Invalid operation";
        };
    };

    // Comparing 

    /**
     * "Интерфейс" для операций сравнение
     * 
     * c_compare_op: знак сравнения (нигде не используется, задается в конструкторе
     * автоматически)
    */
    class Compare : public BinOp {
    protected:
        std::string c_compare_op;
        Compare(std::string sym,  std::string op, ASTNode &l, ASTNode &r) :
            BinOp(sym, l, r), c_compare_op{op} {};
    };

    /**
     * Меньше
    */
    class Less : public Compare {
    public:
        Less(ASTNode &l, ASTNode &r) :
            Compare("Less", "<",  l, r) {};
        std::string eval(eval_context& ctx) {
            if(dynamic_cast<BoolConst*>(&left_) && dynamic_cast<BoolConst*>(&right_)){
                if (left_.eval(ctx) == "false" && right_.eval(ctx) == "true")
                    return "true";
                else return "false";
            }else if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<NumberConst*>(&right_)){
                if (std::stoi(left_.eval(ctx)) < std::stoi(right_.eval(ctx)))
                    return "true";
                else return "false";
            }else if(dynamic_cast<StringConst*>(&left_) && dynamic_cast<StringConst*>(&right_)){
                if (left_.eval(ctx) < right_.eval(ctx))
                    return "true";
                else return "false";
            }else {

            }
            return "Invalid operation";
        };
    };

    /**
     * Меньше или равно
    */
    class Less_E : public Compare {
    public:
        Less_E(ASTNode &l, ASTNode &r) :
                Compare("Less_E", "<=",  l, r) {};
        std::string eval(eval_context& ctx) {
            if(dynamic_cast<BoolConst*>(&left_) && dynamic_cast<BoolConst*>(&right_)){
                if (left_.eval(ctx) == "false" && right_.eval(ctx) == "true")
                    return "true";
                else if (left_.eval(ctx) == "true" && right_.eval(ctx) == "true" || 
                        left_.eval(ctx) == "false" && right_.eval(ctx) == "false")
                    return "true";
                else return "false";
            }else if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<NumberConst*>(&right_)){
                if (std::stoi(left_.eval(ctx)) <= std::stoi(right_.eval(ctx)))
                    return "true";
                else return "false";
            }else if(dynamic_cast<StringConst*>(&left_) && dynamic_cast<StringConst*>(&right_)){
                if (left_.eval(ctx) <= right_.eval(ctx))
                    return "true";
                else return "false";
            }else{

            }
            return "Invalid operation";
        };
    };

    /**
     * Больше или равно
    */
    class Greater_E : public Compare {
    public:
        Greater_E(ASTNode &l, ASTNode &r) :
                Compare("Greater_E", ">=",  l, r) {};
        std::string eval(eval_context& ctx) {
            if(dynamic_cast<BoolConst*>(&left_) && dynamic_cast<BoolConst*>(&right_)){
                if (left_.eval(ctx) == "true" && right_.eval(ctx) == "false")
                    return "true";
                else if (left_.eval(ctx) == "true" && right_.eval(ctx) == "true" || 
                        left_.eval(ctx) == "false" && right_.eval(ctx) == "false")
                    return "true";
                else return "false";
            }else if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<NumberConst*>(&right_)){
                if (std::stoi(left_.eval(ctx)) >= std::stoi(right_.eval(ctx)))
                    return "true";
                else return "false";
            }else if(dynamic_cast<StringConst*>(&left_) && dynamic_cast<StringConst*>(&right_)){
                if (left_.eval(ctx) >= right_.eval(ctx))
                    return "true";
                else return "false";
            }else {

            }
            return "Invalid operation";
        };
    };

    /**
     * Больше
    */
    class Greater : public Compare {
    public:
        Greater(ASTNode &l, ASTNode &r) :
                Compare("Greater", ">", l, r) {};
        std::string eval(eval_context& ctx) {
            if(dynamic_cast<BoolConst*>(&left_) && dynamic_cast<BoolConst*>(&right_)){
                if (left_.eval(ctx) == "true" && right_.eval(ctx) == "false")
                    return "true";
                else return "false";
            }else if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<NumberConst*>(&right_)){
                if (std::stoi(left_.eval(ctx)) > std::stoi(right_.eval(ctx)))
                    return "true";
                else return "false";
            }else if(dynamic_cast<StringConst*>(&left_) && dynamic_cast<StringConst*>(&right_)){
                if (left_.eval(ctx) > right_.eval(ctx))
                    return "true";
                else return "false";
            }else {

            }
            return "Invalid operation";
        };
    };

    /**
     * Равняется
    */
    class Equals : public Compare {
    public:
        Equals(ASTNode &l, ASTNode &r) :
                Compare("Equals", "==", l, r) {};
        std::string eval(eval_context& ctx) {
            if(dynamic_cast<BoolConst*>(&left_) && dynamic_cast<BoolConst*>(&right_)){
                if (left_.eval(ctx) == right_.eval(ctx))
                    return "true";
                else return "false";
            }else if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<NumberConst*>(&right_)){
                if (std::stoi(left_.eval(ctx)) == std::stoi(right_.eval(ctx)))
                    return "true";
                else return "false";
            }else if(dynamic_cast<StringConst*>(&left_) && dynamic_cast<StringConst*>(&right_)){
                if (left_.eval(ctx) == right_.eval(ctx))
                    return "true";
                else return "false";
            }else{

            }
            return "Invalid operation";
        };
    };

    /**
     * Не равняется
    */
    class Not_Equals : public Compare {
    public:
        Not_Equals(ASTNode &l, ASTNode &r) :
                Compare("Not Equals", "!=", l, r) {};
        std::string eval(eval_context& ctx) {
            if(dynamic_cast<BoolConst*>(&left_) && dynamic_cast<BoolConst*>(&right_)){
                if (left_.eval(ctx) != right_.eval(ctx))
                    return "true";
                else return "false";
            }else if(dynamic_cast<NumberConst*>(&left_) && dynamic_cast<NumberConst*>(&right_)){
                if (std::stoi(left_.eval(ctx)) != std::stoi(right_.eval(ctx)))
                    return "true";
                else return "false";
            }else if(dynamic_cast<StringConst*>(&left_) && dynamic_cast<StringConst*>(&right_)){
                if (left_.eval(ctx) != right_.eval(ctx))
                    return "true";
                else return "false";
            }else{

            }
            return "Invalid operation";
        };
    };

    
    // Loops

    /**
     * While
     * 
     * while_cond: условие, при котором блок выполняется
     * while_block: блок вайла
    */
    class While : public ASTNode {
        ASTNode &while_cond;
        Block &while_block;
    public:
        explicit While(ASTNode &cond, Block &body) :
            while_cond{cond}, while_block{body} {};
        void json(std::ostream& out, AST_print_context& ctx) override;
        std::string eval(eval_context& ctx){
            while (true)
            {
                if(dynamic_cast<BoolConst*>(&while_cond)){
                    if (while_cond.eval(ctx) == "true")
                        while_block.eval(ctx);
                    else break;
                }else if(dynamic_cast<NumberConst*>(&while_cond)){
                    if (while_cond.eval(ctx) != "0")
                        while_block.eval(ctx);
                    else break;
                }else if(dynamic_cast<StringConst*>(&while_cond)){
                    if (while_cond.eval(ctx).length() > 0)
                        while_block.eval(ctx);
                    else break; 
                }else {

                }
            }
            
            
        }
    };

    /**
     * For
     * 
     * decl: создание переменных для цикла for
     * cond: условие, пока истина, цикл итерируется
     * iter: ???? (CompExp) пока только ввиде ИДЕНТ (+, -, /, *)= ЗНАЧЕНИЕ
     * for_block: block цикла
     * 
     * При создании задаем условие, операция итерации, блок
     * А создание переменных, через flat, как в случае с Block и assigns
    */
    class For : public ASTNode {
        std::vector<ASTNode*> nodes;
        ASTNode &cond;
        ASTNode &iter;
        Block &for_block;
    public:
        explicit For(ASTNode &c, ASTNode &i, Block &b) :
            cond{c}, iter{i}, for_block{b} {};
        void flat(Block* block) {
            for (auto &i : block->getNodes()) {
                nodes.push_back(i);
            }
        }
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    /**
     * Computational Expression для Фора
     * 3 штука в создании фора, операция, которая выполняется при каждой итерации
     * 
     * FIXME: меня не должно существовать 
    */
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

    /**
     * Объявление функции
     * 
     * params: вектор переменных (Ident)
     * funcBlock: блок функции
     * expr: выражение, которое вернет функция (return)
     * 
     * flat: как обычно, ничего не поменялось, не используем Block в чистом виде
    */
    class FuncDecl: public ASTNode {
        std::vector<ASTNode*> params;
        Block &funcBody;
        ASTNode &expr;
    public:
        explicit FuncDecl(Block &func_body, ASTNode &func_expr) :
            funcBody{func_body}, expr{func_expr} {};
        void flat(Block* block) {
            for (auto &i : block->getNodes()) {
                params.push_back(i);
            }
        }
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    /**
     * Вызов функции
     * 
     * ident: имя вызываемой функции
     * params: вектор передаваемых выражений (переменная, мат. вычисление)
    */
    class FuncCall: public ASTNode {
        ASTNode &ident;
        std::vector<ASTNode*> params;
    public:
        explicit FuncCall(ASTNode &func_ident) :
            ident(func_ident) {};
        void flat(Block* block) {
            for (auto &i : block->getNodes()) {
                params.push_back(i);
            }
        }
        void json(std::ostream& out, AST_print_context& ctx) override;
    };


    // Arrays

    /**
     * Оюращение к элементу массива
     * 
     * left: имя массива (Ident('arr'))
     * right: индекс элемента (Number(15))
    */
    class ArrayEl : public BinOp {
    public:
        ArrayEl(ASTNode &l, ASTNode &r) :
                BinOp(std::string("ArrElem"),  l, r) {};
    };

    /**
     * Объявление массива
     * 
     * Пример [ a, kek, '1323', 1231, a(22) ]
     * Хранит в себе вектор этих элементов массива
    */
    class ArrayDecl : public ASTNode {
        std::vector<ASTNode*> params;
    public:
        ArrayDecl() {};
        void flat(Block* block) {
            for (auto &i : block->getNodes()) {
                params.push_back(i);
            }
        }
        void json(std::ostream& out, AST_print_context& ctx) override;
    };

    // Tuples

    /**
     * Обращение к элементу тюпла
     * 
     * left: имя тюпла (Ident)
     * right: имя элемента (Ident) либо через индекс (Number)
    */
    class TupleEl : public BinOp {
    public:
        TupleEl(ASTNode &l, ASTNode &r) :
                BinOp(std::string("TuplElem"),  l, r) {};
    };

    /**
     * Объявление тюпла
     * { a=15, b=7, c='123', d=a(123) }
     * Хранит в себе вектор этих элементов тюпла
    */
    class TupleDecl : public ASTNode {
        std::vector<ASTNode*> params;
    public:
        TupleDecl() {};
        void flat(Block* block) {
            for (auto &i : block->getNodes()) {
                params.push_back(i);
            }
        }
        void json(std::ostream& out, AST_print_context& ctx) override; 
    };
}
#endif /* AST_HPP */