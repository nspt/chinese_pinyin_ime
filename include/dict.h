#ifndef PINYIN_IME_DICT_H
#define PINYIN_IME_DICT_H

#include <vector>
#include <span>
#include <functional>
#include "pinyin.h"
#include "dict_item.h"

namespace pinyin_ime {

/**
 * \brief 词典，存储音节首字母缩略词（syllables acronym）相同的 DictItem。
 * 
 * \details Dict 本质上是对 vector<DictItem> 的封装，实现词典层面的 Invariant：
 *              1. vector 中的 DictItem 有相同的 acronym。
 *              2. vector 中的 DictItem 是已排序的。
 *          同时提供词典层面的查找功能，以及对 DictItem 的频率修改功能。
 */
class Dict {
public:
    using ItemCRefVec = std::vector<std::reference_wrapper<const DictItem>>;
    static constexpr size_t s_npos{ std::numeric_limits<size_t>::max() };

    /**
     * \brief 添加一个 DictItem，要求 DictItem 的 acronym 与词典一致，除非词典为空。
     * \param item 需要加入到 Dict 的 DictItem。
     * \return 成功添加为 true，否则为 false。
     * \throws std::logic_error 若 DictItem 的 acronym 与词典不同。
     *         std::exception 如果发生错误。
     */
    bool add_item(DictItem item);

    /**
     * \brief 移除一个满足参数 Pred 的 DictItem。
     * \throws std::exception 如果发生错误。
     */
    template<class Pred>
    void erase_item(Pred pred)
    {
        std::erase_if(m_items, pred);
    }

    /**
     * \brief 获取拼音音节的首字母缩略词。
     * \return 指向内部 acronym 的 string_view，在 add_item() 后可能失效。
     */
    std::string_view acronym() const noexcept;

    /**
     * \brief 根据内置策略，自动增加给定索引对应的 DictItem 的 freq。
     * \param item_indexes 指定索引列表，包含需要增加 freq 的 DictItem 的索引。
     * \note DictItem 的 freq 增加后可能因为重新排序导致位置变化，调用此函数后
     *       之前获取的关于 DictItem 的引用、索引不再可信。
     * \throws std::exception 如果发生错误。
     */
    void auto_inc_freq(std::span<size_t> item_indexes);

    /**
     * \brief 返回 DictItem 引用所对应的索引。
     * \param item 查询的 DictItem 的引用。
     * \return 若参数在此 Dict 中，返回其索引，否则返回 s_npos。
     */
    size_t item_index(const DictItem &item) const noexcept;

    /**
     * \brief 封装接口，返回内部 vector<DictItem> 的 begin() const。
     */
    std::vector<DictItem>::const_iterator begin() const noexcept;

    /**
     * \brief 封装接口，返回内部 vector<DictItem> 的 end() const。
     */
    std::vector<DictItem>::const_iterator end() const noexcept;

    /**
     * \brief 封装接口，返回内部 vector<DictItem> 的 size()。
     */
    size_t size() const noexcept;

    /**
     * \brief 封装接口，调用内部 vector<DictItem> 的 operaotr[]() const。
     */
    const DictItem& operator[](size_t i) const noexcept;

    /**
     * \brief 封装接口，调用内部 vector<DictItem> 的 at() const。
     * \throws std::exception 如果发生错误。
     */
    const DictItem& at(size_t i) const;

    /**
     * \brief 查找符合给定 PinYin::TokenSpan 的 DictItem。
     * \details 对于类型为 Invalid 或 Complete 的 Token，要求完全匹配，
     *          对于类型为 Initial 或 Extendible 的 Token，若非完全匹配（仅开头匹配），
     *          仅在没有完全匹配结果的情况下加入结果中。
     * \param tokens 用于查找的 PinYin::TokenSpan。
     * \return 符合条件的 DictItem 的列表：vector，元素类型为 reference_type<const DictItem>。
     * \throws std::exception 如果发生错误。
     */
    ItemCRefVec search(PinYin::TokenSpan tokens) const;

    /**
     * \brief 查找符合给定 std::string_view 的 DictItem。
     * \param pinyin 用于查找的 std::string_view。
     * \return 符合条件的 DictItem 的列表：vector，元素类型为 reference_type<const DictItem>。
     * \throws std::exception 如果发生错误。
     */
    ItemCRefVec search(std::string_view pinyin) const;

    /**
     * \brief 查找符合给定 std::regex 的 DictItem。
     * \param pattern 用于查找的 std::regex。
     * \return 符合条件的 DictItem 的列表：vector，元素类型为 reference_type<const DictItem>。
     * \throws std::exception 如果发生错误。
     */
    ItemCRefVec search(const std::regex &pattern) const;
private:
    /**
     * \brief 根据策略计算给定索引对应的 DictItem 的建议 freq 增长值。
     * \param idx DictItem 的索引。
     * \return 对该 DictItem 的建议 freq 增长值，若 idx 超出范围，返回 0。
     */
    uint32_t suggest_inc_freq(size_t idx) const noexcept;

    /**
     * \brief 对 vector<DictItem> 进行排序，比较函数见 Dict::item_comp()
     * \throws std::exception 如果发生错误。
     */
    void sort();

    std::vector<DictItem> m_items;
    // 词典 acronym，取自首个加入的 DictItem。
    std::string m_acronym;

    /**
     * \brief DictItem 排序比较器。
     * \details 比较策略为：
     *          1. 若音节数不同，则音节少者优先级高。
     *          2. 若音节数相同，逐一比较单个音节：
     *                 1. 若单个音节长度不同，则单个音节短者优先级高。
     *                 2. 若单个音节长度一致，则按字典序比较单个音节，字典序排前者优先级高。
     *          3. 若音节列表完全一致，则 freq 大者优先级高。
     * \param l 参与比较的 DictItem 的引用。
     * \param r 参与比较的 DictItem 的引用。
     * \return 若 l 优先级高于或等于 r，返回 true，否则返回 false。
     * \throws std::exception 如果发生错误。
     */
    static bool item_comp(const DictItem &l, const DictItem &r) noexcept;
};

} // namespace pinyin_ime

#endif // PINYIN_IME_DICT_H