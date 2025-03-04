#ifndef PINYIN_IME_PINYIN_H
#define PINYIN_IME_PINYIN_H

#include <string>
#include <vector>
#include <regex>
#include <span>
#include <limits>
#include "trie.h"

namespace pinyin_ime {

class PinYin {
public:
    enum class TokenType {
        Invalid, Initial, Extendible, Complete
    };

    struct Token {
        Token() = default;
        Token(TokenType type, std::string_view pinyin)
            : m_type{ type }, m_token{ pinyin }
        {}
        TokenType m_type{ TokenType::Invalid };
        std::string_view m_token;
    };

    using TokenSpan = std::span<const Token>;

    PinYin();
    PinYin(std::string str);
    PinYin(const PinYin&) = delete;
    PinYin(PinYin&&) = delete;
    PinYin& operator=(const PinYin&) = delete;
    PinYin& operator=(PinYin&&) = delete;

    void reserve(size_t cap);

    TokenSpan tokens() const noexcept;
    std::string_view pinyin() const noexcept;

    std::string::const_iterator begin() const noexcept;
    std::string::const_iterator end() const noexcept;

    const char& operator[](size_t i) const noexcept;

    size_t fix_count_for_tokens(TokenSpan tokens) const noexcept;
    bool fix_front_tokens(size_t count) noexcept;
    TokenSpan fixed_tokens() const noexcept;
    TokenSpan unfixed_tokens() const noexcept;

    std::string_view fixed_letters() const noexcept;
    std::string_view unfixed_letters() const noexcept;

    TokenSpan backspace(size_t count = 1);
    TokenSpan insert(size_t pos, std::string_view str);
    TokenSpan push_back(char ch);
    TokenSpan push_back(std::string_view str);
    void clear() noexcept;

    static const Trie& syllableTrie() noexcept;
    static void add_syllable(std::string_view syllable);
    static void remove_syllable(std::string_view syllable) noexcept;

    static constexpr char s_delim{ '\'' };
private:
    TokenSpan update_tokens();
    using TokenVec = std::vector<Token>;
    std::vector<TokenVec> token_split_candidates() const;

    TokenVec m_tokens;
    std::string m_pinyin;
    size_t m_fixed_tokens{ 0 },  m_fixed_letters{ 0 };

    static Trie s_syllable_trie;
    static constexpr size_t s_capacity{ 128 };
};

} // namespace pinyin_ime

#endif // PINYIN_IME_PINYIN_H