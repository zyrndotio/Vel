#pragma once

#include <algorithm>
#include <cassert>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "ast.hpp"

enum class CodeGenTarget { LinuxX86_64, MacOSX86_64, WindowsX86_64 };

class CodeGen {
public:
    explicit CodeGen(Program prog, CodeGenTarget target = CodeGenTarget::LinuxX86_64)
        : m_prog(std::move(prog))
        , m_target(target)
    {
    }

    std::string generate()
    {
        // We collect string literals during generation, emit them at the end.

        if (m_target == CodeGenTarget::MacOSX86_64) {
            emit_line("section __TEXT,__text");
            emit_line("global _start");
        } else if (m_target == CodeGenTarget::WindowsX86_64) {
            emit_line("default rel");
            emit_line("section .text");
            emit_line("global main");
            emit_line("extern ExitProcess");
            emit_line("extern WriteFile");
            emit_line("extern GetStdHandle");
            emit_line("extern VirtualAlloc");
        } else {
            emit_line("section .text");
            emit_line("global _start");
        }

        collect_layouts();
        emit_builtins();


        std::vector<Stmt*> fns;
        std::vector<Stmt*> top;
        for (auto* s : m_prog.stmts) {
            if (std::holds_alternative<StmtFn*>(s->var)) {
                auto* fn = std::get<StmtFn*>(s->var);
                m_functions[fn->name.value.value_or("")] = {fn->params.size(), fn->return_type};
                fns.push_back(s);
            } else {
                top.push_back(s);
            }
        }

        for (auto* s : fns) gen_fn(std::get<StmtFn*>(s->var));
        emit_line("");
        emit_line(m_target == CodeGenTarget::WindowsX86_64 ? "main:" : "_start:");
        push_scope();

        for (auto* s : top) gen_stmt(s);

        emit_line("    ; implicit exit");
        if (m_target == CodeGenTarget::WindowsX86_64) {
            emit_line("    xor ecx, ecx");
            emit_line("    call ExitProcess");
        } else {
            emit_line(m_target == CodeGenTarget::MacOSX86_64 ? "    mov rax, 0x2000001" : "    mov rax, 60");
            emit_line("    xor rdi, rdi");
            emit_line("    syscall");
        }

        pop_scope();

        if (!m_data.str().empty()) {
            emit_line("");
            emit_line(m_target == CodeGenTarget::MacOSX86_64 ? "section __DATA,__data" : "section .data");
            m_out << m_data.str();
        }
        if (!m_bss.str().empty()) {
            emit_line("");
            emit_line(m_target == CodeGenTarget::MacOSX86_64 ? "section __BSS,__bss" : "section .bss");
            m_out << m_bss.str();
        }

        return m_out.str();
    }

private:

    void emit_line(const std::string& line)
    {
        m_out << line << "\n";
    }

    void emit(const std::string& s)
    {
        m_out << s;
    }


    struct Var {
        std::string name;
        size_t      stack_loc; // index from bottom of current frame
        bool        is_mut;
    };

    std::vector<std::vector<Var>> m_scopes; // scope stack
    size_t m_stack_size { 0 };

    void push_scope() { m_scopes.push_back({}); }

    void pop_scope()
    {
        if (m_scopes.empty()) return;
        size_t count = m_scopes.back().size();
        if (count > 0) {
            emit_line("    add rsp, " + std::to_string(count * 8));
            m_stack_size -= count;
        }
        m_scopes.pop_back();
    }

    void define_var(const std::string& name, bool is_mut)
    {
        // The variable occupies the slot just pushed: index = m_stack_size - 1
        m_scopes.back().push_back({name, m_stack_size - 1, is_mut});
    }

    // Returns rsp-relative offset string for named variable
    std::string var_addr(const std::string& name)
    {
        for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
            for (auto& v : *it) {
                if (v.name == name) {
                    size_t offset = (m_stack_size - v.stack_loc - 1) * 8;
                    return "QWORD [rsp + " + std::to_string(offset) + "]";
                }
            }
        }
        // Also check function params (stored in fn_params map)
        if (m_fn_params.count(name)) {
            return m_fn_params[name];
        }
        std::cerr << "[Vel] Undefined variable: " << name << "\n";
        exit(EXIT_FAILURE);
    }

    void stack_push(const std::string& reg)
    {
        emit_line("    push " + reg);
        m_stack_size++;
    }

    void stack_pop(const std::string& reg)
    {
        emit_line("    pop " + reg);
        m_stack_size--;
    }


    int m_label_count { 0 };

    std::string new_label()
    {
        return ".L" + std::to_string(m_label_count++);
    }


    int m_str_count { 0 };
    int m_aggregate_count { 0 };
    std::stringstream m_data;
    std::stringstream m_bss;
    std::unordered_map<std::string, std::vector<std::string>> m_struct_fields;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_struct_field_types;
    std::unordered_map<std::string, std::string> m_struct_vars;
    std::unordered_map<std::string, bool> m_string_vars;

    void collect_layouts()
    {
        for (auto* stmt : m_prog.stmts) {
            if (std::holds_alternative<StmtStruct*>(stmt->var)) {
                auto* structure = std::get<StmtStruct*>(stmt->var);
                std::vector<std::string> fields;
                const auto structure_name = structure->name.value.value_or("");
                for (const auto& field : structure->fields) {
                    const auto field_name = field.name.value.value_or("");
                    fields.push_back(field_name);
                    m_struct_field_types[structure_name][field_name] = type_ref_str(field.type);
                }
                m_struct_fields[structure_name] = std::move(fields);
            }
        }
    }

    // Emits a string into .data, returns (label, length)
    std::pair<std::string, size_t> intern_string(const std::string& val)
    {
        std::string label = "vel_str_" + std::to_string(m_str_count++);
        // Emit as db sequence so escapes work
        m_data << label << " db ";
        bool first = true;
        std::string chunk;
        for (char c : val) {
            if (c == '\n' || c == '\t' || c < 32) {
                if (!chunk.empty()) {
                    if (!first) m_data << ",";
                    m_data << "\"" << chunk << "\"";
                    chunk.clear();
                    first = false;
                }
                if (!first) m_data << ",";
                m_data << std::to_string((int)c);
                first = false;
            } else {
                chunk += c;
            }
        }
        if (!chunk.empty()) {
            if (!first) m_data << ",";
            m_data << "\"" << chunk << "\"";
            first = false;
        }
        if (!first) m_data << ",";
        m_data << "0\n"; // null-terminated
        m_data << label << "_len equ $ - " << label << " - 1\n"; // length without null

        return {label, val.size()};
    }


    std::string static_value(Expr* expr)
    {
        if (std::holds_alternative<ExprIntLit*>(expr->var)) return std::get<ExprIntLit*>(expr->var)->tok.value.value_or("0");
        if (std::holds_alternative<ExprBoolLit*>(expr->var)) return std::get<ExprBoolLit*>(expr->var)->tok.value.value_or("false") == "true" ? "1" : "0";
        if (std::holds_alternative<ExprStrLit*>(expr->var)) return intern_string(std::get<ExprStrLit*>(expr->var)->tok.value.value_or("")).first;
        if (std::holds_alternative<ExprArray*>(expr->var)) {
            auto label = "vel_arr_" + std::to_string(m_aggregate_count++);
            auto& elements = std::get<ExprArray*>(expr->var)->elements;
            std::vector<std::string> values;
            for (auto* element : elements) values.push_back(static_value(element));
            m_data << label << " dq " << values.size() << "\n";
            m_data << label << "_data dq ";
            for (size_t i = 0; i < values.size(); ++i) {
                if (i) m_data << ",";
                m_data << values[i];
            }
            m_data << "\n";
            return label;
        }
        if (std::holds_alternative<ExprStructInit*>(expr->var)) {
            auto* init = std::get<ExprStructInit*>(expr->var);
            auto label = "vel_struct_" + std::to_string(m_aggregate_count++);
            std::unordered_map<std::string, Expr*> values;
            for (const auto& [field, value] : init->fields) values[field.value.value_or("")] = value;
            const auto& fields = m_struct_fields[init->name.value.value_or("")];
            std::vector<std::string> field_values;
            for (const auto& field : fields) field_values.push_back(static_value(values.at(field)));
            m_data << label << " dq ";
            for (size_t i = 0; i < field_values.size(); ++i) {
                if (i) m_data << ",";
                m_data << field_values[i];
            }
            m_data << "\n";
            return label;
        }
        return "0";
    }

    void emit_runtime_helpers()
    {
        if (m_target == CodeGenTarget::WindowsX86_64) {
            emit_line(R"(
; vel_alloc: rdi = byte count, returns pointer in rax or zero
vel_alloc:
    mov rdx, rdi
    xor ecx, ecx
    mov r8d, 0x3000
    mov r9d, 4
    sub rsp, 40
    call VirtualAlloc
    add rsp, 40
    ret
vel_print_cstr:
    mov rdi, rsi
    xor rdx, rdx
.count:
    cmp byte [rdi + rdx], 0
    je .write
    inc rdx
    jmp .count
.write:
    mov rsi, rdi
    jmp vel_print_str
vel_concat:
    push r12
    push r13
    push r14
    push r15
    mov r12, rdi
    mov r13, rsi
    xor r14, r14
.len1:
    cmp byte [r12 + r14], 0
    je .len2_start
    inc r14
    jmp .len1
.len2_start:
    xor r15, r15
.len2:
    cmp byte [r13 + r15], 0
    je .allocate
    inc r15
    jmp .len2
.allocate:
    mov rdi, r14
    add rdi, r15
    inc rdi
    call vel_alloc
    test rax, rax
    jz .done
    mov rdx, rax
    mov rdi, rax
    mov rsi, r12
    mov rcx, r14
    rep movsb
    mov rsi, r13
    mov rcx, r15
    rep movsb
    mov byte [rdi], 0
    mov rax, rdx
.done:
    pop r15
    pop r14
    pop r13
    pop r12
    ret
            )");
        } else {
            const bool mac = m_target == CodeGenTarget::MacOSX86_64;
            emit_line(mac ? R"(
; vel_alloc: rdi = byte count, returns pointer in rax or zero
vel_alloc:
    mov rsi, rdi
    xor edi, edi
    mov edx, 3
    mov r10d, 0x1002
    mov r8, -1
    xor r9d, r9d
    mov eax, 0x20000c5
    syscall
    cmp rax, -1
    je .fail
    ret
.fail:
    xor eax, eax
    ret
)" : R"(
; vel_alloc: rdi = byte count, returns pointer in rax or zero
vel_alloc:
    mov rsi, rdi
    xor edi, edi
    mov edx, 3
    mov r10d, 34
    mov r8, -1
    xor r9d, r9d
    mov eax, 9
    syscall
    cmp rax, -1
    je .fail
    ret
.fail:
    xor eax, eax
    ret
)");
            emit_line(R"(
vel_print_cstr:
    mov rdi, rsi
    xor rdx, rdx
.count:
    cmp byte [rdi + rdx], 0
    je .write
    inc rdx
    jmp .count
.write:
    mov rsi, rdi
    jmp vel_print_str
vel_concat:
    push r12
    push r13
    push r14
    push r15
    mov r12, rdi
    mov r13, rsi
    xor r14, r14
.len1:
    cmp byte [r12 + r14], 0
    je .len2_start
    inc r14
    jmp .len1
.len2_start:
    xor r15, r15
.len2:
    cmp byte [r13 + r15], 0
    je .allocate
    inc r15
    jmp .len2
.allocate:
    mov rdi, r14
    add rdi, r15
    inc rdi
    call vel_alloc
    test rax, rax
    jz .done
    mov rdx, rax
    mov rdi, rax
    mov rsi, r12
    mov rcx, r14
    rep movsb
    mov rsi, r13
    mov rcx, r15
    rep movsb
    mov byte [rdi], 0
    mov rax, rdx
.done:
    pop r15
    pop r14
    pop r13
    pop r12
    ret
            )");
        }

        if (m_target == CodeGenTarget::WindowsX86_64) {
            emit_line(R"(
vel_bounds_fail:
    mov ecx, 1
    sub rsp, 40
    call ExitProcess
    hlt
            )");
        } else if (m_target == CodeGenTarget::MacOSX86_64) {
            emit_line(R"(
vel_bounds_fail:
    mov eax, 0x2000001
    mov edi, 1
    syscall
    hlt
            )");
        } else {
            emit_line(R"(
vel_bounds_fail:
    mov eax, 60
    mov edi, 1
    syscall
    hlt
            )");
        }
    }

    void emit_builtins()
    {
        // print_int: prints integer in rax to stdout
        std::string builtins = R"(
; ── vel_print_int ───────────────────────────────────────────────────────────
; Input: rax = signed 64-bit integer
; Clobbers: rax, rbx, rcx, rdx, rsi, rdi, r8
vel_print_int:
    push rbp
    mov rbp, rsp
    sub rsp, 32          ; local buffer (20 digits + sign + null)
    mov r8, 0            ; negative flag
    test rax, rax
    jns .pos
    neg rax
    mov r8, 1
.pos:
    lea rsi, [rbp - 1]
    mov byte [rsi], 0    ; null terminator (unused but safe)
    mov rbx, 10
    mov rcx, 0           ; digit count
.digit_loop:
    xor rdx, rdx
    div rbx
    add dl, '0'
    dec rsi
    mov [rsi], dl
    inc rcx
    test rax, rax
    jnz .digit_loop
    test r8, r8
    jz .no_neg
    dec rsi
    mov byte [rsi], '-'
    inc rcx
.no_neg:
    ; write(1, rsi, rcx)
    mov rax, 1
    mov rdi, 1
    mov rdx, rcx
    syscall
    leave
    ret

; ── vel_print_str ────────────────────────────────────────────────────────────
; Input: rsi = pointer to string, rdx = length
vel_print_str:
    mov rax, 1
    mov rdi, 1
    syscall
    ret

; ── vel_print_bool ───────────────────────────────────────────────────────────
; Input: rax = 0 (false) or 1 (true)
vel_print_bool:
    test rax, rax
    jz .false
    mov rsi, vel_true_str
    mov rdx, 4
    jmp .done
.false:
    mov rsi, vel_false_str
    mov rdx, 5
.done:
    mov rax, 1
    mov rdi, 1
    syscall
    ret

; ── vel_print_newline ─────────────────────────────────────────────────────────
vel_print_newline:
    mov rax, 1
    mov rdi, 1
    mov rsi, vel_newline
    mov rdx, 1
    syscall
    ret
        )";
        if (m_target == CodeGenTarget::MacOSX86_64) {
            size_t pos = 0;
            while ((pos = builtins.find("mov rax, 1", pos)) != std::string::npos) {
                builtins.replace(pos, 10, "mov rax, 0x2000004");
                pos += 18;
            }
        } else if (m_target == CodeGenTarget::WindowsX86_64) {
            builtins = R"(
; Windows x64 builtins. Arguments follow the Win64 calling convention.
vel_print_str:
    sub rsp, 40
    mov rcx, -11
    call GetStdHandle
    mov rcx, rax
    mov r8, rdx
    mov rdx, rsi
    lea r9, [rsp + 32]
    mov qword [rsp + 32], 0
    call WriteFile
    add rsp, 40
    ret
vel_print_int:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    xor r8, r8
    test rax, rax
    jns .pos
    neg rax
    mov r8, 1
.pos:
    lea rsi, [rbp - 1]
    mov byte [rsi], 0
    mov rbx, 10
    xor rcx, rcx
.digit_loop:
    xor rdx, rdx
    div rbx
    add dl, '0'
    dec rsi
    mov [rsi], dl
    inc rcx
    test rax, rax
    jnz .digit_loop
    test r8, r8
    jz .write
    dec rsi
    mov byte [rsi], '-'
    inc rcx
.write:
    mov rdx, rcx
    call vel_print_str
    leave
    ret
vel_print_bool:
    test rax, rax
    jz .false
    mov rsi, vel_true_str
    mov rdx, 4
    jmp vel_print_str
.false:
    mov rsi, vel_false_str
    mov rdx, 5
    jmp vel_print_str
vel_print_newline:
    mov rsi, vel_newline
    mov rdx, 1
    jmp vel_print_str
            )";
        }
        emit_line(builtins);
        emit_runtime_helpers();

        // Reserve space in data section for built-in strings
        m_data << "vel_newline db 10\n";
        m_data << "vel_true_str db \"true\"\n";
        m_data << "vel_false_str db \"false\"\n";
        if (m_target == CodeGenTarget::WindowsX86_64) {
            m_bss << "vel_stdout_handle resq 1\n";
        }
    }

    std::string struct_type_of_expr(Expr* expr)
    {
        if (std::holds_alternative<ExprIdent*>(expr->var)) return m_struct_vars[std::get<ExprIdent*>(expr->var)->tok.value.value_or("")];
        if (std::holds_alternative<ExprStructInit*>(expr->var)) return std::get<ExprStructInit*>(expr->var)->name.value.value_or("");
        if (std::holds_alternative<ExprField*>(expr->var)) {
            auto* field = std::get<ExprField*>(expr->var);
            auto parent = struct_type_of_expr(field->object);
            auto types = m_struct_field_types.find(parent);
            if (types != m_struct_field_types.end() && types->second.contains(field->field.value.value_or(""))) {
                auto type = types->second.at(field->field.value.value_or(""));
                if (m_struct_fields.contains(type)) return type;
            }
        }
        return {};
    }

    bool is_string_expr(Expr* expr)
    {
        if (std::holds_alternative<ExprStrLit*>(expr->var)) return true;
        if (std::holds_alternative<ExprIdent*>(expr->var)) return m_string_vars.contains(std::get<ExprIdent*>(expr->var)->tok.value.value_or(""));
        if (std::holds_alternative<ExprParen*>(expr->var)) return is_string_expr(std::get<ExprParen*>(expr->var)->inner);
        if (std::holds_alternative<ExprField*>(expr->var)) {
            auto* field = std::get<ExprField*>(expr->var);
            auto parent = struct_type_of_expr(field->object);
            auto types = m_struct_field_types.find(parent);
            return types != m_struct_field_types.end() && types->second[field->field.value.value_or("")] == "str";
        }
        if (std::holds_alternative<ExprBinary*>(expr->var)) {
            auto* binary = std::get<ExprBinary*>(expr->var);
            return binary->op.type == TT::Plus && is_string_expr(binary->lhs) && is_string_expr(binary->rhs);
        }
        return false;
    }

    // Result is pushed onto stack (top of stack = rax after gen_expr)

    void gen_expr(Expr* expr)
    {
        std::visit([&](auto* node) { gen_expr_node(node); }, expr->var);
    }

    void gen_expr_node(ExprIntLit* n)
    {
        emit_line("    mov rax, " + n->tok.value.value_or("0"));
        stack_push("rax");
    }

    void gen_expr_node(ExprFloatLit* n)
    {
        // For now: store float as scaled integer (multiply by 1000)
        // Full float support (SSE) is a future pass
        double val = std::stod(n->tok.value.value_or("0.0"));
        long long scaled = static_cast<long long>(val * 1000.0);
        emit_line("    mov rax, " + std::to_string(scaled) + " ; float*1000: " + n->tok.value.value_or("0.0"));
        stack_push("rax");
    }

    void gen_expr_node(ExprStrLit* n)
    {
        auto [label, len] = intern_string(n->tok.value.value_or(""));
        // Push pointer to string label — we encode the label index in rax
        // and store a pointer-sized value: address of label
        // For now: push address as a tagged pointer (str type)
        emit_line("    mov rax, " + label);
        stack_push("rax");
        // Also push length for print to use (packed: [ptr, len])
        // We'll handle this in print by using _len symbol
        (void)len;
    }

    void gen_expr_node(ExprBoolLit* n)
    {
        bool val = (n->tok.value.value_or("false") == "true");
        emit_line("    mov rax, " + std::string(val ? "1" : "0"));
        stack_push("rax");
    }

    void gen_expr_node(ExprIdent* n)
    {
        auto addr = var_addr(n->tok.value.value_or(""));
        emit_line("    mov rax, " + addr);
        stack_push("rax");
    }

    void gen_expr_node(ExprParen* n)
    {
        gen_expr(n->inner);
    }

    void gen_expr_node(ExprUnary* n)
    {
        gen_expr(n->operand);
        stack_pop("rax");
        if (n->op.type == TT::Minus) {
            emit_line("    neg rax");
        } else if (n->op.type == TT::Bang) {
            emit_line("    test rax, rax");
            emit_line("    setz al");
            emit_line("    movzx rax, al");
        }
        stack_push("rax");
    }

    void gen_expr_node(ExprBinary* n)
    {
        if (n->op.type == TT::Plus && is_string_expr(n->lhs) && is_string_expr(n->rhs)) {
            gen_expr(n->lhs);
            gen_expr(n->rhs);
            stack_pop("rsi");
            stack_pop("rdi");
            emit_line("    call vel_concat");
            stack_push("rax");
            return;
        }
        gen_expr(n->lhs);
        gen_expr(n->rhs);
        stack_pop("rbx"); // rhs
        stack_pop("rax"); // lhs

        switch (n->op.type) {
        case TT::Plus:
            emit_line("    add rax, rbx");
            break;
        case TT::Minus:
            emit_line("    sub rax, rbx");
            break;
        case TT::Star:
            emit_line("    imul rax, rbx");
            break;
        case TT::Slash:
            emit_line("    cqo");           // sign-extend rax into rdx:rax
            emit_line("    idiv rbx");
            break;
        case TT::Percent:
            emit_line("    cqo");
            emit_line("    idiv rbx");
            emit_line("    mov rax, rdx"); // remainder
            break;
        case TT::EqEq:
            emit_line("    cmp rax, rbx");
            emit_line("    sete al");
            emit_line("    movzx rax, al");
            break;
        case TT::BangEq:
            emit_line("    cmp rax, rbx");
            emit_line("    setne al");
            emit_line("    movzx rax, al");
            break;
        case TT::Lt:
            emit_line("    cmp rax, rbx");
            emit_line("    setl al");
            emit_line("    movzx rax, al");
            break;
        case TT::LtEq:
            emit_line("    cmp rax, rbx");
            emit_line("    setle al");
            emit_line("    movzx rax, al");
            break;
        case TT::Gt:
            emit_line("    cmp rax, rbx");
            emit_line("    setg al");
            emit_line("    movzx rax, al");
            break;
        case TT::GtEq:
            emit_line("    cmp rax, rbx");
            emit_line("    setge al");
            emit_line("    movzx rax, al");
            break;
        case TT::And:
            emit_line("    test rax, rax");
            emit_line("    setnz al");
            emit_line("    test rbx, rbx");
            emit_line("    setnz bl");
            emit_line("    and al, bl");
            emit_line("    movzx rax, al");
            break;
        case TT::Or:
            emit_line("    or rax, rbx");
            emit_line("    setnz al");
            emit_line("    movzx rax, al");
            break;
        default:
            std::cerr << "[Vel] Unhandled binary op\n";
            exit(EXIT_FAILURE);
        }

        stack_push("rax");
    }

    struct FunctionInfo {
        size_t  arity;
        VelType return_type;
    };

    [[noreturn]] void unsupported_aggregate(const char* feature)
    {
        throw std::runtime_error(std::string("code generation for ") + feature + " is not implemented for this target");
    }

    void gen_expr_node(ExprArray* n)
    {
        Expr wrapper;
        wrapper.var = n;
        emit_line("    mov rax, " + static_value(&wrapper));
        stack_push("rax");
    }
    void gen_expr_node(ExprIndex* n)
    {
        gen_expr(n->object);
        gen_expr(n->index);
        stack_pop("rbx");
        stack_pop("rax");
        emit_line("    cmp rbx, 0");
        emit_line("    jl vel_bounds_fail");
        emit_line("    cmp rbx, QWORD [rax]");
        emit_line("    jae vel_bounds_fail");
        emit_line("    mov rax, QWORD [rax + rbx * 8 + 8]");
        stack_push("rax");
    }
    void gen_expr_node(ExprField* n)
    {
        std::string type_name = struct_type_of_expr(n->object);
        auto fields = m_struct_fields.find(type_name);
        if (fields == m_struct_fields.end()) unsupported_aggregate("struct field access");
        auto it = std::find(fields->second.begin(), fields->second.end(), n->field.value.value_or(""));
        if (it == fields->second.end()) unsupported_aggregate("unknown struct field");
        gen_expr(n->object);
        stack_pop("rax");
        emit_line("    mov rax, QWORD [rax + " + std::to_string(std::distance(fields->second.begin(), it) * 8) + "]");
        stack_push("rax");
    }
    void gen_expr_node(ExprStructInit* n)
    {
        Expr wrapper;
        wrapper.var = n;
        emit_line("    mov rax, " + static_value(&wrapper));
        stack_push("rax");
    }

    void gen_expr_node(ExprCall* n)
    {
        auto name = n->name.value.value_or("");
        auto it = m_functions.find(name);
        if (it == m_functions.end()) {
            std::cerr << "[Vel] Undefined function: " << name << "\n";
            exit(EXIT_FAILURE);
        }
        if (n->args.size() != it->second.arity) {
            std::cerr << "[Vel] Function '" << name << "' expects " << it->second.arity
                      << " argument(s), got " << n->args.size() << "\n";
            exit(EXIT_FAILURE);
        }

        // Push args left-to-right; callee accesses via [rbp+16], [rbp+24], ...
        // where [rbp+16] = last pushed = first arg (stack grows down)
        // So we push in reverse to get first arg at [rbp+16]
        for (int i = (int)n->args.size() - 1; i >= 0; i--) {
            gen_expr(n->args[i]);
        }
        emit_line("    call vel_fn_" + name);
        // Caller cleans up args
        if (!n->args.empty()) {
            emit_line("    add rsp, " + std::to_string(n->args.size() * 8));
            m_stack_size -= n->args.size();
        }
        stack_push("rax"); // return value
    }


    void gen_stmt(Stmt* stmt)
    {
        std::visit([&](auto* node) { gen_stmt_node(node); }, stmt->var);
    }

    void gen_stmt_node(StmtStruct*) { /* declaration is represented in compile-time layout metadata */ }

    void gen_stmt_node(StmtVar* n)
    {
        // Evaluate init expression first — result ends up on top of stack
        gen_expr(n->init);
        // NOW record the variable: its stack_loc is the slot that was just pushed
        // m_stack_size was incremented by the final push inside gen_expr
        // So var lives at stack_loc = m_stack_size - 1
        const auto name = n->name.value.value_or("");
        define_var(name, n->is_mut);
        if (is_string_expr(n->init) || (n->type_ref && n->type_ref->kind == TypeKind::Scalar && n->type_ref->scalar == VelType::Str))
            m_string_vars[name] = true;
        if (n->type_ref && n->type_ref->kind == TypeKind::Struct)
            m_struct_vars[name] = n->type_ref->name;
    }

    void gen_stmt_node(StmtAssign* n)
    {
        auto name = n->name.value.value_or("");

        // Find the var and check mutability
        bool found = false;
        for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
            for (auto& v : *it) {
                if (v.name == name) {
                    if (!v.is_mut) {
                        std::cerr << "[Vel] Cannot assign to immutable variable '" << name << "'\n";
                        exit(EXIT_FAILURE);
                    }
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
        if (!found) {
            std::cerr << "[Vel] Undefined variable '" << name << "'\n";
            exit(EXIT_FAILURE);
        }

        gen_expr(n->value);
        stack_pop("rax");
        emit_line("    mov " + var_addr(name) + ", rax");
    }

    void gen_stmt_node(StmtReturn* n)
    {
        if (!m_current_return_type) {
            std::cerr << "[Vel] return used outside of a function\n";
            exit(EXIT_FAILURE);
        }
        if (*m_current_return_type == VelType::Void && n->value) {
            std::cerr << "[Vel] void function cannot return a value\n";
            exit(EXIT_FAILURE);
        }
        if (*m_current_return_type != VelType::Void && !n->value) {
            std::cerr << "[Vel] function must return a value of type "
                      << veltype_str(*m_current_return_type) << "\n";
            exit(EXIT_FAILURE);
        }
        m_current_fn_has_return = true;
        if (n->value) {
            gen_expr(*n->value);
            stack_pop("rax");
        } else {
            emit_line("    xor rax, rax");
        }
        emit_line("    leave");
        emit_line("    ret");
    }

    void gen_stmt_node(StmtPrint* n)
    {
        auto* expr = n->value;

        if (is_string_expr(expr) && !std::holds_alternative<ExprStrLit*>(expr->var)) {
            gen_expr(expr);
            stack_pop("rsi");
            emit_line("    call vel_print_cstr");
        } else if (std::holds_alternative<ExprStrLit*>(expr->var)) {
            auto* sl = std::get<ExprStrLit*>(expr->var);
            auto [label, len] = intern_string(sl->tok.value.value_or(""));
            emit_line("    mov rsi, " + label);
            emit_line("    mov rdx, " + label + "_len");
            emit_line("    call vel_print_str");
        } else if (std::holds_alternative<ExprBoolLit*>(expr->var)) {
            gen_expr(expr);
            stack_pop("rax");
            emit_line("    call vel_print_bool");
        } else {
            // int, ident, binary expr, call, etc. — evaluate and print as integer
            gen_expr(expr);
            stack_pop("rax");
            emit_line("    call vel_print_int");
        }
        emit_line("    call vel_print_newline");
    }

    void gen_stmt_node(StmtIf* n)
    {
        std::string else_label = new_label();
        std::string end_label  = new_label();

        gen_expr(n->cond);
        stack_pop("rax");
        emit_line("    test rax, rax");
        emit_line("    jz " + else_label);

        // Then branch
        push_scope();
        for (auto* s : n->then_scope->stmts) gen_stmt(s);
        pop_scope();

        emit_line("    jmp " + end_label);
        emit_line(else_label + ":");

        // Else / elif branch
        if (n->else_branch) {
            std::visit([&](auto* branch) {
                if constexpr (std::is_same_v<decltype(branch), StmtIf*>) {
                    gen_stmt_node(branch);
                } else {
                    push_scope();
                    for (auto* s : branch->stmts) gen_stmt(s);
                    pop_scope();
                }
            }, (*n->else_branch)->var);
        }

        emit_line(end_label + ":");
    }

        // Emit cleanup for scopes that will be bypassed by a control-flow jump.
    // This does not mutate the compile-time stack model because the jump ends
    // the current path; normal fall-through still uses the existing model.
    void emit_scope_cleanup(size_t target_depth)
    {
        if (target_depth > m_scopes.size()) return;
        size_t slots = 0;
        for (size_t i = target_depth; i < m_scopes.size(); ++i)
            slots += m_scopes[i].size();
        if (slots > 0)
            emit_line("    add rsp, " + std::to_string(slots * 8));
    }

    void gen_stmt_node(StmtWhile* n)
    {
        LoopLabels loop {new_label(), new_label(), m_scopes.size()};
        m_loops.push_back(loop);
        emit_line(loop.continue_label + ":");
        gen_expr(n->cond);
        stack_pop("rax");
        emit_line("    test rax, rax");
        emit_line("    jz " + loop.break_label);
        push_scope();
        for (auto* s : n->body->stmts) gen_stmt(s);
        pop_scope();
        emit_line("    jmp " + loop.continue_label);
        emit_line(loop.break_label + ":");
        m_loops.pop_back();
    }
    void gen_stmt_node(StmtLoop* n)
    {
        LoopLabels loop {new_label(), new_label(), m_scopes.size()};
        m_loops.push_back(loop);
        emit_line(loop.continue_label + ":");
        push_scope();
        for (auto* s : n->body->stmts) gen_stmt(s);
        pop_scope();
        emit_line("    jmp " + loop.continue_label);
        emit_line(loop.break_label + ":");
        m_loops.pop_back();
    }
    void gen_stmt_node(StmtBreak*)
    {
        if (m_loops.empty()) {
            std::cerr << "[Vel] break used outside of a loop\n";
            exit(EXIT_FAILURE);
        }
        const auto& loop = m_loops.back();
        emit_scope_cleanup(loop.scope_depth);
        emit_line("    jmp " + loop.break_label);
    }
    void gen_stmt_node(StmtContinue*)
    {
        if (m_loops.empty()) {
            std::cerr << "[Vel] continue used outside of a loop\n";
            exit(EXIT_FAILURE);
        }
        const auto& loop = m_loops.back();
        emit_scope_cleanup(loop.scope_depth);
        emit_line("    jmp " + loop.continue_label);
    }

    void gen_stmt_node(StmtScope* n)
    {
        push_scope();
        for (auto* s : n->scope->stmts) gen_stmt(s);
        pop_scope();
    }

    void gen_fn(StmtFn* n)
    {
        std::string label = "vel_fn_" + n->name.value.value_or("");
        emit_line("");
        emit_line("; fn " + n->name.value.value_or("") + ":");
        emit_line(label + ":");
        emit_line("    push rbp");
        emit_line("    mov rbp, rsp");

        // Map param names to rbp-relative addresses
        // Args are pushed right-to-left by caller, so first param is at [rbp+16]
        auto previous_return_type = m_current_return_type;
        bool previous_has_return = m_current_fn_has_return;
        m_current_return_type = n->return_type;
        m_current_fn_has_return = false;

        m_fn_params.clear();
        for (size_t i = 0; i < n->params.size(); i++) {
            std::string addr = "QWORD [rbp + " + std::to_string(16 + i * 8) + "]";
            m_fn_params[n->params[i].name.value.value_or("")] = addr;
        }

        size_t saved_stack = m_stack_size;
        m_stack_size = 0;
        push_scope();

        for (auto* s : n->body->stmts) gen_stmt(s);

        pop_scope();
        m_stack_size = saved_stack;
        m_fn_params.clear();
        if (n->return_type != VelType::Void && !m_current_fn_has_return) {
            std::cerr << "[Vel] function '" << n->name.value.value_or("")
                      << "' must return a value of type " << veltype_str(n->return_type) << "\n";
            exit(EXIT_FAILURE);
        }
        m_current_return_type = previous_return_type;
        m_current_fn_has_return = previous_has_return;

        // Default return for void functions.
        emit_line("    xor rax, rax");
        emit_line("    leave");
        emit_line("    ret");
    }

    void gen_stmt_node(StmtFn* n)
    {
        gen_fn(n);
    }

    void gen_stmt_node(StmtExpr* n)
    {
        gen_expr(n->expr);
        // Discard result (pop it)
        stack_pop("rax");
    }


    Program           m_prog;
    CodeGenTarget     m_target;
    std::stringstream m_out;
    struct LoopLabels {
        std::string continue_label;
        std::string break_label;
        size_t      scope_depth;
    };

    std::vector<LoopLabels> m_loops;
    std::unordered_map<std::string, std::string> m_fn_params;
    std::unordered_map<std::string, FunctionInfo> m_functions;
    std::optional<VelType> m_current_return_type;
    bool m_current_fn_has_return {false};
};
