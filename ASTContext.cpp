#include <stack>

#include "ASTContext.hpp"
#include "ast.hpp"

using namespace AST;

static std::vector<std::string> collect_variables(ASTNode *node) {
    std::vector<std::string> vars;
    
    Ident *ident = dynamic_cast<Ident*>(node);

    if (ident){
        vars.push_back(ident->get_name());
        return vars;
    }

    BinOp *op = dynamic_cast<BinOp*>(node);

    if (!op)
        return vars;

    ASTNode *left = op->get_left();
    ASTNode *right = op->get_right();

    Ident *left_ident = dynamic_cast<Ident*>(left);
    Ident *right_ident = dynamic_cast<Ident*>(right);

    if (!left_ident){
        std::vector<std::string> children = collect_variables(left);
        vars.insert(vars.end(), children.begin(), children.end());
    } else {
        vars.push_back(left_ident->get_name());
    }

    if (!right_ident){
        std::vector<std::string> children = collect_variables(right);
        vars.insert(vars.end(), children.begin(), children.end());
    } else {
        vars.push_back(right_ident->get_name());
    }

    return vars;
}

ASTContext::ASTContext(ASTNode *ast_root) {
    this->entry_point = static_cast<Block*>(ast_root);

    std::stack<std::vector<ASTNode*>> scopes;

    std::vector<ASTNode*> initial_vector = {this->entry_point,};
    scopes.push(initial_vector);

    std::vector<Variable*> variables;
    std::stack<int> in_scope; // number of variables in current scope

    in_scope.push(0); // zero in current scope yet

    /**
     * @brief Traverse all scopes
     * 
     * It will have up-to-down order, e.g.:
     * 
     *  scope 1:
     *      expression 1.1
     *      expression 1.2
     *      scope 2:
     *          expresion 2.1
     *      expression 1.3
     * 
     */
    while (!scopes.empty() && !scopes.top().empty()) {
        ASTNode *cur_node = scopes.top()[0];
        scopes.top().erase(scopes.top().begin());

       /**
        * @brief Check if current node is block
        * 
        * In that case we should add it to the top
        * of the stack and start counting new references
        * which appeared in this scope.
        * 
        * Also handle block from statements
        */
        Block *block = dynamic_cast<Block*>(cur_node);
        While *while_block = dynamic_cast<While*>(cur_node);

        if (while_block)
            block = dynamic_cast<Block*>(while_block->get_body());

        if (block && !dynamic_cast<Assign*>(block->list_nodes()[0])) {

            std::cout << "Block of code detected\n";

            std::cout << "\n\n" << block->str() << "\n\n";

            scopes.push(block->list_nodes());
            in_scope.push(0);
            continue;
        }


        // TODO: handle variables in other code blocks
        

        /**
         * @brief If current expression is assignment, add new variable to current scope
         * 
         * First, check if variable didn't exist before, then add to scope.
         * Othwerwise - ignore;
         */
        Assign *assign_expr = dynamic_cast<Assign*>(block->list_nodes()[0]);
        if (assign_expr) {
            std::string name = dynamic_cast<Ident*>(assign_expr->get_lexpr())->get_name();
 
            bool var_exists = false;
            for (Variable *v: variables) {
                if (v->get_name() == name) {
                    var_exists = true;
                    break;
                }
            }

            if (!var_exists) {
                variables.push_back(new Variable(cur_node, name));
                int num_scop = in_scope.top() + 1;
                in_scope.pop();
                in_scope.push(num_scop);
                std::cout << "Assign detected (" << name << ")\n";
            } else {
                std::cout << "Assigned existing variable (" << name << ")\n";
            }
            

            // check if assign operator uses other variables
            std::vector<std::string> used_variables = collect_variables(assign_expr->get_rexpr());

            for (auto used : used_variables) {
                bool found = false;
                for (auto v: variables) {
                    if (v->get_name() == used){
                        found = true;
                        v->add_reference();
                        break;
                    }
                }
                if (!found) {
                    std::cout << "Undefined reference to " << used << "\n";
                    std::cout << "Aborting\n";
                    exit(1);
                }
            }

            // check if it function call
            FuncCall *fcall = dynamic_cast<FuncCall*> (assign_expr->get_rexpr());
            if (fcall) {
                // find function which it corresponds
                bool found = false;
                for (auto f : functions){
                    if (f->get_name() == fcall->get_ident()->get_name()) {
                        found = true;

                        if (f->count_args() != fcall->get_params()->list_nodes().size()){
                            std::cout
                                    << "(" << f->get_name() << ") "
                                    << "Wrong number of parameters. Expected: "
                                    << f->count_args()
                                    << ", received: "
                                    << fcall->get_params()->list_nodes().size()
                                    << "\n";
                            exit(1);
                        }
                    }
                }
                if (!found) {
                    std::cout << "No such function " << fcall->get_ident()->get_name();
                }
            }


            FuncDecl *func = dynamic_cast<FuncDecl*>(assign_expr->get_rexpr());
            if (func) {

                std::vector<ASTNode*> params = func->get_params()->list_nodes();

                for (auto p: params) {
                    variables.push_back(new Variable(p, dynamic_cast<Ident*>(p)->get_name()));
                }

                scopes.push(func->get_body()->list_nodes());
                in_scope.push(params.size());

                functions.push_back(new Function(func, name, params.size()));

                continue;
            }

        }
        
        /**
         * @brief cleanup scope variables
         * 
         * When we traversed current scope, variables created there
         * are not visible more. We can destroy them successfully. 
         */
        if (scopes.top().empty()) {
            scopes.pop();
            for (int i = 0; i < in_scope.top(); ++i) {
                // if variable had 0 references, print warning
                if (variables[variables.size() - 1]->count_references() == 0) {
                    std::cout 
                            << "Warning: variable " 
                            << variables[variables.size() - 1]->get_name()
                            << " was not used anywhere. You can remove it.\n";
                }
                std::cout << "Destroyed (" << variables[variables.size() - 1]->get_name() << ")\n";
                variables.pop_back();
            }
            in_scope.pop();
            std::cout << "Exited scope\n";
        }
    }

    for (Variable *v: variables) {
        if (v->count_references() == 0 ){
            std::cout 
                        << "Warning: variable " 
                        << v->get_name()
                        << " was not used anywhere. You can remove it.\n";
        }
    }
}

Variable::Variable(ASTNode *entry_point, std::string name) 
    : entry_point(entry_point), name(name), references(0) {}

std::string Variable::get_name() const {
    return this->name;
}

int Variable::count_references() const {
    return this->references;
}

Function::Function(ASTNode *entry_point, std::string name, int num_args)
    : Variable(entry_point, name), num_args(num_args) {}



int Function::count_args() const {
    return this->num_args;
}

std::string Function::get_name() const {
    return Variable::get_name();
}
int Function::count_references() const {
    return Variable::count_references();
}
void Function::add_reference() {
    Variable::add_reference();
}