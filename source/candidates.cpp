#include "candidates.h"

namespace pinyin_ime {

void Candidates::add_query(const Query &query)
{
    m_queries.emplace_back(query);
}

size_t Candidates::size() const noexcept
{
    size_t s{ 0 };
    for (auto &query : m_queries) {
        s += query.size();
    }
    return s;
}

bool Candidates::empty() const noexcept
{
    if (m_queries.empty())
        return true;
    for (auto &query : m_queries)
        if (!query.empty())
            return false;
    return true;
}

Candidates::QueryRef Candidates::query_of(size_t idx)
{
    for (auto &query : m_queries) {
        auto query_size{ query.size() };
        if (idx >= query_size) {
            idx = idx - query_size;
            continue;
        }
        return query;
    }
    throw std::out_of_range{ "Index out of range" };
}

Query::ItemCRef Candidates::operator[](size_t idx) const noexcept
{
    for (auto &query : m_queries) {
        auto query_size{ query.size() };
        if (idx >= query_size) {
            idx = idx - query_size;
            continue;
        }
        return query[idx];
    }
    return m_queries[0][0]; // noexcept, but undefined behaviour
}

void Candidates::clear() noexcept
{
    m_queries.clear();
}

Candidates::Iterator Candidates::begin() const noexcept
{
    return Iterator{ this, 0 };
}

Candidates::Iterator Candidates::end() const noexcept
{
    return Iterator{ this, size() };
}

} // namespace pinyin_ime