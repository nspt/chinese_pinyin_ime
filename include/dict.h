#ifndef PINYIN_IME_DICT_H
#define PINYIN_IME_DICT_H

#include <vector>
#include <span>
#include <functional>
#include "pinyin.h"
#include "dict_item.h"

namespace pinyin_ime {

class Dict {
public:
    using ItemCRefVec = std::vector<std::reference_wrapper<const DictItem>>;
    static constexpr size_t s_npos{ std::numeric_limits<size_t>::max() };

    void add_item(DictItem item);
    template<class Pred>
    void erase_item(Pred pred)
    {
        std::erase_if(m_items, pred);
    }

    void sort();

    uint32_t suggest_inc_freq(DictItem &item) const;
    size_t item_index(const DictItem &item) const noexcept;
    uint32_t suggest_inc_freq(size_t idx) const noexcept;

    std::vector<DictItem>::const_iterator begin() const noexcept;
    std::vector<DictItem>::const_iterator end() const noexcept;

    const DictItem& operator[](size_t i) const;

    ItemCRefVec search(std::span<const PinYin::Token> tokens) const;
    ItemCRefVec search(std::string_view pinyin) const;
    ItemCRefVec search(const std::regex &pattern) const;
private:
    std::vector<DictItem> m_items;
};

} // namespace pinyin_ime

#endif // PINYIN_IME_DICT_H