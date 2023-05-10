#ifndef AST_HPP
#define AST_HPP
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <assert.h>
#include <stdio.h>
#include "MemoryKernel.hpp"

namespace AST {
    class AST_print_context {
    public:
        int indent_;
        AST_print_context() : indent_{0} {};
        void indent() { ++indent_; }
        void dedent() { --indent_; }
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
        virtual void json(std::ostream& out, AST_print_context& mem) = 0;
        virtual MemObject* eval(MemoryKernel& mem);
        std::string str() {
            std::stringstream ss;
            AST_print_context mem;
            json(ss, mem);
            return ss.str();
        }
    protected:
        void json_indent(std::ostream& out, AST_print_context& mem);
        void json_head(std::string node_kind, std::ostream& out, AST_print_context& mem);
        void json_close(std::ostream& out, AST_print_context& mem);
        void json_child(std::string field, ASTNode& child, std::ostream& out, AST_print_context& mem, char sep=',');
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
        void json(std::ostream& out, AST_print_context& mem) override;
        MemObject* eval(MemoryKernel& mem) override;
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
        void json(std::ostream& out, AST_print_context& mem) override;
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
        MemObject* eval(MemoryKernel& mem) override;
    };

    /**
     * Строка
    */
    class StringConst : public LeafNode {
    public:
        StringConst(std::string v) :
            LeafNode(std::string("String"), v) {};
        MemObject* eval(MemoryKernel& mem) override;
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
        MemObject* eval(MemoryKernel& mem) override;
    };

    /**
     * Идентификатор (название переменной)
    */
    class Ident : public LeafNode {
    public:
        explicit Ident(std::string txt) :
            LeafNode(std::string("Ident"), txt) {};
        MemObject* eval(MemoryKernel& mem) override;
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
        MemObject* eval(MemoryKernel& mem) override;
    };

    class OpType : public LeafNode {
    public:
        OpType(std::string v) :
            LeafNode(std::string("Op. type"), v) {};
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
        void json(std::ostream& out, AST_print_context& mem) override;
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
        std::string name;
        ASTNode &value;
    public:
        Assign(AssignMod &mod, std::string lexpr, ASTNode &rexpr) :
           mod{mod}, name{lexpr}, value{rexpr} {};
        void set(AssignMod& mod_) {
            mod.setMod(mod_.getMod());
        }
        void json(std::ostream& out, AST_print_context& mem) override;
        MemObject* eval(MemoryKernel& mem) override;
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
        void json(std::ostream& out, AST_print_context& mem) override;
        MemObject* eval(MemoryKernel& mem) override;
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
        void json(std::ostream& out, AST_print_context& mem) override;
        MemObject* eval(MemoryKernel& mem) override;
    };

    /**
     * Принтуем какое-то выражение
    */
    class Print : public ASTNode {
        ASTNode& left;
    public:
        explicit Print(ASTNode &l) : left{l} {}
        void json(std::ostream& out, AST_print_context& mem) override;
        MemObject* eval(MemoryKernel& mem) override;
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
        void json(std::ostream& out, AST_print_context& mem) override;
    };

    /**
     * Считываем информацию с юзера
     * 
     * type: Тип считываемых данных (VarType)
     * name: Имя переменной (string)
     * 
     * Возможно, создать класс для каждого readInt, readReal, readSrting
     * удобнее, в таком случае call me)
    */
    class Read : public ASTNode {
        std::string name;
        ASTNode &type;
    public:
        Read(ASTNode &l, std::string n) :
                name{n}, type{l} {};
        void json(std::ostream& out, AST_print_context& mem) override;
        MemObject* eval(MemoryKernel& mem) override;
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
        MemObject* eval(MemoryKernel& mem) override;
    };

    /**
     * Плюс
    */
    class Plus : public BinOp {
    public:
        Plus(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Plus"),  l, r) {};
        MemObject* eval(MemoryKernel& mem) override;
    };

    /**
     * Минус
    */
    class Minus : public BinOp {
    public:
        Minus(ASTNode &l, ASTNode &r) :
            BinOp(std::string("Minus"),  l, r) {};
        MemObject* eval(MemoryKernel& mem) override;
    };

    /**
     * Умножение
    */
    class Times : public BinOp {
    public:
        Times(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Times"),  l, r) {};
        MemObject* eval(MemoryKernel& mem) override;
    };

    /**
     * Деление
    */
    class Div : public BinOp {
    public:
        Div(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Div"),  l, r) {};
        MemObject* eval(MemoryKernel& mem) override;
    };

    // Condition expressions

    /**
     * Логическое И
    */
    class And : public BinOp {
    public:
        And(ASTNode &l, ASTNode &r) :
                BinOp(std::string("And"),  l, r) {};
        MemObject* eval(MemoryKernel& mem) override;
    };

    /**
     * Логическое ИЛИ
    */
    class Or : public BinOp {
    public:
        Or(ASTNode &l, ASTNode &r) :
                BinOp(std::string("Or"),  l, r) {};
        MemObject* eval(MemoryKernel& mem) override;
    };

    /**
     * Логическое Отрицание
    */
    class Not : public ASTNode {
        ASTNode& left;
    public:
        explicit Not(ASTNode &l) : left{l} {}
        void json(std::ostream& out, AST_print_context& mem) override;
        MemObject* eval(MemoryKernel& mem) override;
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
        MemObject* eval(MemoryKernel& mem) override;
    };

    /**
     * Меньше или равно
    */
    class Less_E : public Compare {
    public:
        Less_E(ASTNode &l, ASTNode &r) :
                Compare("Less_E", "<=",  l, r) {};
        MemObject* eval(MemoryKernel& mem) override;
    };

    /**
     * Больше или равно
    */
    class Greater_E : public Compare {
    public:
        Greater_E(ASTNode &l, ASTNode &r) :
                Compare("Greater_E", ">=",  l, r) {};
        MemObject* eval(MemoryKernel& mem) override;
    };

    /**
     * Больше
    */
    class Greater : public Compare {
    public:
        Greater(ASTNode &l, ASTNode &r) :
                Compare("Greater", ">", l, r) {};
        MemObject* eval(MemoryKernel& mem) override;
    };

    /**
     * Равняется
    */
    class Equals : public Compare {
    public:
        Equals(ASTNode &l, ASTNode &r) :
                Compare("Equals", "==", l, r) {};
        MemObject* eval(MemoryKernel& mem) override;
    };

    /**
     * Не равняется
    */
    class Not_Equals : public Compare {
    public:
        Not_Equals(ASTNode &l, ASTNode &r) :
                Compare("Not Equals", "!=", l, r) {};
        MemObject* eval(MemoryKernel& mem) override;
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
        void json(std::ostream& out, AST_print_context& mem) override;
        MemObject* eval(MemoryKernel& mem) override;
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
        void json(std::ostream& out, AST_print_context& mem) override;
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
        void json(std::ostream& out, AST_print_context& mem) override;
    };

    // Functions

    /**
     * Объявление функции
     * 
     * params: вектор переменных (Ident)
     * funcBlock: блок функции
     * 
     * flat: как обычно, ничего не поменялось, не используем Block в чистом виде
    */
    class FuncDecl: public ASTNode {
        friend Assign;
        std::vector<ASTNode*> params;
        Block &funcBody;
    public:
        explicit FuncDecl(Block &func_body) :
            funcBody{func_body} {};
        void flat(Block* block) {
            for (auto &i : block->getNodes()) {
                params.push_back(i);
            }
        }
        void json(std::ostream& out, AST_print_context& mem) override;
        MemObject* eval(MemoryKernel& mem) override;
    };

    class Return: public ASTNode {
        ASTNode &expr;
    public:
        explicit Return(ASTNode &func_expr) :
            expr{func_expr} {};
        void json(std::ostream& out, AST_print_context& mem) override;
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
        void json(std::ostream& out, AST_print_context& mem) override;
        MemObject* eval(MemoryKernel& mem) override;
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
        void json(std::ostream& out, AST_print_context& mem) override;
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
        void json(std::ostream& out, AST_print_context& mem) override; 
    };
}
#endif /* AST_HPP */