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

void PinYin::set_capacity(size_t cap)
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

size_t PinYin::fix_count_for_tokens(TokenSpan tokens) const noexcept
{
    if (tokens.empty())
        return 0;
    auto vec_start{ m_tokens.data() };
    auto vec_end{ m_tokens.data() + m_tokens.size() };
    auto span_start{ tokens.data() };
    auto span_end{ tokens.data() + tokens.size() };
    if (vec_start > span_start || span_end > vec_end)
        return 0;
    return span_end - vec_start;
}

bool PinYin::fix_front_tokens(size_t count) noexcept
{
    auto tokens_count = m_tokens.size();
    if (count > tokens_count)
        return false;
    m_fixed_tokens = count;
    if (m_fixed_tokens == 0) {
        m_fixed_letters = 0;
        return true;
    }
    if (m_fixed_tokens == tokens_count) {
        m_fixed_letters = m_pinyin.size();
        return true;
    }
    auto &token = m_tokens[m_fixed_tokens];
    m_fixed_letters = token.m_token.data() - m_pinyin.data();
    return true;
}

const Trie& PinYin::syllableTrie() noexcept
{
    return s_syllable_trie;
}

void PinYin::add_syllable(std::string_view syllable)
{
    s_syllable_trie.add_if_miss(syllable);
}

void PinYin::remove_syllable(std::string_view syllable) noexcept
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
    if (count == 0 || free_letters == 0) {
        return unfixed_tokens();
    }
    if (count > free_letters)
        count = free_letters;
    m_fixed_letters = m_pinyin.size() - count;
    m_pinyin.erase(m_fixed_letters);
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

std::vector<PinYin::TokenVec> PinYin::token_split_candidates() const
{
    using MR = Trie::MatchResult;
    std::vector<TokenVec> candidates;
    std::vector<TokenVec> pending_tasks;

    pending_tasks.emplace_back();
    while (!pending_tasks.empty()) {
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
                prev_type = TokenType::Invalid;
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
                if (cur_end_iter == end_iter) {
                    list.emplace_back(TokenType::Initial, token);
                }
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
    return candidates;
}

PinYin::TokenSpan PinYin::update_tokens()
{
    auto candidates{ token_split_candidates() };
    if (candidates.empty()) {
        m_tokens.erase(m_tokens.begin() + m_fixed_tokens, m_tokens.end());
        return m_tokens;
    }
    auto token_invalid_count = [](const TokenVec& tokens)->size_t{
        size_t count{ 0 };
        for (const auto &token : tokens) {
            if (token.m_type == TokenType::Invalid)
                ++count;
        }
        return count;
    };
    std::pair<decltype(candidates.begin()), size_t> winner{
        candidates.begin(), token_invalid_count(candidates.front())
    };
    auto end_iter{ candidates.end() };
    for (auto iter = winner.first + 1; iter != end_iter; ++iter) {
        size_t invalid_count{ token_invalid_count(*iter) };
        if (invalid_count != winner.second) {
            if (invalid_count < winner.second) {
                winner.first = iter;
                winner.second = invalid_count;
            }
            continue;
        }
        TokenVec &winner_vec = *(winner.first);
        TokenVec &cand_vec = *iter;
        size_t s = std::min(winner_vec.size(), cand_vec.size());
        for (size_t i = 0; i < s; ++i) {
            if (winner_vec[i].m_type != cand_vec[i].m_type) {
                if (winner_vec[i].m_type == TokenType::Invalid) {
                    winner.first = iter;
                    break;
                }
                if (cand_vec[i].m_type == TokenType::Invalid) {
                    break;
                }
            }
            if (winner_vec[i].m_token.size() != cand_vec[i].m_token.size()) {
                if (winner_vec[i].m_token.size() < cand_vec[i].m_token.size())
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