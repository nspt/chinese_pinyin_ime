#include "ime.h"
#include <fstream>

namespace pinyin_ime {

IME::IME(std::string_view dict_file)
{
    load(dict_file);
}

void IME::load(std::string_view dict_file)
{
    using std::operator""s;

    std::ifstream file{ dict_file.data(), std::ios::binary };
    if (!file)
        throw std::runtime_error{ "Open file failed: "s + std::strerror(errno) };
    // remove BOM
    uint8_t bom[3]{ 0 };
    file.read(reinterpret_cast<char*>(bom), 3);
    if (file.gcount() != 3)
        throw std::runtime_error{ "Read file failed: "s + std::strerror(errno) };
    if (bom[0] != 0xef || bom[1] != 0xbb || bom[2] != 0xbf)
        file.seekg(0);

    for (std::string line; std::getline(file, line);) {
        add_item_from_line(line);
    }
}

void IME::save(std::string_view dict_file) const
{
    using std::operator""s;

    std::ofstream file{ dict_file.data(), std::ios::binary | std::ios::trunc };
    if (!file)
        throw std::runtime_error{ "Open file failed: "s + std::strerror(errno) };

    auto end_iter{ m_dict_trie.end() };
    for (auto dict_it{ m_dict_trie.begin() }; dict_it != end_iter; ++dict_it) {
        Dict &dict{ *dict_it };
        for (auto item_it{ dict.begin() }; item_it != dict.end(); ++item_it) {
            const DictItem &item{ *item_it };
            file << item.chinese() << ' ';
            file << item.freq() << ' ';
            file << item.pinyin() << '\n';
        }
    }
}

IME::CandidatesCRef IME::candidates() const noexcept
{
    return m_candidates;
}

IME::CandidatesCRef IME::search(std::string_view pinyin)
{
    std::string_view cur_pinyin{ m_pinyin.pinyin() };
    if (!pinyin.starts_with(cur_pinyin)) {
        if (cur_pinyin.starts_with(pinyin)) {
            auto count = cur_pinyin.size() - pinyin.size();
            if (count <= m_pinyin.unfixed_letters().size())
                return backspace(count);
        }
        reset_search();
        return push_back(pinyin);
    } else if (pinyin.size() == cur_pinyin.size()) {
        return m_candidates;
    } else {
        return push_back(std::string_view{ pinyin.begin() + cur_pinyin.size(), pinyin.end() });
    }
}

IME::CandidatesCRef IME::search_impl(PinYin::TokenSpan tokens)
{
    std::stack<PinYin::TokenSpan> tokens_for_search;
    size_t tokens_size{ tokens.size() };
    for (size_t i{ 0 }; i < tokens_size; ++i) {
        PinYin::TokenSpan sub_tokens{ tokens.begin(), tokens.begin() + i + 1 };
        std::string acronym;
        for (auto &token : sub_tokens) {
            if (!token.m_token.empty())
                acronym.push_back(token.m_token.front());
        }
        if (m_dict_trie.contains(acronym))
            tokens_for_search.push(sub_tokens);
    }
    m_candidates.clear();
    while (!tokens_for_search.empty()) {
        Query q{ m_dict_trie, tokens_for_search.top() };
        if (q.size())
            m_candidates.push_back(std::move(q));
        tokens_for_search.pop();
    }
    return m_candidates;
}

IME::CandidatesCRef IME::push_back(std::string_view pinyin)
{
    return search_impl(m_pinyin.push_back(pinyin));
}

IME::CandidatesCRef IME::backspace(size_t count)
{
    // TODO: behaviour
    return search_impl(m_pinyin.backspace(count));
}

void IME::finish_search(bool inc_freq, bool add_new_sentence)
{
    size_t choices_count{ m_choices.size() };
    if (choices_count && inc_freq) {
        std::map<Dict*, std::vector<size_t>> map;
        for (auto &c : m_choices) {
            map[&c.m_dict].emplace_back(c.m_idx);
        }
        for (auto &p : map) {
            p.first->auto_inc_freq(p.second);
        }
    }
    if (choices_count && add_new_sentence) {
        std::string chinese;
        std::string pinyin;
        for (size_t i{ 0 }; i < choices_count; ++i) {
            auto &choice{ m_choices[i] };
            chinese += choice.m_dict[choice.m_idx].chinese();
            pinyin += choice.m_dict[choice.m_idx].pinyin();
            if (i != choices_count - 1)
                pinyin.push_back(PinYin::s_delim);
        }
        DictItem new_item{ std::move(chinese), std::move(pinyin), 1 };
        m_dict_trie.add_if_miss(new_item.acronym()).add_item(std::move(new_item));
    }
    reset_search();
}

void IME::reset_search() noexcept
{
    m_candidates.clear();
    m_choices.clear();
    m_pinyin.clear();
}

IME::ChoiceVecCRef IME::choices() const noexcept
{
    return m_choices;
}

PinYin::TokenSpan IME::tokens() const noexcept
{
    return m_pinyin.tokens();
}

PinYin::TokenSpan IME::fixed_tokens() const noexcept
{
    return m_pinyin.fixed_tokens();
}

PinYin::TokenSpan IME::unfixed_tokens() const noexcept
{
    return m_pinyin.unfixed_tokens();
}

std::string_view IME::pinyin() const noexcept
{
    return m_pinyin.pinyin();
}

std::string_view IME::fixed_letters() const noexcept
{
    return m_pinyin.fixed_letters();
}

std::string_view IME::unfixed_letters() const noexcept
{
    return m_pinyin.unfixed_letters();
}

IME::CandidatesCRef IME::choose(size_t idx)
{
    auto qi{ m_candidates.to_query_and_index(idx) };
    Query &query{ qi.first.get() };
    size_t q_idx{ qi.second };
    if (!query.dict()) // should not happen
        throw std::logic_error{ "Query has no dict" };
    Dict &dict{ *(query.dict()) };
    auto item{ query[q_idx] };
    size_t item_index{ dict.item_index(item) };
    if (item_index == Dict::s_npos) // should not happen
        throw std::logic_error{ "Get dict item index failed" };
    size_t fix_count{ m_pinyin.fix_count_for_tokens(query.tokens()) };
    if (fix_count == 0)
        throw std::logic_error{ "Tokens to fix is empty" };
    if (!m_pinyin.fix_front_tokens(fix_count))
        throw std::logic_error{ "Fix tokens failed" };
    m_choices.emplace_back(Choice{ query.tokens(), dict, item_index });
    return search_impl(m_pinyin.unfixed_tokens());
}

void IME::add_item_from_line(std::string_view line)
{
    reset_search();
    DictItem item{ line_to_item(line) };
    std::string acronym;
    for (auto &s : item.syllables()) {
        if (!s.empty()) {
            PinYin::add_syllable(s);
            acronym.push_back(s.front());
        }
    }
    m_dict_trie.add_if_miss(acronym).add_item(std::move(item));
}

DictItem IME::line_to_item(std::string_view line)
{
    std::string_view chinese, pinyin;
    uint32_t freq{ 0 };
    auto start{ std::string::npos };
    auto end{ std::string::npos };

    start = line.find_first_not_of(" \t\r");
    if (start == std::string::npos)
        throw std::runtime_error{ "Line format wrong" };
    end = line.find_first_of(" \t\r", start);
    if (end == std::string::npos)
        throw std::runtime_error{ "Line format wrong" };
    chinese = std::string_view{ line.begin() + start, line.begin() + end };

    start = line.find_first_not_of(" \t\r", end);
    if (start == std::string::npos)
        throw std::runtime_error{ "Line format wrong" };
    end = line.find_first_of(" \t\r", start);
    if (end == std::string::npos)
        throw std::runtime_error{ "Line format wrong" };
    freq = static_cast<uint32_t>(std::stoul(std::string{ line.begin() + start, line.begin() + end }));
    
    start = line.find_first_not_of(" \t\r", end);
    if (start == std::string::npos)
        throw std::runtime_error{ "Line format wrong" };
    end = line.find_first_of(" \t\r", start);
    if (end == std::string::npos)
        pinyin = std::string_view{ line.begin() + start, line.end() };
    else
        pinyin = std::string_view{ line.begin() + start, line.begin() + end };

    return DictItem{ chinese, pinyin, freq };
}

IME::Choice::Choice(PinYin::TokenSpan tokens, Dict &dict, size_t index) noexcept
    : m_tokens{ tokens }, m_dict{ dict }, m_idx{ index }
{}

PinYin::TokenSpan IME::Choice::tokens() const noexcept
{
    return m_tokens;
}

std::string_view IME::Choice::chinese() const noexcept
{
    return m_dict[m_idx].chinese();
}

} // namespace pinyin_ime