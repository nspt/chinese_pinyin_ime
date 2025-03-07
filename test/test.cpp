#include <iostream>
#include <iomanip>
#include <limits>
#include "ime.h"

std::string token_type_str(pinyin_ime::PinYin::TokenType type)
{
    using TT = pinyin_ime::PinYin::TokenType;
    switch (type) {
    case TT::Invalid:
        return "X";
    case TT::Initial:
        return "I";
    case TT::Extendible:
        return "E";
    case TT::Complete:
        return "C";
    default:
        return std::to_string(static_cast<int>(type));
    }
}

void eat_line()
{
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void print_ime_state(pinyin_ime::IME &ime)
{
    std::cout << "IME状态:\n";
    std::cout << "    拼音: " << ime.pinyin() << '\n';
    std::cout << "    已固定拼音: " << ime.fixed_letters() << '\n';
    std::cout << "    未固定拼音: " << ime.unfixed_letters() << '\n';
    std::cout << "    Tokens: ";
    for (auto &token : ime.tokens()) {
        std::cout << token.m_token << '(' << token_type_str(token.m_type) << ')' << ' ';
    }
    std::cout << '\n';
    std::cout << "    已固定Tokens: ";
    for (auto &token : ime.fixed_tokens()) {
        std::cout << token.m_token << '(' << token_type_str(token.m_type) << ')' << ' ';
    }
    std::cout << '\n';
    std::cout << "    未固定Tokens: ";
    for (auto &token : ime.unfixed_tokens()) {
        std::cout << token.m_token << '(' << token_type_str(token.m_type) << ')' << ' ';
    }
    std::cout << '\n';
    std::cout << "    已选择词:\n";
    auto &choices{ ime.choices().get() };
    for (size_t i{ 0 }; i < choices.size(); ++i) {
        auto tokens{ choices[i].tokens() };
        auto chinese{ choices[i].chinese() };
        std::cout << std::setw(2) << i << ' '
                  << chinese << ' ';
        for (auto &token : tokens) {
            std::cout << token.m_token << '(' << token_type_str(token.m_type) << ')' << ' ';
        }
        std::cout << '\n';
    }
    std::cout.flush();
}

void print_ime_candidates(pinyin_ime::IME &ime)
{
    auto &candidates{ ime.candidates().get() };
    size_t size{ 0 };
    std::cout << "候选词共有" << candidates.size()
              << "个,输入要打印的个数(0表示不打印,大于等于" << candidates.size()
              << "表示全部打印)" << std::endl;
    if (!(std::cin >> size)) {
        std::cin.clear();
        eat_line();
        std::cout << "输入错误" << std::endl;
        return;
    }
    if (size > candidates.size())
        size = candidates.size();
    eat_line();
    for (size_t i{ 0 }; i < size; ++i) {
        std::cout << std::setw(4) << i << ' '
                  << candidates[i].chinese() << ' '
                  << candidates[i].freq() << ' '
                  << candidates[i].pinyin() << std::endl;
    }
}

void add_pinyin(pinyin_ime::IME &ime)
{
    std::string str;
    std::cout << "输入添加的拼音:" << std::endl;
    std::getline(std::cin, str);
    ime.push_back(str);
}

void backspace(pinyin_ime::IME &ime)
{
    int count;
    std::cout << "输入退格次数:" << std::endl;
    if (!(std::cin >> count)) {
        std::cin.clear();
        eat_line();
        std::cout << "输入错误" << std::endl;
        return;
    }
    eat_line();
    ime.backspace(count);
}

void choose(pinyin_ime::IME &ime)
{
    size_t index;
    std::cout << "输入所选项索引:" << std::endl;
    if (!(std::cin >> index)) {
        std::cin.clear();
        eat_line();
        std::cout << "输入错误" << std::endl;
        return;
    }
    eat_line();
    ime.choose(index);
}

void finish_search(pinyin_ime::IME &ime)
{
    std::cout << "结束搜索" << std::endl;
    ime.finish_search();
}

void reset_search(pinyin_ime::IME &ime)
{
    std::cout << "重置搜索" << std::endl;
    ime.reset_search();
}

void save(pinyin_ime::IME &ime)
{
    std::string file;
    std::cout << "输入文件名:" << std::endl;
    std::getline(std::cin, file);
    ime.save(file);
}

void print_cmd_list()
{
    std::cout << "操作:\n";
    std::cout << "    0. 显示状态\n";
    std::cout << "    1. 显示候选词\n";
    std::cout << "    2. 添加拼音\n";
    std::cout << "    3. 退格\n";
    std::cout << "    4. 选择候选词\n";
    std::cout << "    5. 结束搜索\n";
    std::cout << "    6. 重置搜索\n";
    std::cout << "    7. 保存词典\n";
    std::cout << "    8. 退出" << std::endl;
}

void print_exception(const std::exception& e, bool nested = false)
{
    if (nested)
        std::cerr << ": " << e.what();
    else
        std::cerr << e.what();
    try {
        std::rethrow_if_nested(e);
        std::cerr << '\n';
    } catch (const std::exception& nestedException) {
        print_exception(nestedException, true);
    } catch (...) {}
}

int main(int argc, char *argv[])
{
    using namespace pinyin_ime;

    try {
        int cmd;
        IME ime;
        try {
            std::string dict_file{ "../data/raw_dict_utf8.txt" };
            if (argc >= 2)
                dict_file = argv[1];
            std::cout << "正在加载词典(" << dict_file << ")..." << std::endl;
            ime.load(dict_file);
        } catch (const std::exception &e) {
            std::throw_with_nested(std::runtime_error{"加载词典文件失败"});
        }
        print_cmd_list();
        while (std::cin >> cmd) {
            eat_line();
            switch (cmd) {
            case 0:
                print_ime_state(ime);
                break;
            case 1:
                print_ime_candidates(ime);
                break;
            case 2:
                add_pinyin(ime);
                break;
            case 3:
                backspace(ime);
                break;
            case 4:
                choose(ime);
                break;
            case 5:
                finish_search(ime);
                break;
            case 6:
                reset_search(ime);
                break;
            case 7:
                save(ime);
                break;
            case 8:
                return 0;
            default:
                break;
            }
            print_cmd_list();
        }
        return 0;
    } catch (const std::exception &e) {
        print_exception(e);
        return 1;
    }
}