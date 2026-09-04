#pragma once

#include <cstddef>
#include <cstdlib>
#include <new>

class Arena {
public:
    explicit Arena(const size_t bytes)
        : m_size(bytes)
        , m_buf(static_cast<std::byte*>(malloc(bytes)))
    {
    }

    ~Arena() { free(m_buf); }

    Arena(const Arena&)            = delete;
    Arena& operator=(const Arena&) = delete;

    template <typename T, typename... Args>
    [[nodiscard]] T* alloc(Args&&... args)
    {
        if (m_offset + sizeof(T) > m_size) {
            throw std::bad_alloc{};
        }
        auto* ptr = new (m_buf + m_offset) T(std::forward<Args>(args)...);
        m_offset += sizeof(T);
        return ptr;
    }

private:
    size_t      m_size;
    size_t      m_offset { 0 };
    std::byte*  m_buf;
};
