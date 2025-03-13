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
 * \note IME 是一个状态机，改变其状态（拼音/选择）后，若有之前保存的从 IME 获取到
 *       的 Candidates、Choice 等对象，均视为失效，不可继续使用。
 */
class IME {
public:

    /**
     * \brief 表明 IME 的一次选择，存有该选择对应的拼音 Token、Dict 以及 DictItem 在 Dict 的索引。
     */
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

    /**
     * \brief 默认构造函数。
     */
    IME() = default;

    /**
     * \brief 构造 IME 对象，并通过 load() 从词库文件加载词典数据。
     * \details 词库文件为文本形式，每一行包含一个 DcitItem。
     * \param dict_file 词库文件路径。
     * \throws std::invalid_argument 如果文件内容格式不符。
     *         std::runtime_error 如果读取文件发生错误。
     *         std::exception 如果发生错误。
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
     * \brief 从词库文件加载词典数据至词库树。
     * \details 词库文件为文本形式，每一行包含一个 DcitItem。
     * \param dict_file 词库文件路径。
     * \throws std::invalid_argument 如果文件内容格式不符。
     *         std::runtime_error 如果读取文件发生错误。
     *         std::exception 如果发生错误。
     */
    void load(std::string_view dict_file);

    /**
     * \brief 将词典树 BasicTrie<Dict> 保存为词库文件。
     * \details 词库文件为文本形式，每一行包含一个 DcitItem。
     * \param dict_file 词库文件路径。
     * \throws std::runtime_error 如果写入文件发生错误。
     */
    void save(std::string_view dict_file) const;

    /**
     * \brief 将字符串形式的 DictItem 加入到词库树。
     * \param line 文本形式的 DictItem。
     * \throws std::invalid_argument 如果 line 格式不符。
     *         std::exception 如果发生错误。
     */
    void add_item_from_line(std::string_view line);

    /**
     * \brief 获取当前候选词的 const 引用。
     */
    const Candidates& candidates() const noexcept;

    /**
     * \brief 根据给定拼音搜索候选词。
     * \details 如果 pinyin 是在 IME 当前拼音的尾部上新增或减少字符，则会自动转换为对
     *          push_back() 或 backspace() 的调用。
     * \param pinyin 要搜索的拼音字符串。
     * \return 根据指定拼音搜索后，新的当前候选词的 const 引用。
     */
    const Candidates& search(std::string_view pinyin);

    /**
     * \brief 选择候选词。
     * \details 选择候选词会同时"固定"候选词对应的拼音 Token，从而影响后续拼音的解析。
     * \param idx 选择的候选词的索引。
     * \return 新的当前候选词的 const 引用。
     * \throws std::runtime_error 如果发生错误，包括 idx 超出范围。
     */
    const Candidates& choose(size_t idx);

    /**
     * \brief 尾添加拼音。
     * \param pinyin 需要添加的拼音字符串。
     * \return 新的当前候选词的 const 引用。
     * \throws std::exception 如果发生错误。
     */
    const Candidates& push_back(std::string_view pinyin);

    /**
     * \brief 退格，相当于删除拼音尾部字符，仅允许非固定的拼音字符被删除，会自动判断 count 是否超过个树。
     * \param count 退格次数。
     * \return 新的当前候选词的 const 引用。
     * \throws std::exception 如果发生错误。
     */
    const Candidates& backspace(size_t count = 1);

    /**
     * \brief 结束搜索，更新词库数据后重置搜索状态。
     * \param inc_freq 是否自动增加已选择项的频率。
     * \param add_new_sentence 是否根据已选择项自动添加新的词、句至词库。
     * \throws std::exception 如果发生错误。
     */
    void finish_search(bool inc_freq = true, bool add_new_sentence = true);

    /**
     * \brief 重置搜索状态，即清空 IME 的拼音、候选词、已选择项。
     */
    void reset_search() noexcept;

    /**
     * \brief 获取 IME 的已选择项。
     */
    const std::vector<Choice>& choices() const noexcept;

    /**
     * \brief 获取拼音切分的所有 Token。
     */
    PinYin::TokenSpan tokens() const noexcept;

    /**
     * \brief 获取拼音中已固定的 Tokens 组成的范围。
     * 
     */
    PinYin::TokenSpan fixed_tokens() const noexcept;

    /**
     * \brief 获取拼音中未固定的 Tokens 组成的范围。
     */
    PinYin::TokenSpan unfixed_tokens() const noexcept;

    /**
     * \brief 获取拼音字符串。
     */
    std::string_view pinyin() const noexcept;

    /**
     * \brief 获取拼音当前已固定的 TokenSpan 对应的拼音字符串。
     */
    std::string_view fixed_letters() const noexcept;

    /**
     * \brief 获取拼音当前未固定的 TokenSpan 对应的拼音字符串。
     */
    std::string_view unfixed_letters() const noexcept;

private:
    /**
     * \brief 真正的搜索实现函数。
     * \details 根据给定的 TokenSpan 构造 Query 对象进行搜索，并将结果保存。
     * \param tokens 要搜索的 TokenSpan。
     * \return 搜索后，新的当前候选词的 const 引用。
     */
    const Candidates& search_impl(PinYin::TokenSpan tokens);

    /**
     * \brief 将文本形式的 DictItem 转换为 DictItem 对象。
     * \param line 一行文本形式的 DictItem 字符串，格式应该为"中文 频率/优先级 拼音"。
     * \throws std::invalid_argument 如果 line 格式不符。
     */
    DictItem line_to_item(std::string_view line);
    PinYin m_pinyin;
    BasicTrie<Dict> m_dict_trie;
    Candidates m_candidates;
    std::vector<Choice> m_choices;
};

} // namespace pinyin_ime

#endif // PINYIN_IME_IME_H