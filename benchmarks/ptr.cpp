#include "refptr.h"

#include <fmt/core.h>
#include <fmt/chrono.h>

#include <array>
#include <chrono>

#include "config.h"


template <typename _T, unsigned _I>
void run(char const *prefix) {
  using clk = std::chrono::steady_clock;
  using namespace std::literals;

  unsigned int i = 0;
  _T o {};
  _T *t = &o;

  auto t0 = clk::now();
  for (; i < repetitions; ++i) {
    std::array<_T *, _I> ref;
    for (std::size_t j = 0; j < ref.size(); ++j) { ref[j] = t; }
    dummy(ref);
  }
  auto t1 = clk::now();
  auto dt = t1 - t0;

  auto s = std::chrono::duration_cast<std::chrono::seconds>(dt);
  fmt::print("{} {:>7} {:>7} {:>5} {:>2}.{:0>9}s {:>3}x{} times\n", prefix, sizeof(_T), sizeof(_T), dt/(_I*i), s.count(), (dt - s).count(), _I, i);
}


int main() {
  fmt::print("{}\n", __FILE__);
  run< Small,   1>("Small: ");
  run<Medium,   1>("Medium:");
  run< Large,   1>("Large: ");
  run< Small,  10>("Small: ");
  run<Medium,  10>("Medium:");
  run< Large,  10>("Large: ");
  run< Small, 100>("Small: ");
  run<Medium, 100>("Medium:");
  run< Large, 100>("Large: ");
  return 0;
}
