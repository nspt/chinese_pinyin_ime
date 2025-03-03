#include "dict_item.h"
#include "dict.h"
#include <iostream>

namespace pinyin_ime {

DictItem::DictItem(std::string chinese, std::string pinyin, uint32_t freq)
    : m_chinese{ std::move(chinese) }, m_pinyin{ std::move(pinyin) }, m_freq{ freq }
{
    build_syllables_view();
}

DictItem::DictItem(std::string_view chinese, std::string_view pinyin, uint32_t freq)
    : m_chinese{ chinese }, m_pinyin{ pinyin }, m_freq{ freq }
{
    build_syllables_view();
}

DictItem::DictItem(const DictItem &other)
    : m_chinese{ other.m_chinese }, m_pinyin{ other.m_pinyin }, m_freq{ other.m_freq }
{
    for (auto &s : other.m_syllables) {
        auto offset{ s.data() - other.m_pinyin.data() };
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

const std::string& DictItem::chinese() const noexcept
{
    return m_chinese;
}

void DictItem::set_chinese(std::string chinese)
{
    m_chinese = std::move(chinese);
}

const std::string& DictItem::pinyin() const noexcept
{
    return m_pinyin;
}

void DictItem::set_pinyin(std::string pinyin)
{
    m_pinyin = std::move(pinyin);
    build_syllables_view();
}

const std::vector<std::string_view>& DictItem::syllables() const noexcept
{
    return m_syllables;
}

uint32_t DictItem::freq() const noexcept
{
    return m_freq;
}

void DictItem::set_freq(uint32_t freq)
{
    m_freq = freq;
}

void DictItem::build_syllables_view()
{
    if (!m_syllables.empty())
        m_syllables.clear();

    auto syllables = m_pinyin | std::views::split(PinYin::s_delim) | std::views::transform([](auto &&rng){
        return std::string_view(std::addressof(*rng.begin()), std::ranges::distance(rng));
    });
    for (auto &&s : syllables) {
        if (!s.empty())
            m_syllables.push_back(s);
    }
}

} // namespace pinyin_ime