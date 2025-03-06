#ifndef PINYIN_IME_DICT_ITEM_H
#define PINYIN_IME_DICT_ITEM_H

#include <string>
#include <vector>

namespace pinyin_ime {

/**
 * \brief 词典项，用于存储一个字词的中文、拼音和频率（即优先级）。
 * 
 * \details 除了存储字词的中文、拼音、频率外，DictItem构造时还会解析拼音，获取拼音中的
 *          音节（syllable，即单个字的拼音）列表，比如中文“输入法”的拼音是“shu'ru'fa”，包含
 *          三个音节“shu”、“ru”、“fa”，可以通过 syllables() 获取音节列表，
 *          还可通过 acronym() 获取音节首字母组成的缩略词。
 */
class DictItem {
public:
    /**
     * \brief 构造函数。
     */
    DictItem(std::string chinese, std::string pinyin, uint32_t freq);
    /**
     * \brief 构造函数。
     */
    DictItem(std::string_view chinese, std::string_view pinyin, uint32_t freq);
    /**
     * \brief 拷贝构造函数。
     */
    DictItem(const DictItem &other);
    /**
     * \brief 移动构造函数。
     */
    DictItem(DictItem &&other);
    /**
     * \brief 拷贝赋值函数。
     */
    DictItem& operator=(const DictItem &other);
    /**
     * \brief 移动赋值函数。
     */
    DictItem& operator=(DictItem &&other);

    /**
     * \brief 获取中文。
     * 
     * \return 中文数据的 string_view，DictItem发生变化（set_chinese()、移动）后失效。
     */
    std::string_view chinese() const noexcept;

    /**
     * \brief 设置中文。
     */
    void set_chinese(std::string chinese) noexcept;

    /**
     * \brief 获取拼音。
     * 
     * \return 拼音数据的 string_view，DictItem 发生变化（set_pinyin()、移动）后失效。
     */
    std::string_view pinyin() const noexcept;

    /**
     * \brief 设置拼音。
     */
    void set_pinyin(std::string pinyin);

    /**
     * \brief 获取频率（优先级）。
     */
    uint32_t freq() const noexcept;

    /**
     * \brief 设置频率（优先级）。
     */
    void set_freq(uint32_t freq) noexcept;

    /**
     * \brief 获取拼音音节的首字母缩略词，每次调用时临时计算。
     */
    std::string acronym() const;

    /**
     * \brief 获取拼音的音节列表。
     * \return vector，包含逐个音节的 string_view，
     *         string_view 在 DictItem 发生变化（set_pinyin()、移动）后失效。
     */
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