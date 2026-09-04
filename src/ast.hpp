#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "tokenizer.hpp"

struct Expr;
struct Stmt;
struct Scope;


enum class VelType { Int, Float, Str, Bool, Void, Unknown };

inline std::string veltype_str(VelType t)
{
    switch (t) {
    case VelType::Int:     return "int";
    case VelType::Float:   return "float";
    case VelType::Str:     return "str";
    case VelType::Bool:    return "bool";
    case VelType::Void:    return "void";
    case VelType::Unknown: return "?";
    }
    return "?";
}


struct ExprIntLit   { Token tok; };                    // 42
struct ExprFloatLit { Token tok; };                    // 3.14
struct ExprStrLit   { Token tok; };                    // "hello"
struct ExprBoolLit  { Token tok; };                    // true / false
struct ExprIdent    { Token tok; };                    // x

struct ExprUnary {
    Token   op;   // ! or -
    Expr*   operand;
};

struct ExprBinary {
    Token   op;
    Expr*   lhs;
    Expr*   rhs;
};

struct ExprParen {
    Expr* inner;
};

struct ExprCall {
    Token               name;
    std::vector<Expr*>  args;
};

struct Expr {
    std::variant<
        ExprIntLit*,
        ExprFloatLit*,
        ExprStrLit*,
        ExprBoolLit*,
        ExprIdent*,
        ExprUnary*,
        ExprBinary*,
        ExprParen*,
        ExprCall*
    > var;
};


struct StmtVar {
    Token                    name;
    bool                     is_mut;         // mut vs let
    std::optional<VelType>   type_hint;      // optional annotation
    Expr*                    init;
};

struct StmtAssign {
    Token  name;
    Expr*  value;
};

struct StmtReturn {
    std::optional<Expr*> value;
};

struct StmtPrint {
    Expr* value;
};

struct StmtIf;

struct ElseBranch {
    std::variant<StmtIf*, Scope*> var;
};

struct StmtIf {
    Expr*                      cond;
    Scope*                     then_scope;
    std::optional<ElseBranch*> else_branch;
};

struct StmtWhile {
    Expr*  cond;
    Scope* body;
};

struct StmtLoop {
    Scope* body;
};

struct StmtBreak {};

// continue;
struct StmtContinue {};

// bare scope { ... }
struct StmtScope {
    Scope* scope;
};

// fn name(params...) -> ReturnType { ... }
struct FnParam {
    Token   name;
    VelType type;
};

struct StmtFn {
    Token                  name;
    std::vector<FnParam>   params;
    VelType                return_type;
    Scope*                 body;
};

// Expression used as a statement (e.g. a function call)
struct StmtExpr {
    Expr* expr;
};

struct Stmt {
    std::variant<
        StmtVar*,
        StmtAssign*,
        StmtReturn*,
        StmtPrint*,
        StmtIf*,
        StmtWhile*,
        StmtLoop*,
        StmtBreak*,
        StmtContinue*,
        StmtScope*,
        StmtFn*,
        StmtExpr*
    > var;
};


struct Scope {
    std::vector<Stmt*> stmts;
};


struct Program {
    std::vector<Stmt*> stmts;
};
