#ifndef PINYIN_IME_TRIE_H
#define PINYIN_IME_TRIE_H

#include <string>
#include <stack>
#include <memory>
#include <exception>
#include <cassert>
#include <utility>

namespace pinyin_ime {

/**
 * \brief 字典数（Trie）模板类，每个存在于树中的字符串都支持
 *        且必须绑定一个允许默认构造的类对象（即模板参数 Data）。
 */
template <class Data>
class BasicTrie {
public:
    /**
     * \brief 字符串匹配结果，说明一个字符串在 BasicTrie 中匹配的程度。
     * \details Miss：字符串完全不在 BasicTrie 中。
     *          Partial：字符串不在 BasicTrie 中，但是是一个存在于 BasicTrie 中的字符串的开头部分
     *          Extendible：字符串存在于 BasicTrie 中，
     *              同时是另一个存在于 BasicTrie 中的字符串的开头部分。
     *          Complete：字符串存在于 BasicTrie 中，且没有更长的匹配可能。
     */
    enum class MatchResult {
        Miss, Partial, Extendible, Complete
    };

    /**
     * \brief 添加字符串到 BasicTrie。
     *        若字符串不存在，添加字符串，用 Args 构造字符串绑定的 Data 对象并返回其引用。
     *        若字符串存在，无效果，仅返回其绑定的 Data 对象引用。
     * \param str 要添加的字符串。
     * \param args 用于初始化 Data 的可变模板参数。
     * \return str 所绑定的 Data 对象引用。
     * \throws std::logic_error str为空。
     *         std::exception 如果发生错误。
     */
    template <class... Args>
    Data& add_if_miss(std::string_view str, Args&&... args)
    {
        size_t str_size{ str.size() };
        if (str_size == 0)
            throw std::logic_error{ "String is empty" };
        if (!m_root_arr)
            m_root_arr.reset(new NodeArray{});
        NodeArray *arr{ m_root_arr.get() };
        Node *node{ nullptr };
        for (size_t i{ 0 }; i < str_size; ++i) {
            node = &(arr->m_arr[std::abs(str[i] - NodeArray::s_base) % NodeArray::s_size]);
            if (i == str_size - 1) {
                if (node->m_data)
                    break;
                node->m_data.reset(new Data{ std::forward<Args>(args)... });
                break;
            } else {
                if (!node->m_child_arr)
                    node->m_child_arr.reset(new NodeArray{});
                arr = node->m_child_arr.get();
            }
        }
        assert(node);
        return *(node->m_data);
    }

    /**
     * \brief 添加字符串到 BasicTrie。
     *        若字符串不存在，添加字符串，用 Args 构造字符串绑定的 Data 对象并返回其引用。
     *        若字符串已存在，抛出异常 std::logic_error。
     * \param str 要添加的字符串。
     * \param args 用于初始化 Data 的可变模板参数。
     * \return str 所绑定的 Data 对象引用。
     * \throws std::logic_error str为空或字符串已存在。
     *         std::exception 如果发生错误。
     */
    template <class... Args>
    Data& add(std::string_view str, Args&&... args)
    {
        return add_or_assign(str, false, std::forward<Args>(args)...);
    }

    /**
     * \brief 添加字符串到 BasicTrie。
     *        若字符串不存在，添加字符串，用 Args 构造字符串绑定的 Data 对象并返回其引用。
     *        若字符串已存在，用 Args 构造新的 Data 对象替换掉旧的 Data 对象，返回新对象引用。
     * \param str 要添加的字符串。
     * \param args 用于初始化 Data 的可变模板参数。
     * \return str 所绑定的 Data 对象引用。
     * \throws std::logic_error str为空。
     *         std::exception 如果发生错误。
     */
    template <class... Args>
    Data& add_or_assign(std::string_view str, Args&&... args)
    {
        return add_or_assign(str, true, std::forward<Args>(args)...);
    }

    /**
     * \brief 从 BasicTrie 移除字符串。
     * \param str 要移除的字符串。
     */
    void remove(std::string_view str) noexcept
    {
        if (str.empty() || !m_root_arr)
            return;
        NodeArray *parent{ nullptr };
        NodeArray *arr{ m_root_arr.get() };
        size_t str_size{ str.size() };
        for (size_t i{ 0 }; i < str_size; ++i) {
            auto &node{ arr->m_arr[std::abs(str[i] - NodeArray::s_base) % NodeArray::s_size] };
            if (i == str_size - 1) {
                if (!node.m_data)
                    return;
                node.m_data.reset();
                for (auto &n : arr->m_arr) {
                    if (n.m_child_arr || n.m_data)
                        return;
                }
                if (parent) {
                    for (auto &n : parent->m_arr) {
                        if (n.m_child_arr.get() == arr)
                            n.m_child_arr.reset();
                    }
                } else {
                    m_root_arr.reset();
                }
            } else {
                if (!node.m_child_arr)
                    return;
                parent = arr;
                arr = node.m_child_arr.get();
            }
        }
    }

    /**
     * \brief 获取字符串在 BasicTrie 中的匹配程度，见 MatchResult。
     */
    MatchResult match(std::string_view str) const noexcept
    {
        if (str.empty() || !m_root_arr)
            return MatchResult::Miss;
        NodeArray *arr{ m_root_arr.get() };
        size_t str_size{ str.size() };
        for (size_t i{ 0 }; i < str_size; ++i) {
            auto &node{ arr->m_arr[std::abs(str[i] - NodeArray::s_base) % NodeArray::s_size] };
            if (i == str_size - 1) {
                if (node.m_data) {
                    if (node.m_child_arr)
                        return MatchResult::Extendible;
                    else
                        return MatchResult::Complete;
                } else {
                    if (node.m_child_arr)
                        return MatchResult::Partial;
                    else
                        return MatchResult::Miss;
                }
            } else {
                if (!node.m_child_arr)
                    return MatchResult::Miss;
                arr = node.m_child_arr.get();
            }
        }
        return MatchResult::Miss; // should not reach here
    }

    /**
     * \brief 判断字符串是否存在于 BasicTrie 中。
     */
    bool contains(std::string_view str) const noexcept
    {
        auto r = match(str);
        if (r == MatchResult::Complete ||
            r == MatchResult::Extendible)
            return true;
        return false;
    }

    /**
     * \brief 获取字符串在 BasicTrie 中对应的 Data 对象的引用。
     * \param str 查询的字符串。
     * \return str 对应的 Data 对象的引用。
     * \throws std::logic_error 若 str 不在 BasicTrie 中。
     */
    Data& data(std::string_view str) const
    {
        if (str.empty() || !m_root_arr)
            throw std::logic_error{ "String invalid" };
        NodeArray *arr{ m_root_arr.get() };
        size_t str_size{ str.size() };
        for (size_t i{ 0 }; i < str_size; ++i) {
            auto &node{ arr->m_arr[std::abs(str[i] - NodeArray::s_base) % NodeArray::s_size] };
            if (i == str_size - 1) {
                if (!node.m_data)
                    throw std::logic_error{ "String invalid" };
                return *(node.m_data);
            } else {
                if (!node.m_child_arr)
                    throw std::logic_error{ "String invalid" };
                arr = node.m_child_arr.get();
            }
        }
        throw std::logic_error{ "String invalid" }; // should not reach here
    }

    /**
     * \brief 判断 BasicTrie 是否为空。
     */
    bool empty() const noexcept
    {
        return !m_root_arr;
    }

    struct NodeArray;
    struct Node {
        std::unique_ptr<NodeArray> m_child_arr;
        std::unique_ptr<Data> m_data;
    };
    struct NodeArray {
        static constexpr char s_base{ 'a' };
        static constexpr size_t s_size{ 26 };
        Node m_arr[s_size];
    };

    /**
     * \brief BasicTrie 迭代器，用于遍历 BasicTrie，解引用时默认返回字符串绑定的 Data 对象
     *        若要获取字符串本身，调用 string()。
     */
    class Iterator {
    public:
        Data& operator*() const
        {
            if (!m_arr)
                throw std::logic_error{ "Iterator invalid" };
            return *(m_arr->m_arr[m_idx].m_data);
        }

        Data* operator->() const
        {
            if (!m_arr)
                throw std::logic_error{ "Iterator invalid" };
            return m_arr->m_arr[m_idx].m_data.get();
        }

        std::string string() const
        {
            return m_prefix + static_cast<char>(m_idx + NodeArray::s_base);
        }

        Iterator& operator++()
        {
            m_arr = nullptr;
            m_idx = 0;
            m_prefix.clear();
            while (!m_stack.empty()) {
                m_arr = std::get<0>(m_stack.top());
                m_idx = std::get<1>(m_stack.top());
                m_prefix = std::move(std::get<2>(m_stack.top()));
                m_stack.pop();

                for (size_t i = m_idx + 1; i < NodeArray::s_size; ++i) {
                    if (m_arr->m_arr[i].m_child_arr || m_arr->m_arr[i].m_data) {
                        m_stack.push({ m_arr, i, m_prefix });
                        break;
                    }
                }

                auto &node{ m_arr->m_arr[m_idx] };
                if (node.m_child_arr)
                    m_stack.push({ node.m_child_arr.get(), 0, m_prefix + static_cast<char>(m_idx + NodeArray::s_base) });

                if (node.m_data)
                    break;

                m_arr = nullptr;
                m_idx = 0;
                m_prefix.clear();
            }
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator i{ *this };
            ++(*this);
            return i;
        }

        bool operator==(const Iterator& other) const noexcept
        {
            return (m_arr == other.m_arr && m_idx == other.m_idx);
        }

        bool operator!=(const Iterator& other) const noexcept
        {
            return !(*this == other);
        }

    private:
        BasicTrie<Data>::NodeArray *m_arr{ nullptr };
        std::stack<std::tuple<BasicTrie<Data>::NodeArray*, size_t, std::string>> m_stack;
        size_t m_idx{ 0 };
        std::string m_prefix;
        friend class BasicTrie<Data>;
    };
    
    /**
     * \brief 获取起始迭代器。
     */
    Iterator begin() const noexcept
    {
        Iterator iter;
        if (!m_root_arr)
            return iter;
        if (m_root_arr->m_arr[0].m_data) {
            iter.m_arr = m_root_arr.get();
            iter.m_stack.push({ m_root_arr.get(), 1, "" });
            if (m_root_arr->m_arr[0].m_child_arr) {
                iter.m_stack.push({ m_root_arr->m_arr[0].m_child_arr.get(), 0, "a" });
            }
        } else {
            iter.m_stack.push({ m_root_arr.get(), 0, "" });
            ++iter;
        }
        return iter;
    }

    /**
     * \brief 获取结束迭代器。
     */
    Iterator end() const noexcept
    {
        return {};
    }
private:
    /**
     * \brief 添加字符串到 BasicTrie。
     *        若字符串不存在，添加字符串，用 Args 构造字符串绑定的 Data 对象并返回其引用。
     *        若字符串已存在：
     *            1. assign 为 true：用 Args 构造新的 Data 对象替换掉旧的 Data 对象，返回新对象引用。
     *            2. assign 为 false：抛出异常 std::logic_error
     * \param str 要添加的字符串。
     * \param assign 是否在字符串已存在时构造新 Data 进行替换
     * \param args 用于初始化 Data 的可变模板参数。
     * \return str 所绑定的 Data 对象引用。
     * \throws std::logic_error str为空，或字符串已存在但不允许替换绑定对象。
     *         std::exception 如果发生错误。
     */
    template <class... Args>
    Data& add_or_assign(std::string_view str, bool assign, Args&&... args)
    {
        size_t str_size{ str.size() };
        if (str_size == 0)
            throw std::logic_error{ "String is empty" };
        if (!m_root_arr)
            m_root_arr.reset(new NodeArray{});
        NodeArray *arr{ m_root_arr.get() };
        Node *node{ nullptr };
        for (size_t i{ 0 }; i < str_size; ++i) {
            node = &(arr->m_table[std::abs(str[i] - NodeArray::s_base) % NodeArray::s_size]);
            if (i == str_size - 1) {
                if (node->m_data && !assign)
                    throw std::logic_error{ "String exist" };
                node->m_data.reset(new Data{ std::forward<Args>(args)... });
                break;
            } else {
                if (!node->m_child_arr)
                    node->m_child_arr.reset(new NodeArray{});
                arr = node->m_child_arr.get();
            }
        }
        assert(node);
        return *(node->m_data);
    }
    std::unique_ptr<NodeArray> m_root_arr;
};

using Trie = BasicTrie<bool>;

} // namespace pinyin_ime

#endif // PINYIN_IME_TRIE_H