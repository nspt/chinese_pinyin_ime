#ifndef PINYIN_IME_QUERY_H
#define PINYIN_IME_QUERY_H

#include "pinyin.h"
#include "dict.h"
#include "trie.h"

namespace pinyin_ime {

/**
 * \brief 负责从绑定的词典树 BasicTire<Dict> 中查询符合给定 PinYin::TokenSpan 的 DictItem。
 * \note Query 只应在 IME 内部使用。
 * \details Query 对象供 IME 内部使用，其本身几乎不保存资源，而是保存对资源的引用，使用时需要谨慎：
 *              1. Query 对象以引用的形式绑定到 BasicTrie<Dict>，因此使用 Query 对象时必须
 *                 保证 BasicTrie<Dict> 的存在
 *              2. Query 对象查询所用的 PinYin::TokenSpan 来自于外部的 PinYin 对象，TokenSpan
 *                 的有效性需要外部保证，Query::tokens() 仅返回 Query 对象查询时保存的 TokenSpan。
 *              3. Query 对象在查询结束后，会记录找到的 Dict 对象的地址（Query::dict() 获取），
 *                 若该 Dict 对象不再存在，则 Query::dict() 返回的 Dict 对象地址不再有效。
 *              4. Query 对象在查询结束后，会保存 Dict::search() 返回的 ItemCRefVec
 *                 （一个vector，元素为reference_wrapper<const DictItem>），
 *                 若该 Dict 对象发生了修改，则 Query::items() 返回的查询结果不再有效。
 */
class Query {
public:
    using ItemCRefVecCRef = std::reference_wrapper<const Dict::ItemCRefVec>;
    using ItemCRef = std::reference_wrapper<const DictItem>;

    /**
     * \brief 构造函数，仅绑定 BasicTrie<Dict>。
     * \param dict_trie Query 对象绑定的 BasicTrie<Dict>。
     */
    Query(BasicTrie<Dict> &dict_trie) noexcept;

    /**
     * \brief 构造函数，绑定 BasicTrie<Dict> 并立刻进行查询。
     * \param dict_trie Query 对象绑定的 BasicTrie<Dict>。
     * \param tokens 需要查询的 TokenSpan。
     */
    Query(BasicTrie<Dict> &dict_trie, PinYin::TokenSpan tokens) noexcept;

    /**
     * \brief 默认拷贝构造。
     */
    Query(const Query&) = default;

    /**
     * \brief 移动构造。
     * \details 移动后，other 依然绑定原 BasicTrie<Dict>，但是其它资源移动至此对象。
     */
    Query(Query&& other) noexcept;

    /**
     * \brief 默认拷贝赋值。
     */
    Query& operator=(const Query&) = default;

    /**
     * \brief 移动赋值。
     * \details 移动后，other 依然绑定原 BasicTrie<Dict>，但是其它资源移动至此对象。
     */
    Query& operator=(Query &&other) noexcept;

    /**
     * \brief 执行查询。
     * \param tokens 需要查询的 TokenSpan。
     * \return 查询成功返回 true，失败返回 false。
     * \note 找到对应 Dict 对象即视为查询成功，匹配结果可以为空。
     */
    bool exec(PinYin::TokenSpan tokens) noexcept;

    /**
     * \brief 判断此对象是否已经执行过查询。
     * \param tokens 需要查询的 TokenSpan。
     * \return 查询成功返回 true，失败返回 false。
     * \note 找到对应 Dict 对象即视为查询成功，匹配结果可以为空。
     */
    bool is_active() const noexcept;

    /**
     * \brief 返回此对象查询到的 Dict 对象地址。
     */
    Dict* dict() const noexcept;

    /**
     * \brief 返回此对象查询所用的 TokenSpan。
     */
    PinYin::TokenSpan tokens() const noexcept;

    /**
     * \brief 返回此对象查询结果的const引用（reference_wrapper，以免外部自动拷贝）。
     */
    ItemCRefVecCRef items() const noexcept;

    /**
     * \brief 返回此对象查询结果的 size()。
     */
    size_t size() const noexcept;

    /**
     * \brief 判断此对象查询结果是否为空。
     */
    bool empty() const noexcept;

    /**
     * \brief 访问此对象查询结果中 DictItem 的引用（reference_wrapper，以免外部自动拷贝）。
     */
    ItemCRef operator[](size_t idx) const noexcept;

    /**
     * \brief 清除查询结果及查询所用的 TokenSpan。
     */
    void clear() noexcept;
private:
    std::reference_wrapper<BasicTrie<Dict>> m_dict_trie_ref;
    Dict* m_dict{ nullptr };
    PinYin::TokenSpan m_tokens;
    Dict::ItemCRefVec m_items;
};

} // namespace pinyin_ime

#endif // PINYIN_IME_QUERY_H