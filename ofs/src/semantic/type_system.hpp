#pragma once
#include "../ast/ast.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

namespace ofs {

struct SemanticError : std::runtime_error {
    int line, col;
    SemanticError(const std::string& msg, int l=0, int c=0)
        : std::runtime_error(msg), line(l), col(c) {}
};

// Symbol kinds
enum class SymbolKind { Variable, Function, Monolith, Namespace, Param };

struct Symbol {
    std::string name;
    OFSType     type;
    SymbolKind  kind;
    bool        is_mut   = true;
    bool        is_const = false;
    int         scope_depth = 0;
};

// Scope stack
class Scope {
public:
    Scope();
    void push();                          // enter new scope
    void pop();                           // exit scope
    void define(const std::string& name, Symbol sym);
    const Symbol* lookup(const std::string& name) const;
    bool exists_in_current(const std::string& name) const;
    int  depth() const;

private:
    std::vector<std::unordered_map<std::string, Symbol>> stack_;
};

} // namespace ofs
