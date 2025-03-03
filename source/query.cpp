#include "query.h"

namespace pinyin_ime {

Query::Query(BasicTrie<Dict> &dict_trie)
    : m_dict_trie_ref{ dict_trie }
{}

Query::Query(BasicTrie<Dict> &dict_trie, PinYin::TokenSpan tokens)
    : m_dict_trie_ref{ dict_trie }, m_tokens{ tokens }
{
    exec(m_tokens);
}

void Query::exec(PinYin::TokenSpan tokens)
{
    m_choose_idx = s_npos;
    m_tokens = tokens;
    std::string acronym;
    for (auto &token : tokens) {
        if (!token.m_token.empty())
            acronym.push_back(token.m_token.front());
    }
    Dict &dict{ m_dict_trie_ref.get().data(acronym) };
    m_items = dict.search(tokens);
}

PinYin::TokenSpan Query::tokens() const noexcept
{
    return m_tokens;
}

void Query::select(size_t idx)
{
    if (idx >= size())
        throw std::out_of_range{ "Index out of range" };
    m_choose_idx = idx;
}

bool Query::is_selected() const noexcept
{
    return m_choose_idx != s_npos;
}

Query::ItemCRef Query::selected_item() const
{
    if (m_choose_idx == s_npos)
        throw std::logic_error{ "Query is not selected" };
    return m_items[m_choose_idx];
}

Query::ItemCRefVecCRef Query::items() const noexcept
{
    return m_items;
}

size_t Query::size() const noexcept
{
    return m_items.size();
}

bool Query::empty() const noexcept
{
    return m_items.empty();
}

Query::ItemCRef Query::operator[](size_t idx) const noexcept
{
    return m_items[idx];
}

void Query::clear() noexcept
{
    m_tokens = {};
    m_items.clear();
}

} // namespace pinyin_ime