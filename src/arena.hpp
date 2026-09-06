#pragma once

#include <cstddef>
#include <cstdlib>
#include <new>
#include <utility>

class Arena {
public:
    explicit Arena(const size_t bytes)
        : m_size(bytes)
        , m_buf(static_cast<std::byte*>(malloc(bytes)))
    {
        if (!m_buf && bytes != 0) throw std::bad_alloc{};
    }

    ~Arena() { free(m_buf); }

    Arena(const Arena&)            = delete;
    Arena& operator=(const Arena&) = delete;

    template <typename T, typename... Args>
    [[nodiscard]] T* alloc(Args&&... args)
    {
        constexpr size_t alignment = alignof(T);
        const size_t padding = (alignment - (m_offset % alignment)) % alignment;
        if (m_offset > m_size || padding > m_size - m_offset
            || sizeof(T) > m_size - m_offset - padding) {
            throw std::bad_alloc{};
        }
        m_offset += padding;
        auto* ptr = new (m_buf + m_offset) T(std::forward<Args>(args)...);
        m_offset += sizeof(T);
        return ptr;
    }

private:
    size_t      m_size;
    size_t      m_offset { 0 };
    std::byte*  m_buf;
};
