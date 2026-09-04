# Vel Runtime ABI (v0.3.2 Design Boundary)

This document records the current native runtime contract and the v0.3.2 design boundary for Vel aggregate values. It is intentionally explicit about what is implemented and what remains unstable; it does not claim a completed ownership, garbage-collection, or recursive aggregate model.

## Value model

Scalar values currently occupy one 64-bit stack slot. String expressions evaluate to a pointer to a NUL-terminated byte buffer. The initial native array representation is a three-word header containing length, capacity, and a data pointer, followed by 64-bit element slots. Array literals now allocate this storage dynamically. Resizing, mutation, struct layout, and ownership semantics remain in progress.

String literals are emitted into the target data section. String concatenation allocates a fresh buffer whose size is `length(lhs) + length(rhs) + 1`, copies both operands, appends a NUL terminator, and returns the new pointer. This replaces the former fixed 64 KiB global concatenation buffer.

## Allocation entry point

The internal assembly helper `vel_alloc` accepts a byte count in `rdi` and returns a writable pointer in `rax`. A zero return value indicates allocation failure.

| Target | Allocation mechanism |
|---|---|
| Linux x86-64 | `mmap` system call with private anonymous writable memory |
| macOS x86-64 | `mmap` system call using the macOS syscall namespace |
| Windows x86-64 | `VirtualAlloc` with reserve/commit and read-write protection |

The helper is deliberately ABI-local to generated programs. It does not expose a C++ runtime ABI.

## Concatenation contract

`vel_concat` receives the left and right NUL-terminated string pointers in `rdi` and `rsi`. It computes both lengths, checks allocation success through the return value of `vel_alloc`, copies the operands into independent storage, writes a trailing NUL, and returns the result in `rax`.

The current foundation intentionally does not reclaim concatenation buffers. Ownership and lifetime rules must be completed before mutable arrays, structs containing strings, or long-running applications are declared production-ready.

## Remaining ABI work

The v0.3.2 runtime phase must add explicit string length metadata, resizing and mutation for arrays of nested aggregates, shared struct size/alignment/field-offset computation, aggregate copy semantics, allocation-failure diagnostics, and a defined ownership or reclamation policy. These changes must preserve equivalent behavior on Linux, macOS, and Windows.

## Mutation contract

`mut values: [int] = [1, 2];` permits `values[0] = 9;` and `append(values, 3);`. Indexed writes require an in-range index. `append` grows capacity geometrically when the current length reaches capacity, copies existing elements into the new allocation, stores the new element, and updates length. Mutation of immutable arrays is rejected by the type checker.

The current implementation covers scalar element arrays. Arrays containing strings, structs, or nested arrays still require complete element layout and ownership rules and should not be treated as a stable application ABI.

## Verification

The native data-type test suite now includes a concatenation larger than the previous 64 KiB fixed buffer. It verifies that the generated program produces all expected bytes and a final newline. The fixture is executed on Linux x86-64 and cross-target assembly generation remains covered by the target tests.

> This document describes the v0.3.2 design boundary, not a completed aggregate runtime. Array literals allocate length/capacity/data-pointer headers; scalar arrays support indexed writes and append growth; and reads perform native lower- and upper-bound checks. Nested aggregate mutation, complete recursive layout, ownership, cleanup, and full string methods remain release-blocking work.

---

Copyright © 2026 Zyrndotio. Licensed under GPL-3.0.

## References

[1]: https://github.com/zyrndotio/Vel "Vel repository"
[2]: https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc "Microsoft VirtualAlloc documentation"
[3]: https://man7.org/linux/man-pages/man2/mmap.2.html "Linux mmap documentation"
[4]: https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mmap.2.html "Apple mmap documentation"
