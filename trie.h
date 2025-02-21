#ifndef PINYIN_IME_TRIE
#define PINYIN_IME_TRIE

#include <string>
#include <stack>
#include <memory>
#include <exception>
#include <utility>

namespace pinyin_ime {

template <class Data>
class BasicTrie {
public:
    template <class D>
    class Iterator {
    public:
        D& operator*() const
        {
            if (!m_node)
                throw std::logic_error{ "Iterator invalid" };
            return *m_node->m_table[m_idx].second;
        }

        D* operator->() const
        {
            if (!m_node)
                throw std::logic_error{ "Iterator invalid" };
            return m_node->m_table[m_idx].second.get();
        }

        std::string string() const
        {
            return m_prefix + static_cast<char>(m_idx + 'a');
        }

        Iterator& operator++()
        {
            m_node = nullptr;
            m_idx = 0;
            m_prefix.clear();
            while (!m_stack.empty()) {
                m_node = std::get<0>(m_stack.top());
                m_idx = std::get<1>(m_stack.top());
                m_prefix = std::move(std::get<2>(m_stack.top()));
                m_stack.pop();

                for (auto i = m_idx + 1; i < s_table_size; ++i) {
                    if (m_node->m_table[i].first || m_node->m_table[i].second) {
                        m_stack.push({ m_node, i, m_prefix});
                        break;
                    }
                }

                auto &end{ m_node->m_table[m_idx] };
                if (end.first)
                    m_stack.push({ end.first.get(), 0, m_prefix + static_cast<char>(m_idx + 'a') });

                if (end.second)
                    break;

                m_node = nullptr;
                m_idx = 0;
                m_prefix.clear();
            }
            return *this;
        }

        bool operator==(const Iterator& other) const
        {
            return (m_node == other.m_node && m_idx == other.m_idx);
        }

        bool operator!=(const Iterator& other) const
        {
            return !(*this == other);
        }

    private:
        Iterator() = default;
        typename BasicTrie<D>::Node *m_node{ nullptr };
        std::stack<std::tuple<decltype(m_node), uint8_t, std::string>> m_stack;
        uint8_t m_idx{ 0 };
        std::string m_prefix;
        static constexpr size_t s_table_size{ 
            sizeof(BasicTrie<D>::Node::m_table) / sizeof(BasicTrie<D>::Node::m_table[0])
        };
        friend class BasicTrie<D>;
    };

    enum class MatchResult {
        Miss, Partial, Extendible, Complete
    };

    template <class... Args>
    Data& add_if_miss(std::string_view str, Args&&... args)
    {
        if (str.empty())
            throw std::logic_error{ "String is empty" };
        if (!m_root)
            m_root.reset(new Node{});
        Node *node{ m_root.get() };
        size_t str_size{ str.size() };
        typename Node::End *end{ nullptr };
        for (size_t i{ 0 }; i < str_size; ++i) {
            uint8_t idx{ static_cast<uint8_t>((str[i] - 'a') % 26) };
            end = &(node->m_table[idx]);
            if (i == str_size - 1) {
                if (end->second)
                    break;
                end->second.reset(new Data{ std::forward<Args>(args)... });
                break;
            } else {
                auto &next_node{ end->first };
                if (!next_node)
                    next_node.reset(new Node{});
                node = next_node.get();
            }
        }
        if (end)
            return *(end->second);
        throw std::logic_error{ "Add failed! Should not happen!" };
    }

    template <class... Args>
    Data& add(std::string_view str, Args&&... args)
    {
        return add_or_assign(str, false, std::forward<Args>(args)...);
    }

    template <class... Args>
    Data& add_or_assign(std::string_view str, Args&&... args)
    {
        return add_or_assign(str, true, std::forward<Args>(args)...);
    }

    void remove(std::string_view str) noexcept
    {
        if (str.empty() || !m_root)
            return;
        Node *parent{ nullptr };
        Node *node{ m_root.get() };
        size_t str_size{ str.size() };
        for (size_t i{ 0 }; i < str_size; ++i) {
            uint8_t idx{ static_cast<uint8_t>((str[i] - 'a') % 26) };
            auto &end{ node->m_table[idx] };
            if (i == str_size - 1) {
                if (!end.second)
                    return;
                end.second.reset();
                for (auto &p : node->m_table) {
                    if (p.first || p.second)
                        return;
                }
                if (parent) {
                    for (auto &p : parent->m_table) {
                        if (p.first.get() == node)
                            p.first.reset();
                    }
                } else {
                    m_root.reset();
                }
            } else {
                auto &next_node{ end.first };
                if (!next_node)
                    return;
                parent = node;
                node = next_node.get();
            }
        }
    }

    MatchResult match(std::string_view str) const
    {
        if (str.empty() || !m_root)
            return MatchResult::Miss;
        Node *node{ m_root.get() };
        size_t str_size{ str.size() };
        for (size_t i{ 0 }; i < str_size; ++i) {
            uint8_t idx{ static_cast<uint8_t>((str[i] - 'a') % 26) };
            auto &end_pair{ node->m_table[idx] };
            if (i == str_size - 1) {
                if (end_pair.second) {
                    if (end_pair.first)
                        return MatchResult::Extendible;
                    else
                        return MatchResult::Complete;
                } else {
                    if (end_pair.first)
                        return MatchResult::Partial;
                    else
                        return MatchResult::Miss;
                }
            } else {
                auto &next_node{ end_pair.first };
                if (!next_node)
                    return MatchResult::Miss;
                node = next_node.get();
            }
        }
        return MatchResult::Miss; // should not reach here
    }

    bool contains(std::string_view str) const
    {
        auto r = match(str);
        if (r == MatchResult::Complete ||
            r == MatchResult::Extendible)
            return true;
        return false;
    }

    Data& data(std::string_view str) const
    {
        if (str.empty() || !m_root)
            throw std::logic_error{ "String invalid" };
        Node *node{ m_root.get() };
        size_t str_size{ str.size() };
        for (size_t i{ 0 }; i < str_size; ++i) {
            uint8_t idx{ static_cast<uint8_t>((str[i] - 'a') % 26) };
            auto &end{ node->m_table[idx] };
            if (i == str_size - 1) {
                if (!end.second)
                    throw std::logic_error{ "String invalid" };
                return *(end.second);
            } else {
                auto &next_node{ end.first };
                if (!next_node)
                    throw std::logic_error{ "String invalid" };
                node = next_node.get();
            }
        }
        throw std::logic_error{ "String invalid" }; // should not reach here
    }

    bool empty() const
    {
        return !m_root;
    }

    Iterator<Data> begin() const
    {
        Iterator<Data> iter;
        if (!m_root)
            return iter;
        if (m_root->m_table[0].second) {
            iter.m_stack.push({ m_root.get(), 1, "" });
            if (m_root->m_table[0].first) {
                iter.m_stack.push({ m_root->m_table[0].first.get(), 0, "a" });
            }
            iter.m_node = m_root.get();
        } else {
            iter.m_stack.push({ m_root.get(), 0, "" });
            ++iter;
        }
        return iter;
    }

    constexpr Iterator<Data> end() const
    {
        return Iterator<Data>{};
    }

    std::ostream& print(std::ostream &os, std::string_view separator) const
    {
        bool need_separator{ false };
        auto end_iter = end();
        for (auto iter = begin(); iter != end_iter; ++iter) {
            if (!need_separator) {
                need_separator = true;
                os << iter.string();
            } else {
                os << separator << iter.string();
            }
        }
        return os;
    }

    template <class U>
    friend std::ostream& operator<<(std::ostream &os, const BasicTrie<U> &trie)
    {
        return trie.print(os, " ");
    }

private:
    template <class... Args>
    Data& add_or_assign(std::string_view str, bool assign, Args&&... args)
    {
        if (str.empty())
            throw std::logic_error{ "String is empty" };
        if (!m_root)
            m_root.reset(new Node{});
        Node *node{ m_root.get() };
        size_t str_size{ str.size() };
        decltype(Node::m_table[0]) *end{ nullptr };
        for (size_t i{ 0 }; i < str_size; ++i) {
            uint8_t idx{ static_cast<uint8_t>((str[i] - 'a') % 26) };
            end = &(node->m_table[idx]);
            if (i == str_size - 1) {
                if (end->second && !assign)
                    throw std::logic_error{ "String exist" };
                end->second.reset(new Data{ std::forward<Args>(args)... });
                break;
            } else {
                auto &next_node{ end->first };
                if (!next_node)
                    next_node.reset(new Node{});
                node = next_node.get();
            }
        }
        if (end)
            return *(end->second);
        throw std::logic_error{ "Add failed! Should not happen!" };
    }
    struct Node {
        using End = std::pair<std::unique_ptr<Node>, std::unique_ptr<Data>>;
        End m_table[26];
    };
    std::unique_ptr<Node> m_root;
    friend class BasicTrie<Data>::Iterator<Data>;
};

using Trie = BasicTrie<bool>;

} // namespace pinyin_ime

#endif // PINYIN_IME_TRIE