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
        auto start = line.find_first_not_of(" \t\r");
        auto end = line.find_last_not_of(" \t\r");
        if (start == std::string::npos || end == std::string::npos)
            continue;
        add_item_from_line(std::string_view{ line.data() + start, end - start + 1 });
    }
}

void IME::save(std::string_view dict_file)
{
    using std::operator""s;

    std::ofstream file{ dict_file.data(), std::ios::binary | std::ios::trunc };
    if (!file)
        throw std::runtime_error{ "Open file failed: "s + std::strerror(errno) };

    for (auto dict_it{ m_dict_trie.begin() }; dict_it != m_dict_trie.end(); ++dict_it) {
        Dict &dict{ *dict_it };
        for (auto item_it{ dict.begin() }; item_it != dict.end(); ++item_it) {
            const DictItem &item{ *item_it };
            file << item.chinese() << ' ';
            file << item.freq() << ' ';
            file << item.pinyin() << '\n';
        }
    }
}

IME::CandidatesCRef IME::search(std::string_view pinyin)
{
    std::string_view cur_pinyin{ m_pinyin.pinyin() };
    if (!pinyin.starts_with(cur_pinyin)) {
        if (cur_pinyin.starts_with(pinyin)) {
            return backspace(cur_pinyin.size() - pinyin.size());
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
        try {
            m_candidates.add_query(Query{ m_dict_trie, tokens_for_search.top() });
        } catch (const std::exception &e) {
            // should not happen
        }
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

void IME::reset_search() noexcept
{
    m_candidates.clear();
    m_choices.clear();
    m_pinyin.clear();
}

void IME::choose(size_t idx) noexcept
{

}

void IME::add_item_from_line(std::string_view line)
{
    reset_search();
    DictItem item{ line_to_item(line) };
    std::string acronym;
    for (auto &s : item.syllables()) {
        // DictItem's syllable will not be empty
        PinYin::add_syllable(s);
        acronym.push_back(s.front());
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

} // namespace pinyin_ime