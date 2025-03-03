#include "dict.h"

namespace pinyin_ime {

void Dict::add_item(DictItem item)
{
    if (!m_items.empty()) {
        auto ref_syllables{ m_items[0].syllables() };
        auto new_syllables{ item.syllables() };
        if (ref_syllables.size() != new_syllables.size()) {
            throw std::logic_error{ "Item syllables count wrong" };
        }
        for (size_t i{ 0 }; i < new_syllables.size(); ++i) {
            if (ref_syllables[i].front() != new_syllables[i].front()) {
                throw std::logic_error{ "Item syllables wrong" };
            }
        }
    }
    auto iter{ m_items.begin() };
    auto end_iter{ m_items.end() };
    for (; iter != end_iter; ++iter) {
        if (iter->freq() > item.freq())
            continue;
        break;
    }
    iter = m_items.insert(iter, std::move(item));
}

void Dict::sort()
{
    std::sort(m_items.begin(), m_items.end(), [](const DictItem &l, const DictItem &r){
        return l.freq() > r.freq();
    });
}

std::vector<DictItem>::const_iterator Dict::begin() const noexcept
{
    return m_items.begin();
}

std::vector<DictItem>::const_iterator Dict::end() const noexcept
{
    return m_items.end();
}

size_t Dict::size() const noexcept
{
    return m_items.size();
}

const DictItem& Dict::operator[](size_t i) const
{
    return m_items[i];
}

Dict::ItemCRefVec Dict::search(std::span<const PinYin::Token> tokens) const
{
    using TT = PinYin::TokenType;
    Dict::ItemCRefVec result;
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
            case TT::Extendible:
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

Dict::ItemCRefVec Dict::search(std::string_view pinyin) const
{
    Dict::ItemCRefVec results;
    for (auto& item : m_items) {
        if (item.pinyin() == pinyin) {
            results.push_back(item);
        }
    }
    return results;
}

Dict::ItemCRefVec Dict::search(const std::regex &pattern) const
{
    Dict::ItemCRefVec results;
    for (auto& item : m_items) {
        if (std::regex_match(item.pinyin(), pattern)) {
            results.push_back(item);
        }
    }
    return results;
}

size_t Dict::item_index(const DictItem &item) const noexcept
{
    auto p{ &item };
    auto data{ m_items.data() };
    if (p < data || p >= data + m_items.size()) {
        return s_npos;
    }
    return static_cast<size_t>(p - data);
}

uint32_t Dict::suggest_inc_freq(DictItem &item) const
{
    auto idx{ item_index(item) };
    if (idx == s_npos)
        throw std::logic_error{ "Item doesn't belong to this dict" };
    return suggest_inc_freq(idx);
}

uint32_t Dict::suggest_inc_freq(size_t idx) const noexcept
{
    if (idx >= m_items.size())
        return 0;
    return 1; // TODO
}

} // namespace pinyin_ime