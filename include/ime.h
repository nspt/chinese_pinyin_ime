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
    void save(std::string_view dict_file) const;
    void add_item_from_line(std::string_view line);

    CandidatesCRef candidates() const noexcept;
    CandidatesCRef search(std::string_view pinyin);
    CandidatesCRef choose(size_t idx);
    CandidatesCRef push_back(std::string_view pinyin);
    CandidatesCRef backspace(size_t count = 1);
    void finish_search(bool inc_freq = true, bool add_new_sentence = true);
    void reset_search() noexcept;

    std::vector<std::pair<PinYin::TokenSpan, std::string>> choices() const noexcept;

    PinYin::TokenSpan tokens() const noexcept;
    PinYin::TokenSpan fixed_tokens() const noexcept;
    PinYin::TokenSpan unfixed_tokens() const noexcept;

    std::string_view pinyin() const noexcept;
    std::string_view fixed_letters() const noexcept;
    std::string_view unfixed_letters() const noexcept;

private:
    struct Choice {
        Choice(PinYin::TokenSpan tokens, Dict &dict, size_t index)
            : m_tokens{ tokens }, m_dict{ dict }, m_idx{ index }
        {}
        PinYin::TokenSpan m_tokens;
        Dict& m_dict;
        size_t m_idx;
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