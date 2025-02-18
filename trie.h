#include <string>
#include <memory>
#include <exception>
#include <utility>

namespace pinyin_ime {

template <class Data>
class BasicTrie {
public:
    template <class... Args>
    void add(std::string_view str, Args&&... args)
    {
        add_or_assign(str, false, std::forward<Args>(args)...);
    }

    template <class... Args>
    void add_or_assign(std::string_view str, Args&&... args)
    {
        add_or_assign(str, true, std::forward<Args>(args)...);
    }

    void remove(std::string_view str) noexcept
    {
        if (str.empty() || !root_)
            return;
        Node *parent = nullptr;
        Node *node = root_.get();
        auto str_size = str.size();
        for (size_t i = 0; i < str_size; ++i) {
            size_t idx = static_cast<size_t>(str[i] - 'a') % 26;
            auto &end = node->table_[idx];
            if (i == str_size - 1) {
                if (!end.second)
                    return;
                end.second.reset();
                for (auto &p : node->table_) {
                    if (p.first || p.second)
                        return;
                }
                if (parent) {
                    for (auto &p : parent->table_) {
                        if (p.first.get() == node)
                            p.first.reset();
                    }
                } else {
                    root_.reset();
                }
            } else {
                auto &next_node = end.first;
                if (!next_node)
                    return;
                parent = node;
                node = next_node.get();
            }
        }
    }

    bool contains(std::string_view str)
    {
        if (str.empty() || !root_)
            return false;
        Node *node = root_.get();
        auto str_size = str.size();
        for (size_t i = 0; i < str_size; ++i) {
            size_t idx = static_cast<size_t>(str[i] - 'a') % 26;
            auto &end_pair = node->table_[idx];
            if (i == str_size - 1) {
                if (!end_pair.second)
                    return false;
                return true;
            } else {
                auto &next_node = end_pair.first;
                if (!next_node)
                    return false;
                node = next_node.get();
            }
        }
        return false; // should not reach here
    }

    Data& data(std::string_view str)
    {
        if (str.empty() || !root_)
            throw std::logic_error{ "String invalid" };
        Node *node = root_.get();
        auto str_size = str.size();
        for (size_t i = 0; i < str_size; ++i) {
            size_t idx = static_cast<size_t>(str[i] - 'a') % 26;
            auto &end_pair = node->table_[idx];
            if (i == str_size - 1) {
                if (!end_pair.second)
                    throw std::logic_error{ "String invalid" };
                return *(end_pair.second);
            } else {
                auto &next_node = end_pair.first;
                if (!next_node)
                    throw std::logic_error{ "String invalid" };
                node = next_node.get();
            }
        }
        throw std::logic_error{ "String invalid" }; // should not reach here
    }

private:
    template <class... Args>
    void add_or_assign(std::string_view str, bool assign, Args&&... args)
    {
        if (str.empty())
            return;
        if (!root_)
            root_.reset(new Node{});
        Node *node = root_.get();
        auto str_size = str.size();
        for (size_t i = 0; i < str_size; ++i) {
            size_t idx = static_cast<size_t>(str[i] - 'a') % 26;
            auto &end_pair = node->table_[idx];
            if (i == str_size - 1) {
                if (end_pair.second && !assign)
                    throw std::logic_error{ "String exist" };
                end_pair.second.reset(new Data{ std::forward<Args>(args)... });
            } else {
                auto &next_node = end_pair.first;
                if (!next_node)
                    next_node.reset(new Node{});
                node = next_node.get();
            }
        }
    }
    struct Node {
        std::pair<std::unique_ptr<Node>, std::unique_ptr<Data>> table_[26];
    };
    std::unique_ptr<Node> root_;
};

template <>
class BasicTrie<bool> {
public:
    void add(std::string_view str)
    {
        if (str.empty())
            return;
        if (!root_)
            root_.reset(new Node{});
        Node *node = root_.get();
        auto str_size = str.size();
        for (size_t i = 0; i < str_size; ++i) {
            size_t idx = static_cast<size_t>(str[i] - 'a') % 26;
            auto &end_pair = node->table_[idx];
            if (i == str_size - 1) {
                end_pair.second = true;
            } else {
                auto &next_node = end_pair.first;
                if (!next_node)
                    next_node.reset(new Node{});
                node = next_node.get();
            }
        }
    }

    void remove(std::string_view str) noexcept
    {
        if (str.empty() || !root_)
            return;
        Node *parent = nullptr;
        Node *node = root_.get();
        auto str_size = str.size();
        for (size_t i = 0; i < str_size; ++i) {
            size_t idx = static_cast<size_t>(str[i] - 'a') % 26;
            auto &end = node->table_[idx];
            if (i == str_size - 1) {
                if (!end.second)
                    return;
                end.second = false;
                for (auto &p : node->table_) {
                    if (p.first || p.second)
                        return;
                }
                if (parent) {
                    for (auto &p : parent->table_) {
                        if (p.first.get() == node)
                            p.first.reset();
                    }
                } else {
                    root_.reset();
                }
            } else {
                auto &next_node = end.first;
                if (!next_node)
                    return;
                parent = node;
                node = next_node.get();
            }
        }
    }

    bool contains(std::string_view str)
    {
        if (str.empty() || !root_)
            return false;
        Node *node = root_.get();
        auto str_size = str.size();
        for (size_t i = 0; i < str_size; ++i) {
            size_t idx = static_cast<size_t>(str[i] - 'a') % 26;
            auto &end_pair = node->table_[idx];
            if (i == str_size - 1) {
                return end_pair.second;
            } else {
                auto &next_node = end_pair.first;
                if (!next_node)
                    return false;
                node = next_node.get();
            }
        }
        return false; // should not reach here
    }

    bool data(std::string_view str)
    {
        return contains(str);
    }

private:
    struct Node {
        std::pair<std::unique_ptr<Node>, bool> table_[26];
    };
    std::unique_ptr<Node> root_;
};

using Trie = BasicTrie<bool>;

} // namespace pinyin_ime
