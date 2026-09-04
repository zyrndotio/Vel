#pragma once

#include <optional>
#include <string>
#include <memory>
#include <variant>
#include <vector>

#include "tokenizer.hpp"

struct Expr;
struct Stmt;
struct Scope;


enum class VelType { Int, Float, Str, Bool, Void, Unknown };

enum class TypeKind { Scalar, Array, Struct };

struct TypeRef {
    TypeKind kind {TypeKind::Scalar};
    VelType scalar {VelType::Unknown};
    std::string name;
    TypeRef* element {nullptr};
};

inline bool same_type(const TypeRef* lhs, const TypeRef* rhs)
{
    if (!lhs || !rhs || lhs->kind != rhs->kind) return false;
    if (lhs->kind == TypeKind::Scalar) return lhs->scalar == rhs->scalar;
    if (lhs->kind == TypeKind::Struct) return lhs->name == rhs->name;
    return same_type(lhs->element, rhs->element);
}

inline std::string veltype_str(VelType t);

inline std::string type_ref_str(const TypeRef* type)
{
    if (!type) return "?";
    if (type->kind == TypeKind::Scalar) return veltype_str(type->scalar);
    if (type->kind == TypeKind::Struct) return type->name;
    return "[" + type_ref_str(type->element) + "]";
}

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

struct ExprArray {
    std::vector<Expr*> elements;
};

struct ExprIndex {
    Expr* object;
    Expr* index;
};

struct ExprField {
    Expr* object;
    Token field;
};

struct ExprStructInit {
    Token name;
    std::vector<std::pair<Token, Expr*>> fields;
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
        ExprCall*,
        ExprArray*,
        ExprIndex*,
        ExprField*,
        ExprStructInit*
    > var;
};


struct StmtVar {
    Token                    name;
    bool                     is_mut;         // mut vs let
    std::optional<VelType>   type_hint;      // legacy scalar annotation
    Expr*                    init;
    TypeRef*                 type_ref {nullptr};
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

struct StructField {
    Token name;
    TypeRef* type;
};

struct StmtStruct {
    Token name;
    std::vector<StructField> fields;
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
    TypeRef* type_ref {nullptr};
};

struct StmtFn {
    Token                  name;
    std::vector<FnParam>   params;
    VelType                return_type;
    Scope*                 body;
    TypeRef*               return_ref {nullptr};
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
        StmtStruct*,
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
