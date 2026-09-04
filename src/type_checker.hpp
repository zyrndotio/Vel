#pragma once

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "ast.hpp"

class TypeChecker {
public:
    explicit TypeChecker(const Program& program)
        : m_program(program)
    {
    }

    void check()
    {
        for (auto* stmt : m_program.stmts) {
            if (!std::holds_alternative<StmtFn*>(stmt->var)) continue;
            auto* fn = std::get<StmtFn*>(stmt->var);
            const auto name = fn->name.value.value_or("");
            if (m_functions.contains(name)) fail("duplicate function '" + name + "'");
            FunctionInfo info;
            info.return_type = fn->return_type;
            for (const auto& param : fn->params) info.params.push_back(param.type);
            m_functions.emplace(name, std::move(info));
        }

        push_scope();
        for (auto* stmt : m_program.stmts) check_stmt(stmt);
        pop_scope();
    }

private:
    struct Variable {
        VelType type;
        bool mutable_value;
        TypeRef* type_ref {nullptr};
    };

    struct FunctionInfo {
        std::vector<VelType> params;
        VelType return_type {VelType::Void};
    };

    [[noreturn]] static void fail(const std::string& message)
    {
        std::cerr << "[Vel] Type error: " << message << "\n";
        std::exit(EXIT_FAILURE);
    }

    static bool numeric(VelType type)
    {
        return type == VelType::Int || type == VelType::Float;
    }

    static bool compatible(VelType expected, VelType actual)
    {
        return expected == actual || expected == VelType::Unknown || actual == VelType::Unknown;
    }

    void push_scope() { m_scopes.emplace_back(); }
    void pop_scope() { m_scopes.pop_back(); }

    Variable* lookup(const std::string& name)
    {
        for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) return &found->second;
        }
        return nullptr;
    }

    VelType expression_type(const Expr* expr)
    {
        return std::visit([&](auto* node) -> VelType { return expression_type_node(node); }, expr->var);
    }

    VelType expression_type_node(const ExprIntLit*) { return VelType::Int; }
    VelType expression_type_node(const ExprFloatLit*) { return VelType::Float; }
    VelType expression_type_node(const ExprStrLit*) { return VelType::Str; }
    VelType expression_type_node(const ExprBoolLit*) { return VelType::Bool; }

    VelType expression_type_node(const ExprIdent* node)
    {
        const auto name = node->tok.value.value_or("");
        auto* variable = lookup(name);
        if (!variable) fail("undefined variable '" + name + "'");
        return variable->type;
    }

    VelType expression_type_node(const ExprParen* node) { return expression_type(node->inner); }

    VelType expression_type_node(const ExprUnary* node)
    {
        auto operand = expression_type(node->operand);
        if (node->op.type == TT::Bang) {
            if (operand != VelType::Bool) fail("operator ! requires bool operand");
            return VelType::Bool;
        }
        if (!numeric(operand)) fail("unary - requires numeric operand");
        return operand;
    }

    VelType expression_type_node(const ExprBinary* node)
    {
        auto lhs = expression_type(node->lhs);
        auto rhs = expression_type(node->rhs);
        switch (node->op.type) {
        case TT::Plus:
            if (lhs == VelType::Str && rhs == VelType::Str) return VelType::Str;
            if (!numeric(lhs) || !numeric(rhs) || lhs != rhs)
                fail("operator + requires matching numeric types or two strings");
            return lhs;
        case TT::Minus: case TT::Star: case TT::Slash: case TT::Percent:
            if (!numeric(lhs) || !numeric(rhs) || lhs != rhs)
                fail("arithmetic operands must have the same numeric type");
            return lhs;
        case TT::EqEq: case TT::BangEq: case TT::Lt: case TT::LtEq: case TT::Gt: case TT::GtEq:
            if (!compatible(lhs, rhs)) fail("comparison operands must have the same type");
            return VelType::Bool;
        case TT::And: case TT::Or:
            if (lhs != VelType::Bool || rhs != VelType::Bool)
                fail("logical operands must be bool");
            return VelType::Bool;
        default:
            fail("unsupported binary operator");
        }
    }

    VelType check_call(const ExprCall* node, bool require_value)
    {
        const auto name = node->name.value.value_or("");
        auto function = m_functions.find(name);
        if (function == m_functions.end()) fail("undefined function '" + name + "'");
        if (node->args.size() != function->second.params.size()) {
            fail("function '" + name + "' expects " + std::to_string(function->second.params.size())
                 + " argument(s), got " + std::to_string(node->args.size()));
        }
        for (size_t i = 0; i < node->args.size(); ++i) {
            auto actual = expression_type(node->args[i]);
            auto expected = function->second.params[i];
            if (!compatible(expected, actual)) {
                fail("argument " + std::to_string(i + 1) + " of '" + name + "' expects "
                     + veltype_str(expected) + ", got " + veltype_str(actual));
            }
        }
        if (require_value && function->second.return_type == VelType::Void)
            fail("void function '" + name + "' cannot be used as an expression");
        return function->second.return_type;
    }

    VelType expression_type_node(const ExprCall* node)
    {
        return check_call(node, true);
    }

    VelType expression_type_node(const ExprArray* node)
    {
        std::optional<VelType> element_type;
        for (auto* element : node->elements) {
            auto current = expression_type(element);
            if (!element_type) element_type = current;
            else if (*element_type != current) fail("array elements must have the same type");
        }
        return VelType::Unknown;
    }

    VelType expression_type_node(const ExprIndex* node)
    {
        auto index_type = expression_type(node->index);
        if (index_type != VelType::Int) fail("array index must be int");
        auto object_type = expression_type(node->object);
        if (object_type != VelType::Unknown) fail("indexing requires an array value");
        if (std::holds_alternative<ExprIdent*>(node->object->var)) {
            auto* variable = lookup(std::get<ExprIdent*>(node->object->var)->tok.value.value_or(""));
            if (!variable || !variable->type_ref || variable->type_ref->kind != TypeKind::Array)
                fail("indexing requires an array value");
            return variable->type_ref->element && variable->type_ref->element->kind == TypeKind::Scalar
                ? variable->type_ref->element->scalar : VelType::Unknown;
        }
        return VelType::Unknown;
    }

    VelType expression_type_node(const ExprField* node)
    {
        auto object_type = expression_type(node->object);
        if (object_type != VelType::Unknown) fail("field access requires a struct value");
        if (std::holds_alternative<ExprIdent*>(node->object->var)) {
            auto* variable = lookup(std::get<ExprIdent*>(node->object->var)->tok.value.value_or(""));
            if (!variable || !variable->type_ref || variable->type_ref->kind != TypeKind::Struct)
                fail("field access requires a struct value");
            auto structure = m_structs.find(variable->type_ref->name);
            if (structure == m_structs.end()) fail("undefined struct '" + variable->type_ref->name + "'");
            const auto field_name = node->field.value.value_or("");
            if (!structure->second.contains(field_name))
                fail("struct '" + variable->type_ref->name + "' has no field '" + field_name + "'");
            auto* field_type = structure->second.at(field_name);
            return field_type->kind == TypeKind::Scalar ? field_type->scalar : VelType::Unknown;
        }
        return VelType::Unknown;
    }

    VelType expression_type_node(const ExprStructInit* node)
    {
        const auto name = node->name.value.value_or("");
        auto structure = m_structs.find(name);
        if (structure == m_structs.end()) fail("undefined struct '" + name + "'");
        std::unordered_map<std::string, bool> seen;
        for (const auto& [field, value] : node->fields) {
            const auto field_name = field.value.value_or("");
            if (!structure->second.contains(field_name))
                fail("struct '" + name + "' has no field '" + field_name + "'");
            if (seen.contains(field_name)) fail("duplicate field '" + field_name + "'");
            seen[field_name] = true;
            auto expected = structure->second.at(field_name);
            auto actual = expression_type(value);
            if (expected->kind == TypeKind::Scalar && expected->scalar != actual)
                fail("field '" + field_name + "' expects " + type_ref_str(expected) + ", got " + veltype_str(actual));
        }
        if (seen.size() != structure->second.size())
            fail("struct literal '" + name + "' is missing a field");
        return VelType::Unknown;
    }

    void check_stmt(Stmt* stmt)
    {
        std::visit([&](auto* node) { check_stmt_node(node); }, stmt->var);
    }

    void check_stmt_node(StmtVar* node)
    {
        auto actual = expression_type(node->init);
        auto declared = node->type_hint.value_or(actual);
        if (!compatible(declared, actual))
            fail("variable '" + node->name.value.value_or("") + "' expects "
                 + veltype_str(declared) + ", got " + veltype_str(actual));
        auto name = node->name.value.value_or("");
        if (m_scopes.back().contains(name)) fail("variable '" + name + "' is already defined in this scope");
        m_scopes.back().emplace(name, Variable {declared, node->is_mut, node->type_ref});
    }

    void check_stmt_node(StmtAssign* node)
    {
        const auto name = node->name.value.value_or("");
        auto* variable = lookup(name);
        if (!variable) fail("undefined variable '" + name + "'");
        if (!variable->mutable_value) fail("cannot assign to immutable variable '" + name + "'");
        auto actual = expression_type(node->value);
        if (!compatible(variable->type, actual))
            fail("assignment to '" + name + "' expects " + veltype_str(variable->type)
                 + ", got " + veltype_str(actual));
    }

    void check_stmt_node(StmtReturn* node)
    {
        if (!m_current_return_type) fail("return used outside of a function");
        const auto expected = *m_current_return_type;
        if (expected == VelType::Void && node->value) fail("void function cannot return a value");
        if (expected != VelType::Void && !node->value) fail("function must return a value of type " + veltype_str(expected));
        if (node->value) {
            auto actual = expression_type(*node->value);
            if (!compatible(expected, actual))
                fail("return expects " + veltype_str(expected) + ", got " + veltype_str(actual));
        }
        m_current_fn_has_return = true;
    }

    void check_stmt_node(StmtPrint* node) { expression_type(node->value); }

    void check_stmt_node(StmtStruct* node)
    {
        const auto name = node->name.value.value_or("");
        if (m_structs.contains(name)) fail("duplicate struct '" + name + "'");
        std::unordered_map<std::string, TypeRef*> fields;
        for (const auto& field : node->fields) {
            const auto field_name = field.name.value.value_or("");
            if (fields.contains(field_name)) fail("duplicate field '" + field_name + "'");
            fields.emplace(field_name, field.type);
        }
        m_structs.emplace(name, std::move(fields));
    }

    void check_stmt_node(StmtIf* node)
    {
        if (expression_type(node->cond) != VelType::Bool) fail("if condition must be bool");
        push_scope();
        for (auto* stmt : node->then_scope->stmts) check_stmt(stmt);
        pop_scope();
        if (node->else_branch) {
            std::visit([&](auto* branch) {
                if constexpr (std::is_same_v<decltype(branch), StmtIf*>) check_stmt_node(branch);
                else {
                    push_scope();
                    for (auto* stmt : branch->stmts) check_stmt(stmt);
                    pop_scope();
                }
            }, (*node->else_branch)->var);
        }
    }

    void check_stmt_node(StmtWhile* node)
    {
        if (expression_type(node->cond) != VelType::Bool) fail("while condition must be bool");
        ++m_loop_depth;
        push_scope();
        for (auto* stmt : node->body->stmts) check_stmt(stmt);
        pop_scope();
        --m_loop_depth;
    }

    void check_stmt_node(StmtLoop* node)
    {
        ++m_loop_depth;
        push_scope();
        for (auto* stmt : node->body->stmts) check_stmt(stmt);
        pop_scope();
        --m_loop_depth;
    }

    void check_stmt_node(StmtBreak*)
    {
        if (m_loop_depth == 0) fail("break used outside of a loop");
    }

    void check_stmt_node(StmtContinue*)
    {
        if (m_loop_depth == 0) fail("continue used outside of a loop");
    }

    void check_stmt_node(StmtScope* node)
    {
        push_scope();
        for (auto* stmt : node->scope->stmts) check_stmt(stmt);
        pop_scope();
    }

    void check_stmt_node(StmtFn* node)
    {
        auto previous_return = m_current_return_type;
        auto previous_has_return = m_current_fn_has_return;
        m_current_return_type = node->return_type;
        m_current_fn_has_return = false;
        push_scope();
        for (const auto& param : node->params) {
            auto name = param.name.value.value_or("");
            if (m_scopes.back().contains(name)) fail("duplicate parameter '" + name + "'");
            m_scopes.back().emplace(name, Variable {param.type, false, param.type_ref});
        }
        for (auto* stmt : node->body->stmts) check_stmt(stmt);
        pop_scope();
        if (node->return_type != VelType::Void && !m_current_fn_has_return)
            fail("function '" + node->name.value.value_or("") + "' must return a value of type " + veltype_str(node->return_type));
        m_current_return_type = previous_return;
        m_current_fn_has_return = previous_has_return;
    }

    void check_stmt_node(StmtExpr* node)
    {
        if (std::holds_alternative<ExprCall*>(node->expr->var)) {
            check_call(std::get<ExprCall*>(node->expr->var), false);
        } else {
            expression_type(node->expr);
        }
    }

    const Program& m_program;
    std::vector<std::unordered_map<std::string, Variable>> m_scopes;
    std::unordered_map<std::string, FunctionInfo> m_functions;
    std::unordered_map<std::string, std::unordered_map<std::string, TypeRef*>> m_structs;
    std::optional<VelType> m_current_return_type;
    bool m_current_fn_has_return {false};
    size_t m_loop_depth {0};
};
