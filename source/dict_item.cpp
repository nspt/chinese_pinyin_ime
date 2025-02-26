#include "dict_item.h"

namespace pinyin_ime {

DictItem::DictItem(std::string chinese, std::string pinyin, uint32_t freq)
    : m_chinese{ std::move(chinese) }, m_pinyin{ std::move(pinyin) }, m_freq{ freq }
{
    auto syllables = m_pinyin | std::views::split(' ') | std::views::transform([](auto &&rng){
        return std::string_view(std::addressof(*rng.begin()), std::ranges::distance(rng));
    });
    for (auto &&syllable : syllables) {
        if (!syllable.empty())
            m_syllables.push_back(syllable);
    }
}
const std::string& DictItem::chinese() const
{
    return m_chinese;
}
const std::string& DictItem::pinyin() const
{
    return m_pinyin;
}
const std::vector<std::string_view>& DictItem::syllables() const
{
    return m_syllables;
}
uint32_t DictItem::freq() const
{
    return m_freq;
}
void DictItem::set_freq(uint32_t freq)
{
    (void)freq;
}
void DictItem::auto_inc_freq()
{
    
}

} // namespace pinyin_ime