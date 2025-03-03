#include "pinyin.h"

namespace pinyin_ime {

Trie PinYin::s_syllable_trie;

PinYin::PinYin()
{
    m_pinyin.reserve(s_capacity);
    m_tokens.reserve(s_capacity);
}

PinYin::PinYin(std::string str)
    : m_pinyin{ std::move(str) }
{
    m_pinyin.reserve(s_capacity);
    m_tokens.reserve(s_capacity);
    update_tokens();
}

void PinYin::reserve(size_t cap)
{
    std::vector<std::tuple<size_t, size_t, TokenType>> tokens;
    for (auto &token : m_tokens) {
        auto offset{ token.m_token.data() - m_pinyin.data() };
        auto size{ token.m_token.size() };
        tokens.emplace_back(offset, size, token.m_type);
    }
    m_tokens.clear();
    m_pinyin.reserve(cap);
    m_tokens.reserve(cap);
    for (auto &token : tokens) {
        auto offset{ std::get<0>(token) };
        auto size{ std::get<1>(token) };
        m_tokens.emplace_back(
            std::get<2>(token),
            std::string_view{ m_pinyin.data() + offset, size }
        );
    }
}

std::string_view PinYin::pinyin() const noexcept
{
    return m_pinyin;
}

std::string::const_iterator PinYin::begin() const noexcept
{
    return m_pinyin.begin();
}

std::string::const_iterator PinYin::end() const noexcept
{
    return m_pinyin.end();
}

const char& PinYin::operator[](size_t i) const noexcept
{
    return m_pinyin[i];
}

PinYin::TokenSpan PinYin::fixed_tokens() const noexcept
{
    if (m_fixed_tokens == 0)
        return {};
    return { m_tokens.begin(), m_fixed_tokens };
}

PinYin::TokenSpan PinYin::unfixed_tokens() const noexcept
{
    if (m_fixed_tokens < m_tokens.size())
        return { m_tokens.begin() + m_fixed_tokens, m_tokens.end() };
    return {};
}

std::string_view PinYin::fixed_letters() const noexcept
{
    if (m_fixed_letters == 0)
        return {};
    return { m_pinyin.begin(), m_pinyin.begin() + m_fixed_letters };
}

std::string_view PinYin::unfixed_letters() const noexcept
{
    if (m_fixed_letters < m_pinyin.size())
        return { m_pinyin.begin() + m_fixed_letters, m_pinyin.end() };
    return {};
}

void PinYin::fix_front_tokens(size_t count)
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
        m_fixed_letters = m_pinyin.size();
        return;
    }
    auto &token = m_tokens[m_fixed_tokens];
    m_fixed_letters = token.m_token.data() - m_pinyin.data();
}

const Trie& PinYin::syllableTrie() noexcept
{
    return s_syllable_trie;
}

void PinYin::add_syllable(std::string_view syllable)
{
    s_syllable_trie.add_if_miss(syllable);
}

void PinYin::remove_syllable(std::string_view syllable)
{
    s_syllable_trie.remove(syllable);
}

PinYin::TokenSpan PinYin::tokens() const noexcept
{
    return m_tokens;
}

PinYin::TokenSpan PinYin::backspace(size_t count)
{
    size_t free_letters = m_pinyin.size() - m_fixed_letters;
    if (count == 0 || free_letters == 0)
        return unfixed_tokens();
    if (count > free_letters)
        count = free_letters;
    if (count == free_letters) {
        m_tokens.erase(m_tokens.begin() + m_fixed_tokens, m_tokens.end());
        m_fixed_letters = m_pinyin.size();
        return unfixed_tokens();
    }
    m_pinyin.erase(m_pinyin.size() - count);
    return update_tokens();
}

PinYin::TokenSpan PinYin::insert(size_t pos, std::string_view str)
{
    if (pos < m_fixed_letters)
        throw std::logic_error{ "Can't insert before fixed position" };
    if (m_pinyin.size() + str.size() >= m_pinyin.capacity())
        return unfixed_tokens(); // no effect
    m_pinyin.insert(pos, str);
    return update_tokens();
}

PinYin::TokenSpan PinYin::push_back(char ch)
{
    if (m_pinyin.size() + 1 >= m_pinyin.capacity())
        return unfixed_tokens(); // no effect
    m_pinyin.push_back(ch);
    return update_tokens();
}

PinYin::TokenSpan PinYin::push_back(std::string_view str)
{
    if (m_pinyin.size() + str.size() >= m_pinyin.capacity())
        return unfixed_tokens(); // no effect
    m_pinyin += str;
    return update_tokens();
}

void PinYin::clear() noexcept
{
    m_fixed_letters = m_fixed_tokens = 0;
    m_tokens.clear();
    m_pinyin.clear();
}

PinYin::TokenSpan PinYin::update_tokens()
{
    using TokenList = std::vector<Token>;
    using MR = Trie::MatchResult;

    if (m_fixed_letters == m_pinyin.size())
        return m_tokens;

    std::vector<TokenList> candidates;
    std::vector<TokenList> pending_tasks(1);

    while (pending_tasks.size()) {
        auto begin_iter{ m_pinyin.begin() + m_fixed_letters };
        auto end_iter{ m_pinyin.end() };
        auto start_iter{ begin_iter };
        auto cur_iter{ start_iter };

        auto list = std::move(pending_tasks.back());
        pending_tasks.pop_back();
        if (!list.empty()) {
            auto &last_token = list.back();
            auto offset = last_token.m_token.data() - m_pinyin.data() + last_token.m_token.size();
            cur_iter = start_iter = begin_iter = begin_iter + offset;
        }
        for (auto prev_type = TokenType::Invalid; cur_iter != end_iter;) {
            auto cur_end_iter = cur_iter + 1;
            std::string_view token{ start_iter, cur_end_iter };
            if (*cur_iter == s_delim) {
                if (cur_iter != start_iter) {
                    list.emplace_back(prev_type, std::string_view{ start_iter, cur_iter });
                }
                cur_iter = start_iter = cur_end_iter;
                continue;
            }
            switch (s_syllable_trie.match(token)) {
            case MR::Miss:
                if (cur_iter != start_iter) {
                    list.emplace_back(prev_type, std::string_view{ start_iter, cur_iter });
                    prev_type = TokenType::Invalid;
                    start_iter = cur_iter;
                } else {
                    list.emplace_back(TokenType::Invalid, token);
                    prev_type = TokenType::Invalid;
                    start_iter = ++cur_iter;
                }
            break;
            case MR::Partial:
                prev_type = TokenType::Initial;
                ++cur_iter;
            break;
            case MR::Extendible: {
                if (cur_end_iter != end_iter) {
                    std::string_view next_token{ start_iter, cur_end_iter + 1 };
                    if (s_syllable_trie.match(next_token) != MR::Miss) {
                        pending_tasks.push_back(list);
                        pending_tasks.back().emplace_back(TokenType::Extendible, token);
                        prev_type = TokenType::Extendible;
                        ++cur_iter;
                    } else { // next_token is MR::Miss
                        list.emplace_back(TokenType::Extendible, token);
                        prev_type = TokenType::Invalid;
                        start_iter = ++cur_iter;
                    }
                } else { // cur_end_pos == end_pos
                    list.emplace_back(TokenType::Extendible, token);
                    prev_type = TokenType::Invalid;
                    start_iter = ++cur_iter;
                }
            }
            break;
            case MR::Complete:
                list.emplace_back(TokenType::Complete, token);
                prev_type = TokenType::Invalid;
                start_iter = ++cur_iter;
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
    m_tokens.erase(m_tokens.begin() + m_fixed_tokens, m_tokens.end());
    m_tokens.insert(m_tokens.end(),
        std::make_move_iterator(winner.first->begin()),
        std::make_move_iterator(winner.first->end())
    );
    return unfixed_tokens();
}

} // namespace pinyin_ime