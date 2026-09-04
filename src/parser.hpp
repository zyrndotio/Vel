#pragma once

#include <iostream>
#include <optional>
#include <vector>

#include "arena.hpp"
#include "ast.hpp"
#include "tokenizer.hpp"

class Parser {
public:
    explicit Parser(std::vector<Token> tokens, Arena& arena)
        : m_tokens(std::move(tokens))
        , m_arena(arena)
    {
    }

    Program parse()
    {
        Program prog;
        while (!at_end()) {
            if (auto* s = parse_stmt()) {
                prog.stmts.push_back(s);
            } else {
                error("statement");
            }
        }
        return prog;
    }

private:
    [[noreturn]] void error(const std::string& expected) const
    {
        const auto& t = current();
        std::cerr << "[Vel] Parse error at line " << t.line << ":" << t.col
                  << " — expected " << expected
                  << ", got " << tt_str(t.type) << "\n";
        exit(EXIT_FAILURE);
    }

    bool at_end() const
    {
        return m_pos >= m_tokens.size() || m_tokens[m_pos].type == TT::Eof;
    }

    const Token& current() const
    {
        return m_tokens[m_pos];
    }

    const Token& peek(size_t offset = 0) const
    {
        size_t idx = m_pos + offset;
        if (idx >= m_tokens.size()) return m_tokens.back();
        return m_tokens[idx];
    }

    Token consume()
    {
        return m_tokens[m_pos++];
    }

    bool check(TT t) const { return !at_end() && current().type == t; }

    std::optional<Token> try_consume(TT t)
    {
        if (check(t)) return consume();
        return {};
    }

    Token expect(TT t)
    {
        if (!check(t)) error(tt_str(t));
        return consume();
    }

    VelType parse_type()
    {
        if (try_consume(TT::TyInt))   return VelType::Int;
        if (try_consume(TT::TyFloat)) return VelType::Float;
        if (try_consume(TT::TyStr))   return VelType::Str;
        if (try_consume(TT::TyBool))  return VelType::Bool;
        error("type (int, float, str, bool)");
    }

    TypeRef* parse_type_ref()
    {
        if (check(TT::OpenBracket)) {
            consume();
            auto* element = parse_type_ref();
            expect(TT::CloseBracket);
            auto* type = m_arena.alloc<TypeRef>();
            type->kind = TypeKind::Array;
            type->element = element;
            return type;
        }
        auto* type = m_arena.alloc<TypeRef>();
        if (check(TT::TyInt) || check(TT::TyFloat) || check(TT::TyStr) || check(TT::TyBool)) {
            type->kind = TypeKind::Scalar;
            type->scalar = parse_type();
            return type;
        }
        if (check(TT::Ident)) {
            type->kind = TypeKind::Struct;
            type->name = consume().value.value_or("");
            return type;
        }
        error("type (int, float, str, bool, [type], StructName)");
    }

    static VelType scalar_type(const TypeRef* type)
    {
        return type && type->kind == TypeKind::Scalar ? type->scalar : VelType::Unknown;
    }

    std::optional<VelType> try_parse_type()
    {
        if (check(TT::TyInt))   { consume(); return VelType::Int; }
        if (check(TT::TyFloat)) { consume(); return VelType::Float; }
        if (check(TT::TyStr))   { consume(); return VelType::Str; }
        if (check(TT::TyBool))  { consume(); return VelType::Bool; }
        return {};
    }

    // Primary: literal, ident, call, paren, unary
    Expr* parse_primary()
    {
        // Unary: ! or -
        if (check(TT::Bang) || check(TT::Minus)) {
            auto op = consume();
            auto* operand = parse_primary();
            auto* node = m_arena.alloc<ExprUnary>(op, operand);
            auto* expr = m_arena.alloc<Expr>();
            expr->var = node;
            return expr;
        }

        // Parenthesised expression
        if (check(TT::OpenParen)) {
            consume();
            auto* inner = parse_expr();
            expect(TT::CloseParen);
            auto* node = m_arena.alloc<ExprParen>(inner);
            auto* expr = m_arena.alloc<Expr>();
            expr->var = node;
            return expr;
        }

        // Int literal
        if (check(TT::IntLit)) {
            auto tok = consume();
            auto* node = m_arena.alloc<ExprIntLit>(tok);
            auto* expr = m_arena.alloc<Expr>();
            expr->var = node;
            return expr;
        }

        // Float literal
        if (check(TT::FloatLit)) {
            auto tok = consume();
            auto* node = m_arena.alloc<ExprFloatLit>(tok);
            auto* expr = m_arena.alloc<Expr>();
            expr->var = node;
            return expr;
        }

        // String literal
        if (check(TT::StrLit)) {
            auto tok = consume();
            auto* node = m_arena.alloc<ExprStrLit>(tok);
            auto* expr = m_arena.alloc<Expr>();
            expr->var = node;
            return expr;
        }

        // Array literal
        if (check(TT::OpenBracket)) {
            consume();
            std::vector<Expr*> elements;
            while (!check(TT::CloseBracket) && !at_end()) {
                elements.push_back(parse_expr());
                if (!try_consume(TT::Comma)) break;
            }
            expect(TT::CloseBracket);
            auto* expr = m_arena.alloc<Expr>();
            expr->var = m_arena.alloc<ExprArray>(std::move(elements));
            return expr;
        }

        // Bool literal
        if (check(TT::BoolLit)) {
            auto tok = consume();
            auto* node = m_arena.alloc<ExprBoolLit>(tok);
            auto* expr = m_arena.alloc<Expr>();
            expr->var = node;
            return expr;
        }

        // Identifier or function call
        if (check(TT::Ident)) {
            auto name = consume();
            auto* expr = m_arena.alloc<Expr>();
            expr->var = m_arena.alloc<ExprIdent>(name);
            while (true) {
                if (check(TT::OpenCurly)) {
                    consume();
                    std::vector<std::pair<Token, Expr*>> fields;
                    while (!check(TT::CloseCurly) && !at_end()) {
                        auto field = expect(TT::Ident);
                        expect(TT::Colon);
                        fields.push_back({field, parse_expr()});
                        if (!try_consume(TT::Comma)) break;
                    }
                    expect(TT::CloseCurly);
                    auto ident = std::get<ExprIdent*>(expr->var)->tok;
                    expr->var = m_arena.alloc<ExprStructInit>(ident, std::move(fields));
                } else if (check(TT::OpenParen)) {
                    consume();
                    std::vector<Expr*> args;
                    while (!check(TT::CloseParen) && !at_end()) {
                        args.push_back(parse_expr());
                        if (!try_consume(TT::Comma)) break;
                    }
                    expect(TT::CloseParen);
                    if (!std::holds_alternative<ExprIdent*>(expr->var)) error("callable identifier");
                    auto ident = std::get<ExprIdent*>(expr->var)->tok;
                    expr->var = m_arena.alloc<ExprCall>(ident, std::move(args));
                } else if (check(TT::OpenBracket)) {
                    consume();
                    auto* index = parse_expr();
                    expect(TT::CloseBracket);
                    auto* node = m_arena.alloc<ExprIndex>(expr, index);
                    expr = m_arena.alloc<Expr>();
                    expr->var = node;
                } else if (check(TT::Dot)) {
                    consume();
                    auto field = expect(TT::Ident);
                    auto* node = m_arena.alloc<ExprField>(expr, field);
                    expr = m_arena.alloc<Expr>();
                    expr->var = node;
                } else {
                    break;
                }
            }
            return expr;
        }

        error("expression");
    }

    // Binary expressions with precedence climbing
    Expr* parse_expr(int min_prec = 0)
    {
        Expr* lhs = parse_primary();

        while (true) {
            if (at_end()) break;
            auto prec = bin_prec(current().type);
            if (!prec || *prec < min_prec) break;

            auto op   = consume();
            auto* rhs = parse_expr(*prec + 1);

            auto* bin  = m_arena.alloc<ExprBinary>(op, lhs, rhs);
            auto* expr = m_arena.alloc<Expr>();
            expr->var  = bin;
            lhs = expr;
        }

        return lhs;
    }

    Scope* parse_scope()
    {
        expect(TT::OpenCurly);
        auto* scope = m_arena.alloc<Scope>();
        while (!check(TT::CloseCurly) && !at_end()) {
            if (auto* s = parse_stmt()) {
                scope->stmts.push_back(s);
            } else {
                error("statement inside block");
            }
        }
        expect(TT::CloseCurly);
        return scope;
    }

    Stmt* parse_stmt()
    {
        // struct definition
        if (check(TT::Struct)) return parse_struct();

        // fn definition
        if (check(TT::Fn)) return parse_fn();

        // Variable declaration: let or mut
        if (check(TT::Let) || check(TT::Mut)) return parse_var_decl();


        // return
        if (check(TT::Return)) return parse_return();

        // print
        if (check(TT::Print)) return parse_print();

        // if
        if (check(TT::If)) return parse_if();

        // while
        if (check(TT::While)) return parse_while();

        // loop
        if (check(TT::Loop)) return parse_loop();

        // break
        if (check(TT::Break)) {
            consume();
            expect(TT::Semi);
            auto* stmt = m_arena.alloc<Stmt>();
            stmt->var  = m_arena.alloc<StmtBreak>();
            return stmt;
        }

        // continue
        if (check(TT::Continue)) {
            consume();
            expect(TT::Semi);
            auto* stmt = m_arena.alloc<Stmt>();
            stmt->var  = m_arena.alloc<StmtContinue>();
            return stmt;
        }

        // bare scope
        if (check(TT::OpenCurly)) {
            auto* scope = parse_scope();
            auto* node  = m_arena.alloc<StmtScope>(scope);
            auto* stmt  = m_arena.alloc<Stmt>();
            stmt->var   = node;
            return stmt;
        }

        // Assignment or expression statement.
        if (check(TT::Ident)) {
            auto* target = parse_expr();
            if (try_consume(TT::Eq)) {
                auto* value = parse_expr();
                expect(TT::Semi);
                auto* node = m_arena.alloc<StmtAssign>(target, value);
                auto* stmt = m_arena.alloc<Stmt>();
                stmt->var = node;
                return stmt;
            }
            expect(TT::Semi);
            auto* node = m_arena.alloc<StmtExpr>(target);
            auto* stmt = m_arena.alloc<Stmt>();
            stmt->var = node;
            return stmt;
        }

        return nullptr;
    }

    // fn name(params) -> type { scope }
    // fn name(params) { scope }   — defaults to void
    Stmt* parse_fn()
    {
        expect(TT::Fn);
        auto name = expect(TT::Ident);
        expect(TT::OpenParen);

        std::vector<FnParam> params;
        while (!check(TT::CloseParen) && !at_end()) {
            auto pname = expect(TT::Ident);
            expect(TT::Colon);
            auto* ptype_ref = parse_type_ref();
            params.push_back({pname, scalar_type(ptype_ref), ptype_ref});
            if (!try_consume(TT::Comma)) break;
        }
        expect(TT::CloseParen);

        VelType ret = VelType::Void;
        TypeRef* ret_ref = nullptr;
        if (try_consume(TT::Arrow)) {
            ret_ref = parse_type_ref();
            ret = scalar_type(ret_ref);
        }

        auto* body = parse_scope();
        auto* node = m_arena.alloc<StmtFn>(name, std::move(params), ret, body);
        node->return_ref = ret_ref;
        auto* stmt = m_arena.alloc<Stmt>();
        stmt->var  = node;
        return stmt;
    }

    // let x: int = expr;
    // mut y = expr;
    Stmt* parse_var_decl()
    {
        bool is_mut = check(TT::Mut);
        consume(); // let or mut

        auto name = expect(TT::Ident);

        std::optional<VelType> type_hint;
        TypeRef* type_ref = nullptr;
        if (try_consume(TT::Colon)) {
            type_ref = parse_type_ref();
            type_hint = scalar_type(type_ref);
        }

        expect(TT::Eq);
        auto* init = parse_expr();
        expect(TT::Semi);

        auto* node = m_arena.alloc<StmtVar>(name, is_mut, type_hint, init);
        node->type_ref = type_ref;
        auto* stmt = m_arena.alloc<Stmt>();
        stmt->var  = node;
        return stmt;
    }

    Stmt* parse_struct()
    {
        expect(TT::Struct);
        auto name = expect(TT::Ident);
        auto* node = m_arena.alloc<StmtStruct>();
        node->name = name;
        expect(TT::OpenCurly);
        while (!check(TT::CloseCurly) && !at_end()) {
            auto field = expect(TT::Ident);
            expect(TT::Colon);
            auto* type = parse_type_ref();
            node->fields.push_back({field, type});
            if (!try_consume(TT::Comma)) try_consume(TT::Semi);
        }
        expect(TT::CloseCurly);
        auto* stmt = m_arena.alloc<Stmt>();
        stmt->var = node;
        return stmt;
    }


    // return expr;   or   return;
    Stmt* parse_return()
    {
        expect(TT::Return);
        std::optional<Expr*> val;
        if (!check(TT::Semi)) {
            val = parse_expr();
        }
        expect(TT::Semi);

        auto* node = m_arena.alloc<StmtReturn>(val);
        auto* stmt = m_arena.alloc<Stmt>();
        stmt->var  = node;
        return stmt;
    }

    // print expr;
    Stmt* parse_print()
    {
        expect(TT::Print);
        auto* val = parse_expr();
        expect(TT::Semi);

        auto* node = m_arena.alloc<StmtPrint>(val);
        auto* stmt = m_arena.alloc<Stmt>();
        stmt->var  = node;
        return stmt;
    }

    // if (cond) { ... } elif (cond) { ... } else { ... }
    Stmt* parse_if()
    {
        expect(TT::If);
        expect(TT::OpenParen);
        auto* cond = parse_expr();
        expect(TT::CloseParen);
        auto* then_scope = parse_scope();

        std::optional<ElseBranch*> else_branch;

        if (check(TT::Elif)) {
            // Reparse as nested if
            auto* nested_if_stmt = parse_elif_chain();
            auto* branch = m_arena.alloc<ElseBranch>();
            branch->var  = std::get<StmtIf*>(nested_if_stmt->var);
            else_branch  = branch;
        } else if (try_consume(TT::Else)) {
            auto* else_scope = parse_scope();
            auto* branch = m_arena.alloc<ElseBranch>();
            branch->var  = else_scope;
            else_branch  = branch;
        }

        auto* node = m_arena.alloc<StmtIf>(cond, then_scope, else_branch);
        auto* stmt = m_arena.alloc<Stmt>();
        stmt->var  = node;
        return stmt;
    }

    // elif as a nested if (so elif chains work recursively)
    Stmt* parse_elif_chain()
    {
        expect(TT::Elif);
        expect(TT::OpenParen);
        auto* cond = parse_expr();
        expect(TT::CloseParen);
        auto* then_scope = parse_scope();

        std::optional<ElseBranch*> else_branch;

        if (check(TT::Elif)) {
            auto* nested = parse_elif_chain();
            auto* branch = m_arena.alloc<ElseBranch>();
            branch->var  = std::get<StmtIf*>(nested->var);
            else_branch  = branch;
        } else if (try_consume(TT::Else)) {
            auto* else_scope = parse_scope();
            auto* branch = m_arena.alloc<ElseBranch>();
            branch->var  = else_scope;
            else_branch  = branch;
        }

        auto* node = m_arena.alloc<StmtIf>(cond, then_scope, else_branch);
        auto* stmt = m_arena.alloc<Stmt>();
        stmt->var  = node;
        return stmt;
    }

    // while (cond) { ... }
    Stmt* parse_while()
    {
        expect(TT::While);
        expect(TT::OpenParen);
        auto* cond = parse_expr();
        expect(TT::CloseParen);
        auto* body = parse_scope();

        auto* node = m_arena.alloc<StmtWhile>(cond, body);
        auto* stmt = m_arena.alloc<Stmt>();
        stmt->var  = node;
        return stmt;
    }

    // loop { ... }
    Stmt* parse_loop()
    {
        expect(TT::Loop);
        auto* body = parse_scope();

        auto* node = m_arena.alloc<StmtLoop>(body);
        auto* stmt = m_arena.alloc<Stmt>();
        stmt->var  = node;
        return stmt;
    }

    std::vector<Token> m_tokens;
    size_t             m_pos { 0 };
    Arena&             m_arena;
};
