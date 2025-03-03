#ifndef PINYIN_IME_IME_H
#define PINYIN_IME_IME_H

#include <ranges>
#include <string_view>
#include <cerrno>
#include "trie.h"
#include "dict.h"
#include "pinyin.h"
#include "query.h"
#include "candidates.h"

namespace pinyin_ime {

class IME {
public:
    using CandidatesCRef = std::reference_wrapper<const Candidates>;
    IME() = default;
    IME(std::string_view dict_file);
    IME(const IME&) = delete;
    IME(IME&&) = delete;
    IME& operator=(const IME&) = delete;
    IME& operator=(IME&&) = delete;

    void load(std::string_view dict_file);
    void save(std::string_view dict_file);
    void add_item_from_line(std::string_view line);

    CandidatesCRef search(std::string_view pinyin);
    CandidatesCRef push_back(std::string_view pinyin);
    CandidatesCRef backspace(size_t count = 1);
    void reset_search() noexcept;

    void choose(size_t idx) noexcept;
private:
    struct Choice {
        PinYin::TokenSpan m_tokens;
        DictItem& m_item;
    };
    enum class BackspaceBehaviour {
        Token, Letter
    };
    CandidatesCRef search_impl(PinYin::TokenSpan tokens);
    DictItem line_to_item(std::string_view line);
    BasicTrie<Dict> m_dict_trie;
    Candidates m_candidates;
    std::vector<Choice> m_choices;
    PinYin m_pinyin;
};

} // namespace pinyin_ime

#endif // PINYIN_IME_IME_H