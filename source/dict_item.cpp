#include "dict_item.h"
#include "pinyin.h"
#include <ranges>

namespace pinyin_ime {

DictItem::DictItem(std::string chinese, std::string pinyin, uint32_t freq)
    : m_chinese{ std::move(chinese) }, m_pinyin{ std::move(pinyin) }, m_freq{ freq }
{
    build_syllables_view();
}

DictItem::DictItem(const DictItem &other)
    : m_chinese{ other.m_chinese }, m_pinyin{ other.m_pinyin }, m_freq{ other.m_freq }
{
    m_syllables.reserve(other.m_syllables.size());
    auto p = other.m_pinyin.data();
    for (auto &s : other.m_syllables) {
        auto offset{ s.data() - p };
        m_syllables.push_back(std::string_view{ m_pinyin.data() + offset, s.size() });
    }
}

DictItem::DictItem(DictItem &&other)
    : m_chinese{ std::move(other.m_chinese) },
      m_pinyin{ std::move(other.m_pinyin) }, m_freq{ other.m_freq }
{
    other.m_syllables.clear();
    build_syllables_view();
}

DictItem& DictItem::operator=(const DictItem &other)
{
    m_chinese = other.m_chinese;
    m_pinyin = other.m_pinyin;
    m_freq = other.m_freq;
    for (auto &s : other.m_syllables) {
        auto offset{ s.data() - other.m_pinyin.data() };
        m_syllables.push_back(std::string_view{ m_pinyin.data() + offset, s.size() });
    }
    return *this;
}

DictItem& DictItem::operator=(DictItem &&other)
{
    m_chinese = std::move(other.m_chinese);
    m_pinyin = std::move(other.m_pinyin);
    m_freq = other.m_freq;
    other.m_syllables.clear();
    build_syllables_view();
    return *this;
}

std::string_view DictItem::chinese() const noexcept
{
    return m_chinese;
}

void DictItem::set_chinese(std::string chinese) noexcept
{
    m_chinese = std::move(chinese);
}

std::string_view DictItem::pinyin() const noexcept
{
    return m_pinyin;
}

void DictItem::set_pinyin(std::string pinyin)
{
    m_pinyin = std::move(pinyin);
    build_syllables_view();
}

std::string DictItem::acronym() const
{
    std::string str;
    str.reserve(m_syllables.size());
    for (auto &s : m_syllables) {
        str.push_back(s.front());
    }
    return str;
}

const std::vector<std::string_view>& DictItem::syllables() const noexcept
{
    return m_syllables;
}

uint32_t DictItem::freq() const noexcept
{
    return m_freq;
}

void DictItem::set_freq(uint32_t freq) noexcept
{
    m_freq = freq;
}

void DictItem::build_syllables_view()
{
    if (!m_syllables.empty())
        m_syllables.clear();
    auto syllables = m_pinyin
        | std::views::split(PinYin::s_delim)
        | std::views::transform([](auto &&rng) -> std::string_view {
            return std::string_view(std::addressof(*rng.begin()), std::ranges::distance(rng));
        });
    for (auto &&s : syllables) {
        if (!s.empty())
            m_syllables.push_back(std::move(s));
    }
}

std::strong_ordering DictItem::operator<=>(const DictItem &other) const noexcept
{
    // 1. 比较音节首字母缩略词，若不同（不属于同一个 Dict），缩略词排前者优先级高
    {
        auto this_acronym = acronym();
        auto other_acronym = other.acronym();
        if (this_acronym != other_acronym)
        {
            return this_acronym <=> other_acronym;
        }
    }

    // 2. 比较频率，若不同，频率高者优先级高
    if (m_freq != other.m_freq)
    {
        if (m_freq > other.m_freq)
            return std::strong_ordering::less;
        if (m_freq < other.m_freq)
            return std::strong_ordering::greater;
    }

    // 3. 逐音节比较字典序
    for (size_t i{ 0 }; i < m_syllables.size(); ++i) {
        auto r = m_syllables[i] <=> other.m_syllables[i];
        if (r != std::strong_ordering::equal)
            return r;
    }

    // 4. 按“字典序”比较 pinyin 和 chinese
    if (auto r = m_pinyin <=> other.m_pinyin; r != std::strong_ordering::equal)
        return r;
    return m_chinese <=> other.m_chinese;
}

} // namespace pinyin_ime