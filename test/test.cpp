#include <iomanip>
#include <ranges>
#include <string_view>
#include <iostream>
#include <vector>
#include <span>
#include "ime.h"
#include <iostream>
#include <cstring>

int main()
{
    using namespace std;
    using namespace pinyin_ime;
    IME ime{ "../data/raw_dict_utf8.txt" };
    auto &candidates = ime.search("z'i'zhi'zhi'ming").get();
    for (auto &cand : candidates) {
        cout << cand.chinese() << ' '
             << cand.freq() << ' '
             << cand.pinyin() << endl;
    }
    return 0;
}