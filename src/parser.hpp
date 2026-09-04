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
            // Function call?
            if (check(TT::OpenParen)) {
                consume(); // '('
                std::vector<Expr*> args;
                while (!check(TT::CloseParen) && !at_end()) {
                    args.push_back(parse_expr());
                    if (!try_consume(TT::Comma)) break;
                }
                expect(TT::CloseParen);
                auto* node = m_arena.alloc<ExprCall>(name, std::move(args));
                auto* expr = m_arena.alloc<Expr>();
                expr->var = node;
                return expr;
            }
            // Plain identifier
            auto* node = m_arena.alloc<ExprIdent>(name);
            auto* expr = m_arena.alloc<Expr>();
            expr->var = node;
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
        // fn definition
        if (check(TT::Fn)) return parse_fn();

        // Variable declaration: let or mut
        if (check(TT::Let) || check(TT::Mut)) return parse_var_decl();

        // Assignment: ident =
        if (check(TT::Ident) && peek(1).type == TT::Eq) return parse_assign();

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

        // Expression statement (e.g. function call)
        if (check(TT::Ident)) {
            auto* expr = parse_expr();
            expect(TT::Semi);
            auto* node = m_arena.alloc<StmtExpr>(expr);
            auto* stmt = m_arena.alloc<Stmt>();
            stmt->var  = node;
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
            VelType ptype = parse_type();
            params.push_back({pname, ptype});
            if (!try_consume(TT::Comma)) break;
        }
        expect(TT::CloseParen);

        VelType ret = VelType::Void;
        if (try_consume(TT::Arrow)) {
            ret = parse_type();
        }

        auto* body = parse_scope();
        auto* node = m_arena.alloc<StmtFn>(name, std::move(params), ret, body);
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
        if (try_consume(TT::Colon)) {
            type_hint = parse_type();
        }

        expect(TT::Eq);
        auto* init = parse_expr();
        expect(TT::Semi);

        auto* node = m_arena.alloc<StmtVar>(name, is_mut, type_hint, init);
        auto* stmt = m_arena.alloc<Stmt>();
        stmt->var  = node;
        return stmt;
    }

    // x = expr;
    Stmt* parse_assign()
    {
        auto name = consume(); // ident
        expect(TT::Eq);
        auto* val = parse_expr();
        expect(TT::Semi);

        auto* node = m_arena.alloc<StmtAssign>(name, val);
        auto* stmt = m_arena.alloc<Stmt>();
        stmt->var  = node;
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
