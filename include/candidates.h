#ifndef PINYIN_IME_CANDIDATES_H
#define PINYIN_IME_CANDIDATES_H

#include "query.h"

namespace pinyin_ime {

class Candidates {
public:
    using QueryRef = std::reference_wrapper<Query>;

    class Iterator {
    public:
        Iterator() = default;
        Iterator(const Candidates *c, size_t i)
            : m_candidates{ c }, m_idx{ i }
        {}
        const DictItem& operator*() const
        {
            return (*m_candidates)[m_idx];
        }
        const DictItem* operator->() const
        {
            return &((*m_candidates)[m_idx].get());
        }
        Iterator& operator++()
        {
            ++m_idx;
            return *this;
        }
        Iterator operator++(int)
        {
            Iterator i{ *this };
            ++m_idx;
            return i;
        }
        Iterator& operator--()
        {
            --m_idx;
            return *this;
        }
        Iterator operator--(int)
        {
            Iterator i{ *this };
            --m_idx;
            return i;
        }
        Iterator& operator+(size_t offset)
        {
            m_idx += offset;
            return *this;
        }
        Iterator& operator-(size_t offset)
        {
            m_idx -= offset;
            return *this;
        }
        std::ptrdiff_t operator-(const Iterator& rhs)
        {
            if (m_idx >= rhs.m_idx)
                return static_cast<std::ptrdiff_t>(m_idx - rhs.m_idx);
            return -(static_cast<std::ptrdiff_t>(rhs.m_idx - m_idx));
        }
        bool operator==(const Iterator& rhs)
        {
            return m_candidates == rhs.m_candidates && m_idx == rhs.m_idx;
        }
        bool operator!=(const Iterator& rhs)
        {
            return !(*this == rhs);
        }
    private:
        const Candidates* m_candidates{ nullptr };
        size_t m_idx{ 0 };

        friend class Candidates;
    };

    void push_back(Query query);
    size_t size() const noexcept;
    bool empty() const noexcept;
    std::pair<QueryRef, size_t> to_query_and_index(const Iterator& it);
    std::pair<QueryRef, size_t> to_query_and_index(size_t idx);
    Query::ItemCRef operator[](size_t idx) const noexcept;
    void clear() noexcept;
    Iterator begin() const noexcept;
    Iterator end() const noexcept;
private:
    std::vector<Query> m_queries;
};

} // namespace pinyin_ime

#endif // PINYIN_IME_CANDIDATES_H