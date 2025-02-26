#include "dict.h"

namespace pinyin_ime {

void Dict::add_item(DictItem item)
{
    if (item.m_dict)
        throw std::logic_error{ "Item already belongs to other Dict" };
    if (!m_items.empty()) {
        auto &ref_syllables{ m_items[0].syllables() };
        auto &new_syllables{ item.syllables() };
        if (ref_syllables.size() != new_syllables.size())
            throw std::logic_error{ "Item syllables count wrong" };
        for (size_t i{ 0 }; i < new_syllables.size(); ++i) {
            if (ref_syllables[i].front() != new_syllables[i].front())
                throw std::logic_error{ "Item syllables wrong" };
        }
    }
    auto iter{ m_items.begin() };
    auto end_iter{ m_items.end() };
    for (; iter != end_iter; ++iter) {
        if (iter->m_freq > item.m_freq)
            continue;
        break;
    }
    iter = m_items.insert(iter, std::move(item));
    iter->m_dict = this;
}
std::vector<DictItem>::const_iterator Dict::begin() const
{
    return m_items.begin();
}
std::vector<DictItem>::const_iterator Dict::end() const
{
    return m_items.end();
}
const DictItem& Dict::operator[](size_t i) const
{
    return m_items[i];
}
Dict::SearchResult Dict::search(const std::span<PinYin::Token> &tokens)
{
    using TT = PinYin::TokenType;
    Dict::SearchResult result;
    if (m_items.empty())
        return {};
    if (tokens.size() != m_items[0].syllables().size())
        return {};
    for (auto &item : m_items) {
        bool match{ true };
        auto &syllables{ item.syllables() };
        for (size_t i{ 0 }; match && i < tokens.size(); ++i) {
            switch (tokens[i].m_type) {
            case TT::Initial:
                if (!syllables[i].starts_with(tokens[i].m_token))
                    match = false;
            break;
            default:
                if (syllables[i] != tokens[i].m_token)
                    match = false;
            break;
            }
        }
        if (match) {
            result.push_back(item);
        }
    }
    return result;
}
Dict::SearchResult Dict::search(std::string_view pinyin)
{
    Dict::SearchResult results;
    for (auto& item : m_items) {
        if (item.m_pinyin == pinyin) {
            results.push_back(item);
        }
    }
    return results;
}
Dict::SearchResult Dict::search(const std::regex &pattern)
{
    Dict::SearchResult results;
    for (auto& item : m_items) {
        if (std::regex_match(item.m_pinyin, pattern)) {
            results.push_back(item);
        }
    }
    return results;
}

} // namespace pinyin_ime