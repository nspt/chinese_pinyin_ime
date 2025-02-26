#ifndef PINYIN_IME_DICT_ITEM
#define PINYIN_IME_DICT_ITEM

#include <string>
#include <vector>
#include <ranges>

namespace pinyin_ime {

class Dict;
class DictItem {
public:
    DictItem(std::string chinese, std::string pinyin, uint32_t freq);
    const std::string& chinese() const;
    const std::string& pinyin() const;
    const std::vector<std::string_view>& syllables() const;
    uint32_t freq() const;
    void set_freq(uint32_t freq);
    void auto_inc_freq();
private:
    std::string m_chinese;
    std::string m_pinyin;
    uint32_t m_freq;
    std::vector<std::string_view> m_syllables;
    Dict *m_dict{ nullptr };
    friend class Dict;
    friend class IME;
};

} // namespace pinyin_ime

#endif // PINYIN_IME_DICT_ITEM