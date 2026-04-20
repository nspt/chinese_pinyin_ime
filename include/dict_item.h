#ifndef PINYIN_IME_DICT_ITEM_H
#define PINYIN_IME_DICT_ITEM_H

#include <compare>
#include <string>
#include <vector>
#include <cstdint>

namespace pinyin_ime {

/**
 * \brief 词典项，用于存储一个字/词的中文、拼音和频率。
 * 
 * \details 除了存储字词的中文、拼音、频率外，DictItem 构造时会根据解析拼音，获取拼音中的
 *          音节（syllable，即单个字的拼音）列表，比如中文 "输入法" 的拼音是 "shu'ru'fa"，包含
 *          三个音节 "shu"、"ru"、"fa"。
 *          音节列表可以通过 syllables() 获取。
 *          通过 acronym() 可以获取音节首字母组成的缩略词，如 "shu'ru'fa" 的 acronym 为 "srf"。
 */
class DictItem {
public:
    DictItem(std::string chinese, std::string pinyin, uint32_t freq);
    DictItem(const DictItem &other);
    DictItem(DictItem &&other);
    DictItem& operator=(const DictItem &other);
    DictItem& operator=(DictItem &&other);

    std::string_view chinese() const noexcept;
    void set_chinese(std::string chinese) noexcept;

    std::string_view pinyin() const noexcept;
    void set_pinyin(std::string pinyin);

    uint32_t freq() const noexcept;
    void set_freq(uint32_t freq) noexcept;

    std::string acronym() const;
    const std::vector<std::string_view>& syllables() const noexcept;

    std::strong_ordering operator<=>(const DictItem &other) const noexcept;
private:
    void build_syllables_view();
    std::string m_chinese;
    std::string m_pinyin;
    uint32_t m_freq;
    std::vector<std::string_view> m_syllables;
};

} // namespace pinyin_ime

#endif // PINYIN_IME_DICT_ITEM_H