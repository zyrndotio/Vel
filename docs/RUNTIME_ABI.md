# Vel Runtime ABI (v0.3.0 Foundation)

This document records the initial native runtime contract for Vel aggregate values. It is intentionally limited to the first runtime foundation and does not yet define ownership, garbage collection, or mutable aggregate assignment.

## Value model

Scalar values currently occupy one 64-bit stack slot. String expressions evaluate to a pointer to a NUL-terminated byte buffer. The initial native array representation is a read-only static header containing a 64-bit element count followed by 64-bit element slots. Full dynamic array storage, struct layout, and ownership semantics remain in progress.

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

The current foundation intentionally does not reclaim concatenation buffers. Ownership and lifetime rules must be designed before mutable arrays, structs containing strings, or long-running applications are declared production-ready.

## Remaining ABI work

The next runtime phase must add explicit string length metadata, dynamic array headers containing data pointer/length/capacity, shared struct size/alignment/field-offset computation, aggregate copy semantics, allocation-failure diagnostics, and a defined ownership or reclamation policy. These changes must preserve equivalent behavior on Linux, macOS, and Windows.

## Verification

The native data-type test suite now includes a concatenation larger than the previous 64 KiB fixed buffer. It verifies that the generated program produces all expected bytes and a final newline. The fixture is executed on Linux x86-64 and cross-target assembly generation remains covered by the target tests.

> This document describes the v0.3.0 foundation, not a completed aggregate runtime. Array reads now perform native lower- and upper-bound checks, but dynamic arrays, mutable aggregate writes, complete struct layout, ownership, and full string methods remain release-blocking work.

---

Copyright © 2026 Zyrndotio. Licensed under GPL-3.0.

## References

[1]: https://github.com/zyrndotio/Vel "Vel repository"
[2]: https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc "Microsoft VirtualAlloc documentation"
[3]: https://man7.org/linux/man-pages/man2/mmap.2.html "Linux mmap documentation"
[4]: https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/mmap.2.html "Apple mmap documentation"
