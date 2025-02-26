#ifndef PINYIN_IME_PINYIN
#define PINYIN_IME_PINYIN

#include <string>
#include <vector>
#include <regex>
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
        Token(TokenType type, std::string_view str)
            : m_type{ type }, m_token{ str }
        {}
        TokenType m_type{ TokenType::Invalid };
        std::string_view m_token;
    };
    PinYin();
    PinYin(std::string str);
    PinYin(const PinYin&) = delete;
    void reserve(size_t cap);
    std::string::const_iterator begin() const;
    std::string::const_iterator end() const;
    const char& operator[](size_t i) const;
    size_t fixed_tokens_count() const;
    void fix_front_tokens(size_t count);
    const Trie& syllableTrie() const;
    static void add_syllable(std::string_view syllable);
    static void remove_syllable(std::string_view syllable);
    const std::vector<Token>& tokens() const;
    const std::vector<Token>& backspace(size_t count = 1);
    const std::vector<Token>& insert(size_t pos, std::string_view str);
    const std::vector<Token>& insert(const std::string::const_iterator &iter, std::string_view str);
    const std::vector<Token>& push_back(char ch);
    const std::vector<Token>& push_back(std::string_view str);
private:
    const std::vector<Token>& update_tokens();

    std::vector<Token> m_tokens;
    std::string m_str;
    size_t m_fixed_tokens{ 0 },  m_fixed_letters{ 0 };

    static Trie s_syllable_trie;
    static constexpr char s_delim{ '\'' };
    static constexpr size_t s_capacity{ 128 };
};

} // namespace pinyin_ime

#endif // PINYIN_IME_PINYIN