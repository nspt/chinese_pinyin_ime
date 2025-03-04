#ifndef PINYIN_IME_QUERY_H
#define PINYIN_IME_QUERY_H

#include "pinyin.h"
#include "dict.h"
#include "trie.h"

namespace pinyin_ime {

class Query {
public:
    using ItemCRefVecCRef = std::reference_wrapper<const Dict::ItemCRefVec>;
    using ItemCRef = std::reference_wrapper<const DictItem>;
    Query(BasicTrie<Dict> &dict_trie) noexcept;
    Query(BasicTrie<Dict> &dict_trie, PinYin::TokenSpan tokens) noexcept;
    Query(const Query&) = default;
    Query(Query&& other) noexcept;
    Query& operator=(const Query&) = default;
    Query& operator=(Query &&other) noexcept;

    bool exec(PinYin::TokenSpan tokens) noexcept;
    bool is_active() const noexcept;
    Dict* dict() const noexcept;
    PinYin::TokenSpan tokens() const noexcept;
    ItemCRefVecCRef items() const noexcept;
    size_t size() const noexcept;
    bool empty() const noexcept;
    ItemCRef operator[](size_t idx) const noexcept;
    void clear() noexcept;
private:
    std::reference_wrapper<BasicTrie<Dict>> m_dict_trie_ref;
    Dict* m_dict{ nullptr };
    PinYin::TokenSpan m_tokens;
    Dict::ItemCRefVec m_items;
};

} // namespace pinyin_ime

#endif // PINYIN_IME_QUERY_H