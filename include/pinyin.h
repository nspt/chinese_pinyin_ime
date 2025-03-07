#ifndef PINYIN_IME_PINYIN_H
#define PINYIN_IME_PINYIN_H

#include <string>
#include <vector>
#include <regex>
#include <span>
#include <limits>
#include "trie.h"

namespace pinyin_ime {

/**
 * \brief 拼音类，实现存储和解析拼音字符串功能。
 * \details PinYin 本质上是对一个 std::string（拼音字符串）实现封装和抽象，
 *              对其进行解析后向外部提供视图进行访问，并且提供与输入法使用相关的 Token 固定功能。
 *          PinYin 依靠存储着有效音节（syllable）的字典树进行解析功能，初始状态下该
 *              字典树为空，需要外部通过 add_syllable() 添加有效音节。
 *          Token 是 PinYin 对拼音字符串解析、分割后得到的单元，它可能是音节（细分为可扩展和
 *              不可继续扩展）、音节的起始部分或非音节字符串，比如拼音字符串 "srufai"，
 *              会分割为四个 Token：s、ru、fa、i，分别是音节起始、可扩展音节、可扩展音节、非音节。
 *          Token 的分割采用贪心策略，PinYin 会尽可能分割更长的音节作为 Token，比如 "zhuan" 会分割为
 *              "zhuan" 而不是 "zhu" 和 "an"。
 *              此行为可以被字符串中的分割符（'\''）影响，比如"z'huan" 会因为分割符的存在而分割
 *              为 "z" 和 "huan"。
 *              固定 Token 也会对分割产生影响，已经被固定的 Token 所对应的拼音字符串将不再参与分割。
 */
class PinYin {
public:
    /**
     * \brief Token 类型。
     * \details Invalid:非音节字符串。
     *          Initial:某个有效音节的起始部分。
     *          Extendible:有效音节，同时是另一个有效音节的起始部分。
     *          Complete:有效音节，且不能继续扩展为另一个有效音节。
     */
    enum class TokenType {
        Invalid, Initial, Extendible, Complete
    };

    /**
     * \brief 拼音分割后的单元，是对 PinYin 内部 std::string 的视图，
     *        如果不在 fixed_tokens() 范围内，则 PinYin 被修改后失效。
     */
    struct Token {
        Token() = default;
        Token(TokenType type, std::string_view pinyin)
            : m_type{ type }, m_token{ pinyin }
        {}
        TokenType m_type{ TokenType::Invalid };
        std::string_view m_token;
    };
    /**
     * \brief 连续多个 Token 组成的范围。
     */
    using TokenSpan = std::span<const Token>;

    /**
     * \brief 构造函数。
     * \throws std::exception 如果发生错误。
     */
    PinYin();

    /**
     * \brief 构造函数。
     * \param str 初始化拼音字符串。
     * \throws std::exception 如果发生错误。
     */
    PinYin(std::string str);

    /**
     * \brief 禁止拷贝构造。
     */
    PinYin(const PinYin&) = delete;
    /**
     * \brief 禁止移动构造。
     */
    PinYin(PinYin&&) = delete;
    /**
     * \brief 禁止拷贝赋值。
     */
    PinYin& operator=(const PinYin&) = delete;
    /**
     * \brief 禁止移动赋值。
     */
    PinYin& operator=(PinYin&&) = delete;

    /**
     * \brief 设置允许的最大拼音字符串长度。
     * \note 内部 std::string 通过 reserve 预留空间，从而实现在达到最大容量前，
     *       外部视图访问的有效性，默认情况下最大容量为 PinYin::s_capacity
     * \throws std::exception 如果发生错误。
     */
    void set_capacity(size_t cap);

    /**
     * \brief 获取当前分割得到的所有 Tokens，PinYin 被修改后失效。
     */
    TokenSpan tokens() const noexcept;

    /**
     * \brief 获取内部拼音字符串视图，PinYin 被修改后失效。
     */
    std::string_view pinyin() const noexcept;

    /**
     * \brief 获取内部拼音字符串的起始迭代器，PinYin 被修改后失效。
     */
    std::string::const_iterator begin() const noexcept;

    /**
     * \brief 获取内部拼音字符串的结束迭代器，PinYin 被修改后失效。
     */
    std::string::const_iterator end() const noexcept;

    /**
     * \brief 获取内部拼音字符串的给定索引字符，PinYin 被修改后失效。
     */
    const char& operator[](size_t i) const noexcept;

    /**
     * \brief 固定前 N 个 Token，被固定的 Token 将不参与后续解析分割。
     * \param count 需要固定的 Token 数量
     * \return count 不超过当前 Token 总数量返回 true，否则返回 false。
     */
    bool fix_front_tokens(size_t count) noexcept;

    /**
     * \brief 辅助函数，计算要将给定 TokenSpan 固定，需要固定多少个 Token，
     *        搭配 fix_front_tokens() 使用。
     * \details 本质上是计算给定 TokenSpan 的最后一个 Token 属于第几个 Token。
     * \param tokens 要计算的子 TokenSpan
     * \return 需要固定的 Token 个数，若 tokens 不属于此 PinYin，返回 0。
     */
    size_t fix_count_for_tokens(TokenSpan tokens) const noexcept;

    /**
     * \brief 获取当前已固定的 Tokens 组成的范围。
     */
    TokenSpan fixed_tokens() const noexcept;

    /**
     * \brief 获取当前未固定的 Tokens 组成的范围。
     */
    TokenSpan unfixed_tokens() const noexcept;

    /**
     * \brief 获取当前已固定的 TokenSpan 对应的内部拼音字符串的子范围视图。
     */
    std::string_view fixed_letters() const noexcept;

    /**
     * \brief 获取当前未固定的 TokenSpan 对应的内部拼音字符串的子范围视图。
     */
    std::string_view unfixed_letters() const noexcept;

    /**
     * \brief 退格，即删除拼音字符串的尾部字符，仅允许删除未固定的部分。
     * \param count 退格次数，即删除个数。
     * \return 退格操作后，未固定的 TokenSpan，即 unfixed_tokens()。
     * \note 若无未固定字符可删除，则无效果。此函数调用后，之前获取到的视图，
     *       除了已固定部分的视图，其它视图不保证有效。
     * \throws std::exception 如果发生错误。
     */
    TokenSpan backspace(size_t count = 1);

    /**
     * \brief 在拼音字符串给定位值插入新拼音字符串，仅允许操作未固定的部分。
     * \param pos 插入位置。
     * \param str 待插入字符串。
     * \return 插入操作后，未固定的 TokenSpan，即 unfixed_tokens()。
     * \note 若插入会导致超出最大容量，则无效果。此函数调用后，之前获取到的视图，
     *       除了已固定部分的视图，其它视图不保证有效。
     * \throws std::logic_error 如果 pos 位于已固定部分。
     *         std::exception 如果发生错误。
     */
    TokenSpan insert(size_t pos, std::string_view str);

    /**
     * \brief 在拼音字符串尾部添加新拼音字符。
     * \param ch 待添加字符。
     * \return 添加操作后，未固定的 TokenSpan，即 unfixed_tokens()。
     * \note 若添加会导致超出最大容量，则无效果。此函数调用后，之前获取到的视图，
     *       除了已固定部分的视图，其它视图不保证有效。
     * \throws std::exception 如果发生错误。
     */
    TokenSpan push_back(char ch);

    /**
     * \brief 在拼音字符串尾部添加新拼音字符串。
     * \param ch 待添加字符串。
     * \return 添加操作后，未固定的 TokenSpan，即 unfixed_tokens()。
     * \note 若添加会导致超出最大容量，则无效果。此函数调用后，之前获取到的视图，
     *       除了已固定部分的视图，其它视图不保证有效。
     * \throws std::exception 如果发生错误。
     */
    TokenSpan push_back(std::string_view str);

    /**
     * \brief 清空拼音字符串，此函数调用后，所有之前获取的视图均无效。
     */
    void clear() noexcept;

    /**
     * \brief 获取 PinYin 类使用的音节字典树。
     */
    static const Trie& syllableTrie() noexcept;

    /**
     * \brief 向 PinYin 类使用的音节字典树添加新音节，内部调用 BasicTrie<>::add_if_miss。
     * \throws std::exception 如果发生错误。
     */
    static void add_syllable(std::string_view syllable);

    /**
     * \brief 从 PinYin 类使用的音节字典树删除音节。
     */
    static void remove_syllable(std::string_view syllable) noexcept;

    /**
     * \brief 拼音字符串中使用的分割符号
     */
    static constexpr char s_delim{ '\'' };
private:
    /**
     * \brief 根据当前状态（拼音字符串、已固定范围）重新分割 Token。
     * \return 重新分割后未固定的 TokenSpan，即 unfixed_tokens()。
     */
    TokenSpan update_tokens();

    using TokenVec = std::vector<Token>;
    /**
     * \brief 辅助函数，获取当前状态所有合理的分割可能，供 update_tokens() 调用。
     * \return 所有可能的分割方法（TokenVec 表示一个分割方案）列表。
     */
    std::vector<TokenVec> token_split_candidates() const;

    TokenVec m_tokens;
    std::string m_pinyin;
    size_t m_fixed_tokens{ 0 },  m_fixed_letters{ 0 };

    static Trie s_syllable_trie;
    static constexpr size_t s_capacity{ 128 };
};

} // namespace pinyin_ime

#endif // PINYIN_IME_PINYIN_H