#include "type_system.hpp"

namespace ofs {

Scope::Scope() {
    // Start with global scope
    stack_.push_back({});
}

void Scope::push() {
    stack_.push_back({});
}

void Scope::pop() {
    if (stack_.size() > 1) {
        stack_.pop_back();
    }
}

void Scope::define(const std::string& name, Symbol sym) {
    sym.scope_depth = static_cast<int>(stack_.size()) - 1;
    stack_.back()[name] = std::move(sym);
}

const Symbol* Scope::lookup(const std::string& name) const {
    // Search from innermost to outermost scope
    for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return &found->second;
    }
    return nullptr;
}

bool Scope::exists_in_current(const std::string& name) const {
    return stack_.back().find(name) != stack_.back().end();
}

int Scope::depth() const {
    return static_cast<int>(stack_.size()) - 1;
}

} // namespace ofs
