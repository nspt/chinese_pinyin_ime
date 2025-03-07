#include "query.h"

namespace pinyin_ime {

Query::Query(BasicTrie<Dict> &dict_trie) noexcept
    : m_dict_trie_ref{ dict_trie }
{}

Query::Query(BasicTrie<Dict> &dict_trie, PinYin::TokenSpan tokens) noexcept
    : m_dict_trie_ref{ dict_trie }, m_tokens{ tokens }
{
    exec(m_tokens);
}

Query::Query(Query&& other) noexcept
    : m_dict_trie_ref{ other.m_dict_trie_ref },
      m_dict{ other.m_dict },
      m_tokens{ other.m_tokens },
      m_items{ std::move(other.m_items) }
{
    other.clear();
}

Query& Query::operator=(Query &&other) noexcept
{
    m_dict_trie_ref = other.m_dict_trie_ref;
    m_dict = other.m_dict;
    m_tokens = other.m_tokens;
    m_items = std::move(other.m_items);
    other.clear();
    return *this;
}

bool Query::exec(PinYin::TokenSpan tokens) noexcept
{
    try {
        m_tokens = tokens;
        std::string acronym;
        for (auto &token : tokens) {
            if (!token.m_token.empty())
                acronym.push_back(token.m_token.front());
        }
        m_dict = &(m_dict_trie_ref.get().data(acronym));
        m_items = m_dict->search(tokens);
        return true;
    } catch (const std::exception &e) {
        m_dict = nullptr;
        m_items.clear();
        return false;
    }
}

bool Query::is_active() const noexcept
{
    return !m_tokens.empty();
}

PinYin::TokenSpan Query::tokens() const noexcept
{
    return m_tokens;
}

Dict* Query::dict() const noexcept
{
    return m_dict;
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
    m_dict = nullptr;
    m_tokens = {};
    m_items.clear();
}

} // namespace pinyin_ime