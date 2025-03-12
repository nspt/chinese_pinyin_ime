#ifndef PINYIN_IME_IME_H
#define PINYIN_IME_IME_H

#include <ranges>
#include <string_view>
#include <cerrno>
#include "trie.h"
#include "dict.h"
#include "pinyin.h"
#include "candidates.h"

namespace pinyin_ime {

/**
 * \brief 输入法引擎（Input Method Engine）类，输入法库对外接口。
 * \details IME 管理着一个 PinYin 对象和一个 BasicTrie<Dict> 对象，其行为相当于 PinYin 对象的代理，
 *          对外部提供间接修改 PinYin 的接口（search()、push_back()、backspace()、choose()等），
 *          同时也是 PinYin 对象与 BasicTrie<Dict> 对象的中介，在 PinYin 对象发生改变后，
 *          根据 PinYin 对象的 Token 状态，前往 BasicTrie<Dict> 查询匹配的结果
 *          （借助 Query 类实现）并保存以供外部访问。
 */
class IME {
public:
    using CandidatesCRef = std::reference_wrapper<const Candidates>;

    class Choice {
    public:
        PinYin::TokenSpan tokens() const noexcept;
        std::string_view chinese() const noexcept;
    private:
        Choice(PinYin::TokenSpan tokens, Dict &dict, size_t index) noexcept;
        PinYin::TokenSpan m_tokens;
        Dict& m_dict;
        size_t m_idx;

        friend class IME;
    };
    using ChoiceVecCRef = std::reference_wrapper<const std::vector<Choice>>;

    /**
     * \brief 默认构造函数。
     */
    IME() = default;

    /**
     * \brief 构造 IME 对象，并通过 load() 从词库文件加载词典数据。
     * \throws std::exception 如果发生错误。
     */
    IME(std::string_view dict_file);

    /**
     * \brief 禁止拷贝构造。
     */
    IME(const IME&) = delete;

    /**
     * \brief 禁止移动构造。
     */
    IME(IME&&) = delete;

    /**
     * \brief 禁止拷贝赋值。
     */
    IME& operator=(const IME&) = delete;

    /**
     * \brief 禁止移动赋值。
     */
    IME& operator=(IME&&) = delete;

    /**
     * \brief 从词库文件加载词典数据。
     * \details 词库文件为文本形式，每一行包含一个 DcitItem。
     * \param dict_file 词库文件路径。
     * \throws std::exception 如果发生错误。
     */
    void load(std::string_view dict_file);

    /**
     * \brief 将词典树 BasicTrie<Dict> 保存为词库文件。
     * \details 词库文件为文本形式，每一行包含一个 DcitItem。
     * \param dict_file 词库文件路径。
     * \throws std::exception 如果发生错误。
     */
    void save(std::string_view dict_file) const;

    /**
     * \brief 将词典树 BasicTrie<Dict> 保存为词库文件。
     * \param dict_file 词库文件路径。
     * \throws std::exception 如果发生错误。
     */
    void add_item_from_line(std::string_view line);

    CandidatesCRef candidates() const noexcept;
    CandidatesCRef search(std::string_view pinyin);
    CandidatesCRef choose(size_t idx);
    CandidatesCRef push_back(std::string_view pinyin);
    CandidatesCRef backspace(size_t count = 1);
    void finish_search(bool inc_freq = true, bool add_new_sentence = true);
    void reset_search() noexcept;

    ChoiceVecCRef choices() const noexcept;

    PinYin::TokenSpan tokens() const noexcept;
    PinYin::TokenSpan fixed_tokens() const noexcept;
    PinYin::TokenSpan unfixed_tokens() const noexcept;

    std::string_view pinyin() const noexcept;
    std::string_view fixed_letters() const noexcept;
    std::string_view unfixed_letters() const noexcept;

private:
    CandidatesCRef search_impl(PinYin::TokenSpan tokens);
    DictItem line_to_item(std::string_view line);
    PinYin m_pinyin;
    BasicTrie<Dict> m_dict_trie;
    Candidates m_candidates;
    std::vector<Choice> m_choices;
};

} // namespace pinyin_ime

#endif // PINYIN_IME_IME_H