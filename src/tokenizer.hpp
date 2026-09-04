#pragma once

#include <cassert>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

enum class TT {
    IntLit,
    FloatLit,
    StrLit,
    BoolLit,

    Ident,
    Fn,
    Struct,
    Return,
    Let,
    Mut,
    If,
    Elif,
    Else,
    Loop,
    While,
    Break,
    Continue,
    Print,

    TyInt,
    TyFloat,
    TyStr,
    TyBool,

    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Eq,
    EqEq,
    BangEq,
    Lt,
    LtEq,
    Gt,
    GtEq,
    And,
    Or,
    Bang,
    Arrow,

    Semi,
    Colon,
    Comma,
    Dot,
    OpenParen,
    CloseParen,
    OpenCurly,
    CloseCurly,
    OpenBracket,
    CloseBracket,

    Eof,
};

inline std::string tt_str(TT t)
{
    switch (t) {
    case TT::IntLit:       return "int literal";
    case TT::FloatLit:     return "float literal";
    case TT::StrLit:       return "string literal";
    case TT::BoolLit:      return "bool literal";
    case TT::Ident:        return "identifier";
    case TT::Fn:           return "`fn`";
    case TT::Struct:       return "`struct`";
    case TT::Return:       return "`return`";
    case TT::Let:          return "`let`";
    case TT::Mut:          return "`mut`";
    case TT::If:           return "`if`";
    case TT::Elif:         return "`elif`";
    case TT::Else:         return "`else`";
    case TT::Loop:         return "`loop`";
    case TT::While:        return "`while`";
    case TT::Break:        return "`break`";
    case TT::Continue:     return "`continue`";
    case TT::Print:        return "`print`";
    case TT::TyInt:        return "type `int`";
    case TT::TyFloat:      return "type `float`";
    case TT::TyStr:        return "type `str`";
    case TT::TyBool:       return "type `bool`";
    case TT::Plus:         return "`+`";
    case TT::Minus:        return "`-`";
    case TT::Star:         return "`*`";
    case TT::Slash:        return "`/`";
    case TT::Percent:      return "`%`";
    case TT::Eq:           return "`=`";
    case TT::EqEq:         return "`==`";
    case TT::BangEq:       return "`!=`";
    case TT::Lt:           return "`<`";
    case TT::LtEq:         return "`<=`";
    case TT::Gt:           return "`>`";
    case TT::GtEq:         return "`>=`";
    case TT::And:          return "`&&`";
    case TT::Or:           return "`||`";
    case TT::Bang:         return "`!`";
    case TT::Arrow:        return "`->`";
    case TT::Semi:         return "`;`";
    case TT::Colon:        return "`:`";
    case TT::Comma:        return "`,`";
    case TT::Dot:          return "`.`";
    case TT::OpenParen:    return "`(`";
    case TT::CloseParen:   return "`)`";
    case TT::OpenCurly:    return "`{`";
    case TT::CloseCurly:   return "`}`";
    case TT::OpenBracket:  return "`[`";
    case TT::CloseBracket: return "`]`";
    case TT::Eof:          return "end of file";
    }
    assert(false);
    return "";
}

// Binary operator precedence (higher = tighter binding)
inline std::optional<int> bin_prec(TT t)
{
    switch (t) {
    case TT::Or:      return 1;
    case TT::And:     return 2;
    case TT::EqEq:
    case TT::BangEq:  return 3;
    case TT::Lt:
    case TT::LtEq:
    case TT::Gt:
    case TT::GtEq:    return 4;
    case TT::Plus:
    case TT::Minus:   return 5;
    case TT::Star:
    case TT::Slash:
    case TT::Percent: return 6;
    default:          return {};
    }
}

struct Token {
    TT                      type;
    int                     line   { 1 };
    int                     col    { 1 };
    std::optional<std::string> value {};
};

class Tokenizer {
public:
    explicit Tokenizer(std::string src)
        : m_src(std::move(src))
    {
    }

    std::vector<Token> tokenize()
    {
        std::vector<Token> tokens;

        while (!at_end()) {
            skip_whitespace_and_comments();
            if (at_end()) break;

            const int tok_line = m_line;
            const int tok_col  = m_col;

            const char c = peek();

            if (c == '"') {
                tokens.push_back(read_string(tok_line, tok_col));
                continue;
            }

            if (std::isdigit(c)) {
                tokens.push_back(read_number(tok_line, tok_col));
                continue;
            }

            if (std::isalpha(c) || c == '_') {
                tokens.push_back(read_word(tok_line, tok_col));
                continue;
            }

            if (c == '=' && peek(1) == '=') { advance(2); tokens.push_back({TT::EqEq,    tok_line, tok_col}); continue; }
            if (c == '!' && peek(1) == '=') { advance(2); tokens.push_back({TT::BangEq,  tok_line, tok_col}); continue; }
            if (c == '<' && peek(1) == '=') { advance(2); tokens.push_back({TT::LtEq,    tok_line, tok_col}); continue; }
            if (c == '>' && peek(1) == '=') { advance(2); tokens.push_back({TT::GtEq,    tok_line, tok_col}); continue; }
            if (c == '-' && peek(1) == '>') { advance(2); tokens.push_back({TT::Arrow,   tok_line, tok_col}); continue; }
            if (c == '&' && peek(1) == '&') { advance(2); tokens.push_back({TT::And,     tok_line, tok_col}); continue; }
            if (c == '|' && peek(1) == '|') { advance(2); tokens.push_back({TT::Or,      tok_line, tok_col}); continue; }

            TT single;
            switch (c) {
            case '+': single = TT::Plus;         break;
            case '-': single = TT::Minus;        break;
            case '*': single = TT::Star;         break;
            case '/': single = TT::Slash;        break;
            case '%': single = TT::Percent;      break;
            case '=': single = TT::Eq;           break;
            case '<': single = TT::Lt;           break;
            case '>': single = TT::Gt;           break;
            case '!': single = TT::Bang;         break;
            case ';': single = TT::Semi;         break;
            case ':': single = TT::Colon;        break;
            case ',': single = TT::Comma;        break;
            case '.': single = TT::Dot;          break;
            case '(': single = TT::OpenParen;    break;
            case ')': single = TT::CloseParen;   break;
            case '{': single = TT::OpenCurly;    break;
            case '}': single = TT::CloseCurly;   break;
            case '[': single = TT::OpenBracket;  break;
            case ']': single = TT::CloseBracket; break;
            default:
                std::cerr << "[Vel] Unexpected character '" << c
                          << "' at line " << m_line << ":" << m_col << "\n";
                exit(EXIT_FAILURE);
            }
            advance();
            tokens.push_back({single, tok_line, tok_col});
        }

        tokens.push_back({TT::Eof, m_line, m_col});
        return tokens;
    }

private:

    bool at_end(size_t offset = 0) const
    {
        return m_pos + offset >= m_src.size();
    }

    char peek(size_t offset = 0) const
    {
        if (at_end(offset)) return '\0';
        return m_src[m_pos + offset];
    }

    char advance(size_t count = 1)
    {
        char c = '\0';
        for (size_t i = 0; i < count; ++i) {
            if (at_end()) break;
            c = m_src[m_pos++];
            if (c == '\n') { m_line++; m_col = 1; }
            else            { m_col++; }
        }
        return c;
    }


    void skip_whitespace_and_comments()
    {
        while (!at_end()) {
            const char c = peek();

            // Whitespace
            if (std::isspace(c)) { advance(); continue; }

            // Line comment: // ...
            if (c == '/' && peek(1) == '/') {
                while (!at_end() && peek() != '\n') advance();
                continue;
            }

            // Block comment: /* ... */
            if (c == '/' && peek(1) == '*') {
                advance(2);
                while (!at_end()) {
                    if (peek() == '*' && peek(1) == '/') { advance(2); break; }
                    advance();
                }
                continue;
            }

            break;
        }
    }


    Token read_string(int line, int col)
    {
        advance(); // consume opening "
        std::string val;
        while (!at_end() && peek() != '"') {
            if (peek() == '\\') {
                advance();
                switch (peek()) {
                case 'n':  val += '\n'; advance(); break;
                case 't':  val += '\t'; advance(); break;
                case '"':  val += '"';  advance(); break;
                case '\\': val += '\\'; advance(); break;
                default:
                    val += '\\';
                    val += peek();
                    advance();
                    break;
                }
            } else {
                val += advance();
            }
        }
        if (at_end()) {
            std::cerr << "[Vel] Unterminated string at line " << line << ":" << col << "\n";
            exit(EXIT_FAILURE);
        }
        advance(); // consume closing "
        return {TT::StrLit, line, col, val};
    }

    Token read_number(int line, int col)
    {
        std::string val;
        bool is_float = false;
        while (!at_end() && std::isdigit(peek())) val += advance();
        if (!at_end() && peek() == '.' && std::isdigit(peek(1))) {
            is_float = true;
            val += advance(); // '.'
            while (!at_end() && std::isdigit(peek())) val += advance();
        }
        return {is_float ? TT::FloatLit : TT::IntLit, line, col, val};
    }

    Token read_word(int line, int col)
    {
        std::string val;
        while (!at_end() && (std::isalnum(peek()) || peek() == '_'))
            val += advance();

        // Keyword table
        if (val == "fn")       return {TT::Fn,       line, col};
        if (val == "struct")    return {TT::Struct,    line, col};
        if (val == "return")   return {TT::Return,    line, col};
        if (val == "let")      return {TT::Let,       line, col};
        if (val == "mut")      return {TT::Mut,       line, col};
        if (val == "if")       return {TT::If,        line, col};
        if (val == "elif")     return {TT::Elif,      line, col};
        if (val == "else")     return {TT::Else,      line, col};
        if (val == "loop")     return {TT::Loop,      line, col};
        if (val == "while")    return {TT::While,     line, col};
        if (val == "break")    return {TT::Break,     line, col};
        if (val == "continue") return {TT::Continue,  line, col};
        if (val == "print")    return {TT::Print,     line, col};
        if (val == "true")     return {TT::BoolLit,   line, col, "true"};
        if (val == "false")    return {TT::BoolLit,   line, col, "false"};
        if (val == "int")      return {TT::TyInt,     line, col};
        if (val == "float")    return {TT::TyFloat,   line, col};
        if (val == "str")      return {TT::TyStr,     line, col};
        if (val == "bool")     return {TT::TyBool,    line, col};

        return {TT::Ident, line, col, val};
    }

    std::string m_src;
    size_t      m_pos  { 0 };
    int         m_line { 1 };
    int         m_col  { 1 };
};
