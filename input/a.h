#pragma once
#include <vector>
struct B
{
    int b;
};
struct A
{
    std::vector<int> int_arr;
    std::vector<B> b_arr;
    int int_val;
};