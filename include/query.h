#ifndef PINYIN_IME_QUERY
#define PINYIN_IME_QUERY

#include <vector>
#include <span>
#include <functional>
#include "pinyin.h"
#include "dict_item.h"

namespace pinyin_ime {

class Query {
public:
    Query() = default;
    Query(std::span<PinYin::Token> tokens, std::vector<std::reference_wrapper<DictItem>> items)
        : m_tokens{ tokens }, m_items{ std::move(items) }
    {}
    std::span<const PinYin::Token>& tokens()
    {
        return m_tokens;
    }
    const std::vector<std::reference_wrapper<DictItem>>& items()
    {
        return m_items;
    }
private:
    std::span<const PinYin::Token> m_tokens;
    std::vector<std::reference_wrapper<DictItem>> m_items;
    friend class QueryList;
    friend class IME;
};

class QueryList {
public:
    class Iterator {
    public:
        DictItem& operator*() const
        {
            if (!m_queries)
                throw std::logic_error{ "Iterator invalid" };
            auto &query{ (*m_queries)[m_query_idx] };
            return query.m_items[m_item_idx];
        }

        DictItem* operator->() const
        {
            if (!m_queries)
                throw std::logic_error{ "Iterator invalid" };
            auto &query{ (*m_queries)[m_query_idx] };
            return &(query.m_items[m_item_idx].get());
        }

        Iterator& operator++()
        {
            if (!m_queries)
                return *this;
            auto &query{ (*m_queries)[m_query_idx] };
            if (m_item_idx + 1 < query.m_items.size()) {
                ++m_item_idx;
                return *this;
            }
            if (m_query_idx + 1 < m_queries->size()) {
                ++m_query_idx;
                m_item_idx = 0;
                if ((*m_queries)[m_query_idx].m_items.empty()) {
                    m_queries = nullptr;
                    m_query_idx = 0;
                }
            } else {
                m_queries = nullptr;
                m_item_idx = m_query_idx = 0;
            }
            return *this;
        }
        bool operator==(const Iterator& other) const
        {
            return (m_queries == other.m_queries
                    && m_query_idx == other.m_query_idx
                    && m_item_idx == other.m_item_idx);
        }

        bool operator!=(const Iterator& other) const
        {
            return !(*this == other);
        }
    private:
        Iterator() = default;
        std::vector<Query> *m_queries{ nullptr };
        size_t m_query_idx{ 0 };
        size_t m_item_idx{ 0 };
        friend class IME;
    };
private:
    std::vector<Query> m_queries;
};

} // namespace pinyin_ime

#endif // PINYIN_IME_QUERY