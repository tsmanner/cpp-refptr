#pragma once


constexpr unsigned int repetitions = 1'000'000;

template <typename _T> __attribute__((__noinline__)) void dummy(_T &&) {}

struct Small { char c; };
struct Medium { char s[1000]; };
struct Large { char s[1'000'000]; };
