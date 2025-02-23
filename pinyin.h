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
        Token(size_t off, TokenType type, std::string_view str)
            : m_offset{ off }, m_type{ type }, m_token{ str }
        {}
        size_t m_offset{ 0 };
        TokenType m_type{ TokenType::Invalid };
        std::string m_token;
    };
    PinYin() = default;
    PinYin(std::string str)
        : m_str{ str }
    {
        update_tokens();
    }
    std::string::const_iterator begin() const
    {
        return m_str.cbegin();
    }
    std::string::const_iterator end() const
    {
        return m_str.cend();
    }
    const char& operator[](size_t i) const
    {
        return m_str[i];
    }
    size_t fixed_tokens_count() const
    {
        return m_fixed_tokens;
    }
    void fix_front_tokens(size_t count)
    {
        auto tokens_count = m_tokens.size();
        if (count > tokens_count)
            count = tokens_count;
        m_fixed_tokens = count;
        if (m_fixed_tokens == 0) {
            m_fixed_letters = 0;
            return;
        }
        if (m_fixed_tokens == tokens_count) {
            m_fixed_letters = m_str.size();
            return;
        }
        auto &token = m_tokens[m_fixed_tokens];
        m_fixed_letters = token.m_offset + token.m_token.size();
    }
    const Trie& syllableTrie() const
    {
        return s_syllable_trie;
    }
    static void add_syllable(std::string_view syllable)
    {
        s_syllable_trie.add_if_miss(syllable);
    }
    static void remove_syllable(std::string_view syllable)
    {
        s_syllable_trie.remove(syllable);
    }
    const std::vector<Token>& tokens() const
    {
        return m_tokens;
    }
    const std::vector<Token>& backspace(size_t count = 1)
    {
        size_t free_letters = m_str.size() - m_fixed_letters;
        if (count == 0 || free_letters == 0)
            return m_tokens;
        if (count > free_letters)
            count = free_letters;
        if (count == free_letters) {
            m_tokens.erase(m_tokens.begin() + m_fixed_tokens, m_tokens.end());
            m_fixed_letters = m_str.size();
            return m_tokens;
        }
        m_str.erase(m_str.size() - count);
        return update_tokens();
    }
    const std::vector<Token>& insert(size_t pos, std::string_view str)
    {
        if (pos < m_fixed_letters)
            throw std::logic_error{ "Can't insert before fixed position" };
        m_str.insert(pos, str);
        return update_tokens();
    }
    const std::vector<Token>& insert(const std::string::const_iterator &iter, std::string_view str)
    {
        auto offset = iter - begin();
        if (offset < m_fixed_letters)
            throw std::logic_error{ "Can't insert before fixed position" };
        m_str.insert(iter, str.begin(), str.end());
        return update_tokens();
    }
    const std::vector<Token>& push_back(char ch)
    {
        m_str.push_back(ch);
        return update_tokens();
    }
    const std::vector<Token>& push_back(std::string_view str)
    {
        m_str += str;
        return update_tokens();
    }
private:
    const std::vector<Token>& update_tokens()
    {
        using TokenList = std::vector<Token>;
        using MR = Trie::MatchResult;

        if (m_fixed_letters == m_str.size())
            throw std::logic_error{ "All pinyin is fixed, can't update tokens" };
        std::vector<TokenList> candidates;
        std::vector<TokenList> pending_tasks(1);
        std::string_view free_str{ m_str.begin() + m_fixed_letters, m_str.end() };

        while (!pending_tasks.empty()) {
            size_t begin_pos = m_fixed_letters;
            size_t end_pos = m_str.size();
            size_t start_pos = begin_pos;
            size_t cur_pos = start_pos;
            const char *p_str = m_str.data();
            auto list = std::move(pending_tasks.back());
            pending_tasks.pop_back();
            if (!list.empty()) {
                auto &last_token = list.back();
                auto offset = last_token.m_offset + last_token.m_token.size();
                cur_pos = start_pos = begin_pos = offset;
            }
            for (auto prev_type = TokenType::Invalid; cur_pos != end_pos;) {
                size_t cur_end_pos = cur_pos + 1;
                std::string_view token{ p_str + start_pos, cur_end_pos - start_pos };
                if (m_str[cur_pos] == s_delim) {
                    if (cur_pos != start_pos) {
                        list.emplace_back(start_pos, prev_type,
                            std::string_view{ p_str + start_pos, cur_pos - start_pos }
                        );
                    }
                    start_pos = cur_pos + 1;
                    cur_pos = start_pos;
                    continue;
                }
                switch (s_syllable_trie.match(token)) {
                case MR::Miss:
                    if (cur_pos != start_pos) {
                        list.emplace_back(start_pos, prev_type,
                            std::string_view{ p_str + start_pos, cur_pos - start_pos }
                        );
                        prev_type = TokenType::Invalid;
                        start_pos = cur_pos;
                    } else {
                        list.emplace_back(start_pos, TokenType::Invalid,
                            std::string_view{ p_str + start_pos, 1 }
                        );
                        prev_type = TokenType::Invalid;
                        start_pos = ++cur_pos;
                    }
                break;
                case MR::Partial:
                    prev_type = TokenType::Initial;
                    ++cur_pos;
                break;
                case MR::Extendible: {
                    if (cur_end_pos != end_pos) {
                        std::string_view next_token{ p_str + start_pos, cur_end_pos - start_pos + 1 };
                        if (s_syllable_trie.match(next_token) != MR::Miss) {
                            pending_tasks.push_back(list);
                            pending_tasks.back().emplace_back(start_pos, TokenType::Extendible, token);
                            prev_type = TokenType::Extendible;
                            ++cur_pos;
                        } else { // next_token is MR::Miss
                            list.emplace_back(start_pos, TokenType::Extendible, token);
                            prev_type = TokenType::Invalid;
                            start_pos = ++cur_pos;
                        }
                    } else { // cur_end_pos == end_pos
                        list.emplace_back(start_pos, TokenType::Extendible, token);
                        prev_type = TokenType::Invalid;
                        start_pos = ++cur_pos;
                    }
                }
                break;
                case MR::Complete:
                    list.emplace_back(start_pos, TokenType::Complete, token);
                    prev_type = TokenType::Invalid;
                    start_pos = ++cur_pos;
                break;
                }
            }
            candidates.emplace_back(std::move(list));
        }

        if (candidates.empty()) {
            m_tokens.erase(m_tokens.begin() + m_fixed_tokens, m_tokens.end());
            return m_tokens;
        }

        auto token_invalid_count = [](const TokenList& tokens)->int{
            int count{ 0 };
            for (const auto &token : tokens) {
                if (token.m_type == TokenType::Invalid)
                    ++count;
            }
            return count;
        };
        std::pair<decltype(candidates.begin()), int> winner{
            candidates.begin(), token_invalid_count(candidates.front())
        };
        auto end_iter = candidates.end();
        for (auto iter = winner.first + 1; iter != end_iter; ++iter) {
            int invalid_count = token_invalid_count(*iter);
            if (invalid_count != winner.second) {
                if (invalid_count < winner.second) {
                    winner.first = iter;
                    winner.second = invalid_count;
                }
                continue;
            }
            TokenList &winner_list = *(winner.first);
            TokenList &cand_list = *iter;
            size_t s = std::min(winner_list.size(), cand_list.size());
            for (size_t i = 0; i < s; ++i) {
                if (cand_list[i].m_type != TokenType::Invalid &&
                    winner_list[i].m_type == TokenType::Invalid) {
                    winner.first = iter;
                    break;
                }
                if (cand_list[i].m_token.size() > winner_list[i].m_token.size()) {
                    winner.first = iter;
                    break;
                }
            }
        }
        if (m_fixed_tokens == 0)
            m_tokens = std::move(*(winner.first));
        else {
            m_tokens.erase(m_tokens.begin() + m_fixed_tokens, m_tokens.end());
            m_tokens.insert(m_tokens.end(),
                std::make_move_iterator(winner.first->begin()),
                std::make_move_iterator(winner.first->end())
            );
        }
        return m_tokens;
    }
    std::vector<Token> m_tokens;
    std::string m_str;
    size_t m_fixed_tokens{ 0 },  m_fixed_letters{ 0 };
    static Trie s_syllable_trie;
    static constexpr char s_delim{ '\'' };
};

Trie PinYin::s_syllable_trie;

} // namespace pinyin_ime

#endif // PINYIN_IME_PINYIN