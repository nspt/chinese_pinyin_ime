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
        Invalid, Initial, Syllable
    };
    struct Token {
        std::string_view m_token;
        TokenType m_type;
    };
    PinYin() = default;
    PinYin(std::string pinyin)
        : m_pinyin{ pinyin }
    {
        update_tokens();
    }
    std::string::const_iterator begin() const
    {
        return m_pinyin.cbegin();
    }
    std::string::const_iterator end() const
    {
        return m_pinyin.cend();
    }
    const char& operator[](std::string::size_type i) const
    {
        return m_pinyin[i];
    }
    PinYin& insert(std::string::size_type pos, std::string_view str)
    {
        m_pinyin.insert(pos, str);
        return *this;
    }
    std::string::const_iterator insert(const std::string::const_iterator &iter, std::string_view str)
    {
        return m_pinyin.insert(iter, str.begin(), str.end());
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
    const std::vector<Token>& push_back(char ch)
    {
        m_pinyin.push_back(ch);
        return update_tokens();
    }
    const std::vector<Token>& push_back(std::string_view str)
    {
        m_pinyin += str;
        return update_tokens();
    }
private:
    const std::vector<Token>& update_tokens()
    {
        using TokenList = std::vector<Token>;
        using MR = Trie::MatchResult;
        std::vector<TokenList> candidates;
        std::vector<TokenList> pending_tasks(1);

        while (!pending_tasks.empty()) {
            auto list = std::move(pending_tasks.back());
            pending_tasks.pop_back();
            auto begin_iter = m_pinyin.begin();
            auto end_iter = m_pinyin.end();
            auto start_iter = begin_iter;
            auto cur_iter = start_iter;
            if (!list.empty()) {
                auto &last_token = list.back();
                auto offset = last_token.m_token.data() - m_pinyin.data() + last_token.m_token.size();
                start_iter = begin_iter + offset;
                cur_iter = start_iter;
                begin_iter = start_iter;
            }
            for (auto prev_type = TokenType::Invalid; cur_iter != end_iter;) {
                std::string_view token{ start_iter, cur_iter + 1 };
                if (*cur_iter == s_delim) {
                    if (cur_iter != start_iter) {
                        list.emplace_back(std::string_view{ start_iter, cur_iter }, prev_type);
                    }
                    start_iter = cur_iter + 1;
                    cur_iter = start_iter;
                    continue;
                }
                switch (s_syllable_trie.match(token)) {
                case MR::Miss:
                    if (cur_iter != start_iter) {
                        list.emplace_back(std::string_view{ start_iter, cur_iter }, prev_type);
                        prev_type = TokenType::Invalid;
                        start_iter = cur_iter;
                    } else {
                        list.emplace_back(token, TokenType::Invalid);
                        prev_type = TokenType::Invalid;
                        start_iter = ++cur_iter;
                    }
                break;
                case MR::Partial:
                    prev_type = TokenType::Initial;
                    ++cur_iter;
                break;
                case MR::Extendible: {
                    auto next_iter = cur_iter + 1;
                    if (next_iter != end_iter) {
                        std::string_view next_token{ start_iter, next_iter + 1 };
                        if (s_syllable_trie.match(next_token) != MR::Miss) {
                            pending_tasks.push_back(list);
                            pending_tasks.back().emplace_back(token, TokenType::Syllable);
                            prev_type = TokenType::Syllable;
                            ++cur_iter;
                        } else { // next_token is MR::Miss
                            list.emplace_back(token, TokenType::Syllable);
                            prev_type = TokenType::Invalid;
                            start_iter = ++cur_iter;
                        }
                    } else { // next_iter == end_iter
                        list.emplace_back(token, TokenType::Syllable);
                        prev_type = TokenType::Invalid;
                        start_iter = ++cur_iter;
                    }
                }
                break;
                case MR::Complete:
                    list.emplace_back(token, TokenType::Syllable);
                    prev_type = TokenType::Invalid;
                    start_iter = ++cur_iter;
                break;
                }
            }
            candidates.emplace_back(std::move(list));
        }

        if (candidates.empty()) {
            m_tokens.clear();
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
        m_tokens = std::move(*(winner.first));
        return m_tokens;
    }
    std::vector<Token> m_tokens;
    std::string m_pinyin;
    static Trie s_syllable_trie;
    static constexpr char s_delim{ '\'' };
};

Trie PinYin::s_syllable_trie;

} // namespace pinyin_ime

#endif // PINYIN_IME_PINYIN