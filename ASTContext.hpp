#ifndef __SEMANTICAL_ANALYSIS
#define __SEMANTICAL_ANALYSIS

#include <vector>

#include "ast.hpp"

namespace AST{
    class ASTContext;
    class Function;
    class Variable;

    class ASTContext {
    
    protected:
        std::vector<Function*> functions;
        AST::Block *entry_point;
    
    public:
        ASTContext(ASTNode *ast_root);
        ~ASTContext();
    };

    class Variable {
    private:
        ASTNode *entry_point;

    public:
        Variable(ASTNode *entry_point, std::string name);
        virtual ~Variable() = default;
        std::string get_name() const;
        int count_references() const;
        void add_reference() {++references;}

    protected:
        std::string name;
        int references; // in currrent scope

    };

    class Function : Variable {
    private:
        int num_args;

    public:
        Function(ASTNode *entry_point, std::string name, int num_args);
        std::string get_name() const;
        int count_references() const;
        void add_reference();
        int count_args() const;
    };
}
#endif // __SEMANTICAL_ANALYSIS