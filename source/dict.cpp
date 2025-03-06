#include "dict.h"

namespace pinyin_ime {

bool Dict::add_item(DictItem item)
{
    if (m_items.empty()) {
        m_items.emplace_back(std::move(item));
        m_acronym = item.acronym();
        return true;
    }
    if (item.acronym() != m_acronym)
        throw std::logic_error{ "Item acronym do not match" };
    auto begin_iter{ m_items.begin() };
    auto end_iter{ m_items.end() };
    auto insert_iter{ end_iter };
    for (auto cur_iter{ begin_iter }; cur_iter != end_iter; ++cur_iter) {
        if (insert_iter == end_iter && !item_comp(*cur_iter, item)) {
            insert_iter = cur_iter;
        }
        if (cur_iter->chinese() == item.chinese()
            && cur_iter->pinyin() == item.pinyin()) {
            return false;
        }
    }
    insert_iter = m_items.insert(insert_iter, std::move(item));
    return true;
}

std::string_view Dict::acronym() const noexcept
{
    return m_acronym;
}

void Dict::sort()
{
    std::sort(m_items.begin(), m_items.end(), item_comp);
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

const DictItem& Dict::operator[](size_t i) const noexcept
{
    return m_items[i];
}

const DictItem& Dict::at(size_t i) const
{
    return m_items.at(i);
}

Dict::ItemCRefVec Dict::search(PinYin::TokenSpan tokens) const
{
    enum class MatchResult {
        Fail, Partial, Full
    };
    using MR = MatchResult;
    using TT = PinYin::TokenType;

    Dict::ItemCRefVec result;
    Dict::ItemCRefVec ext_result;
    if (m_items.empty())
        return {};
    if (tokens.size() != m_items[0].syllables().size())
        return {};
    for (auto &item : m_items) {
        MR match{ MR::Full };
        auto &syllables{ item.syllables() };
        for (size_t i{ 0 }; match != MR::Fail && i < tokens.size(); ++i) {
            switch (tokens[i].m_type) {
            case TT::Initial:
            case TT::Extendible:
                if (!syllables[i].starts_with(tokens[i].m_token)) {
                    match = MR::Fail;
                    break;
                } else if (match == MR::Full
                           && syllables[i].size() != tokens[i].m_token.size()) {
                    match = MR::Partial;
                }
                break;
            default:
                if (syllables[i] != tokens[i].m_token)
                    match = MR::Fail;
                break;
            }
        }
        if (match == MR::Full) {
            result.emplace_back(item);
        } else if (match == MR::Partial) {
            ext_result.emplace_back(item);
        }
    }
    if (result.empty()) {
        return ext_result;
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
        if (std::regex_match(std::string{ item.pinyin() }, pattern)) {
            results.push_back(item);
        }
    }
    return results;
}

void Dict::auto_inc_freq(std::span<size_t> item_indexes)
{
    auto size{ m_items.size() };
    for (auto idx : item_indexes) {
        if (idx >= size)
            continue;
        m_items[idx].set_freq(m_items[idx].freq() + suggest_inc_freq(idx));
    }
    sort();
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

uint32_t Dict::suggest_inc_freq(size_t idx) const noexcept
{
    if (idx >= m_items.size())
        return 0;
    return 1; // TODO
}

bool Dict::item_comp(const DictItem &l, const DictItem &r) noexcept
{
    auto &l_syllables{ l.syllables() };
    auto &r_syllables{ r.syllables() };
    size_t l_s_size{ l_syllables.size() };
    size_t r_s_size{ r_syllables.size() };

    if (l_s_size < r_s_size)
        return true;
    if (l_s_size > r_s_size)
        return false;

    for (size_t i{ 0 }; i < l_s_size; ++i) {
        if (l_syllables[i] == r_syllables[i])
            continue;
        if (l_syllables[i].size() != r_syllables[i].size())
            return l_syllables[i].size() < r_syllables[i].size();
        return l_syllables[i] < r_syllables[i];
    }
    return l.freq() >= r.freq();
}

} // namespace pinyin_ime