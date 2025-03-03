#ifndef PINYIN_IME_DICT_ITEM_H
#define PINYIN_IME_DICT_ITEM_H

#include <string>
#include <vector>
#include <ranges>

namespace pinyin_ime {

class Dict;
class DictItem {
public:
    DictItem(std::string chinese, std::string pinyin, uint32_t freq);
    DictItem(std::string_view chinese, std::string_view pinyin, uint32_t freq);
    DictItem(const DictItem &other);
    DictItem(DictItem &&other);
    DictItem& operator=(const DictItem &other);
    DictItem& operator=(DictItem &&other);

    const std::string& chinese() const noexcept;
    void set_chinese(std::string chinese);

    const std::string& pinyin() const noexcept;
    void set_pinyin(std::string pinyin);

    uint32_t freq() const noexcept;
    void set_freq(uint32_t freq);

    const std::vector<std::string_view>& syllables() const noexcept;
private:
    void build_syllables_view();
    std::string m_chinese;
    std::string m_pinyin;
    uint32_t m_freq;
    std::vector<std::string_view> m_syllables;
};

} // namespace pinyin_ime

#endif // PINYIN_IME_DICT_ITEM_H