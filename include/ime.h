#ifndef PINYIN_IME_IME
#define PINYIN_IME_IME

#include <fstream>
#include <ranges>
#include <string_view>
#include <cerrno>
#include "trie.h"
#include "dict.h"
#include "pinyin.h"

namespace pinyin_ime {

class IME {
public:
    IME() = default;
    IME(std::string_view dict_file)
    {
        init(dict_file);
    }
    size_t init(std::string_view dict_file)
    {
        using std::operator""s;
        size_t item_count{ 0 };

        std::ifstream file{ dict_file.data() };
        if (!file)
            throw std::runtime_error{ "Open file failed: "s + std::strerror(errno) };

        // remove BOM and '\r'
        char c1, c2, c3;
        file.get(c1).get(c2).get(c3);
        if (c1 != 0xef || c2 != 0xbb || c3 != 0xbf)
            file.putback(c3).putback(c2).putback(c1);

        for (std::string line; std::getline(file, line);) {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            try {
                add_item_from_line(line);
                ++item_count;
            } catch (...){}
        }
        return item_count;
    }
    void reset_search()
    {

    }

private:
    void add_item_from_line(std::string_view line)
    {
        reset_search();
        DictItem item{ line_to_item(line) };
        std::string acronym{ get_acronym(item.pinyin()) };
        m_dict_trie.add_if_miss(acronym).add_item(std::move(item));
    }
    std::string get_acronym(std::string_view pinyin)
    {
        std::string acronym;
        auto syllables = pinyin | std::views::split(' ') | std::views::transform([](auto &&rng){
            return std::string_view(std::addressof(*rng.begin()), std::ranges::distance(rng));
        });
        for (auto &&s : syllables) {
            if (s.empty())
                continue;
            acronym.push_back(s.front());
        }
        return acronym;
    }
    DictItem line_to_item(std::string_view line)
    {
        std::string chinese;
        std::string pinyin;
        uint32_t freq{ 0 };

        auto b = line.begin();
        auto e = b;
        auto end_iter = line.end();

        // get chinese
        while (e != end_iter && *e != ' ')
            ++e;
        if (e == b || e == end_iter)
            throw std::runtime_error{ "Line format wrong" };
        chinese = std::string{ b, e };

        // get freq
        b = ++e;
        while (e != end_iter && *e != ' ')
            ++e;
        if (e == b || e == end_iter)
            throw std::runtime_error{ "Line format wrong" };
        freq = static_cast<uint32_t>(std::stoul(std::string{ b, e }));
        
        // get pinyin
        b = e + 1;
        if (b == end_iter)
            throw std::runtime_error{ "Line format wrong" };
        pinyin = std::string{ b, end_iter };
        size_t start = pinyin.find_first_not_of(' ');
        size_t end = pinyin.find_last_not_of(' ');
        if (start != std::string::npos && end != std::string::npos) {
            pinyin = pinyin.substr(start, end - start + 1);
        } else {
            throw std::runtime_error{ "Line format wrong" };
        }

        return DictItem{ std::move(chinese), std::move(pinyin), std::move(freq) };
    }
    BasicTrie<Dict> m_dict_trie;
    PinYin m_pinyin;
};

} // namespace pinyin_ime

#endif // PINYIN_IME_IME