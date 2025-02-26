#include <iomanip>
#include <iostream>
#include <ranges>
#include <string_view>
#include <vector>
#include <span>
#include "trie.h"
 
int main()
{
    using namespace std;
    std::vector<int> v{ 1, 2, 3, 4, 5 };

    const auto &vr{ v };
    span<const int> s{ vr.begin() + 1, vr.end() };

    return 0;
}