#ifndef PINYIN_IME_CANDIDATES_H
#define PINYIN_IME_CANDIDATES_H

#include "query.h"

namespace pinyin_ime {

/**
 * \brief IME 向外部提供候选词的类，本质上是对多个 Query 对象（std::vector<Query>）的封装，
 *        使外部能以连续的形式访问多个 Query 对象的查询结果的集合。\
 * \details 借助 shared_ptr，Candidates 允许外部高效拷贝使用。
 */
class Candidates {
public:
    using QueryRef = std::reference_wrapper<Query>;

    /**
     * \brief Candidates 迭代器，解引用获取 const DictItem&。
     */
    class Iterator {
    public:
        Iterator() = default;
        Iterator(const Candidates *c, size_t i)
            : m_candidates{ c }, m_idx{ i }
        {}
        const DictItem& operator*() const
        {
            return (*m_candidates)[m_idx];
        }
        const DictItem* operator->() const
        {
            return &((*m_candidates)[m_idx]);
        }
        Iterator& operator++()
        {
            ++m_idx;
            return *this;
        }
        Iterator operator++(int)
        {
            Iterator i{ *this };
            ++m_idx;
            return i;
        }
        Iterator& operator--()
        {
            --m_idx;
            return *this;
        }
        Iterator operator--(int)
        {
            Iterator i{ *this };
            --m_idx;
            return i;
        }
        Iterator& operator+(size_t offset)
        {
            m_idx += offset;
            return *this;
        }
        Iterator& operator-(size_t offset)
        {
            m_idx -= offset;
            return *this;
        }
        std::ptrdiff_t operator-(const Iterator& rhs)
        {
            if (m_idx >= rhs.m_idx)
                return static_cast<std::ptrdiff_t>(m_idx - rhs.m_idx);
            return -(static_cast<std::ptrdiff_t>(rhs.m_idx - m_idx));
        }
        bool operator==(const Iterator& rhs)
        {
            return m_candidates == rhs.m_candidates && m_idx == rhs.m_idx;
        }
        bool operator!=(const Iterator& rhs)
        {
            return !(*this == rhs);
        }
    private:
        const Candidates* m_candidates{ nullptr };
        size_t m_idx{ 0 };

        friend class Candidates;
    };

    /**
     * \brief 构造函数
     */
    Candidates();

    /**
     * \brief 返回 Candidates 中候选词 DictItem 的总数。
     */
    size_t size() const noexcept;

    /**
     * \brief 判断 Candidates 是否为空。
     */
    bool empty() const noexcept;

    /**
     * \brief 获取 idx 对应的 const DictItem&。
     */
    const DictItem& operator[](size_t idx) const noexcept;

    /**
     * \brief 返回起始迭代器。
     */
    Iterator begin() const noexcept;

    /**
     * \brief 返回结束迭代器。
     */
    Iterator end() const noexcept;
private:

    /**
     * \brief 尾添加新的 Query。
     */
    void push_back(Query query);

    /**
     * \brief 清空 Query。
     */
    void clear() noexcept;

    /**
     * \brief 将 Candidates 迭代器转换为内部 Query 和 Query 的结果索引。
     * \throws std::out_of_range 如果 it 无效。
     */
    std::pair<QueryRef, size_t> to_query_and_index(const Iterator& it);

    /**
     * \brief 将 Candidates 索引转换为内部 Query 和 Query 的结果索引。
     * \throws std::out_of_range 如果 idx 无效。
     */
    std::pair<QueryRef, size_t> to_query_and_index(size_t idx);

    std::shared_ptr<std::vector<Query>> m_queries;
    friend class IME;
};

} // namespace pinyin_ime

#endif // PINYIN_IME_CANDIDATES_H