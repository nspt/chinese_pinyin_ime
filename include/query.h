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
    Query(BasicTrie<Dict> &dict_trie);
    Query(BasicTrie<Dict> &dict_trie, PinYin::TokenSpan tokens);

    void exec(PinYin::TokenSpan tokens);
    PinYin::TokenSpan tokens() const noexcept;

    void select(size_t idx);
    bool is_selected() const noexcept;
    ItemCRef selected_item() const;

    ItemCRefVecCRef items() const noexcept;
    size_t size() const noexcept;
    bool empty() const noexcept;
    ItemCRef operator[](size_t idx) const noexcept;
    void clear() noexcept;
private:
    std::reference_wrapper<BasicTrie<Dict>> m_dict_trie_ref;
    PinYin::TokenSpan m_tokens;
    Dict::ItemCRefVec m_items;
    size_t m_choose_idx{ s_npos };

    static constexpr size_t s_npos{ std::numeric_limits<size_t>::max() };
};

} // namespace pinyin_ime

#endif // PINYIN_IME_QUERY_H