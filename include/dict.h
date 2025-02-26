#ifndef PINYIN_IME_DICT
#define PINYIN_IME_DICT

#include <vector>
#include <span>
#include <functional>
#include "pinyin.h"
#include "dict_item.h"

namespace pinyin_ime {

class Dict {
public:
    using SearchResult = std::vector<std::reference_wrapper<DictItem>>;
    void add_item(DictItem item);
    std::vector<DictItem>::const_iterator begin() const;
    std::vector<DictItem>::const_iterator end() const;
    const DictItem& operator[](size_t i) const;
    SearchResult search(const std::span<PinYin::Token> &tokens);
    SearchResult search(std::string_view pinyin);
    SearchResult search(const std::regex &pattern);
private:
    std::vector<DictItem> m_items;
};

} // namespace pinyin_ime

#endif // PINYIN_IME_DICT