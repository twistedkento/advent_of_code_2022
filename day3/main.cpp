#include <algorithm>
#include <bitset>
#include <cstdio>
#include <fcntl.h>
#include <sstream>
#include <sys/mman.h>
#include <sys/stat.h>

namespace {
const char *inputMap = nullptr;

[[nodiscard]] inline unsigned char get_char_value(char item) noexcept {
  if (item >= 'a' && item <= 'z') {
    return item - 97;
  }
  if (item >= 'A' && item <= 'Z') {
    return item - 39;
  }
  return 0;
}

constexpr size_t index_size = 52;

template<typename... Targs>
[[nodiscard]] inline unsigned char find_intersecting(Targs&&...v) noexcept {
  for (size_t idx = 0; idx < index_size; ++idx) {
    if ((... && v[idx])) {
      return idx + 1;
    }
  }
  return 0;
}


[[nodiscard]] unsigned int part1(auto &&ss) noexcept {
  unsigned int total = 0;

  for (std::string s; std::getline(ss, s);) {
    auto it = std::next(s.begin(), s.length() / 2);

    std::bitset<index_size> index;

    std::for_each(s.begin(), it,
                  [&index](auto c) { index.set(get_char_value(c)); });

    std::bitset<index_size> second_index;

    std::for_each(it, s.end(), [&second_index](auto c) {
      second_index.set(get_char_value(c));
    });

    total += find_intersecting(index, second_index);
  }

  return total;
}

[[nodiscard]] unsigned int part2(auto &&ss) noexcept {
  unsigned int total = 0;

  for (std::string s1, s2, s3; ss.peek() != EOF;) {
    std::getline(ss, s1);
    std::bitset<index_size> index1;
    std::for_each(s1.begin(), s1.end(),
                  [&index1](auto c) { index1.set(get_char_value(c)); });

    std::getline(ss, s2);
    std::bitset<index_size> index2;
    std::for_each(s2.begin(), s2.end(),
                  [&index2](auto c) { index2.set(get_char_value(c)); });

    std::getline(ss, s3);
    std::bitset<index_size> index3;
    std::for_each(s3.begin(), s3.end(),
                  [&index3](auto c) { index3.set(get_char_value(c)); });

    total += find_intersecting(index1, index2, index3);
  }

  return total;
}

} // namespace
int main(void) noexcept {
  struct stat s;
  int fd = open("input.txt", O_RDONLY);
  fstat(fd, &s);

  inputMap = static_cast<const char *>(
      mmap(nullptr, s.st_size, PROT_READ, MAP_SHARED, fd, 0));
  std::istringstream iss1{std::string(inputMap, s.st_size), std::ios_base::in};
  auto total1 = part1(iss1);
  std::printf("Total1: %d\n", total1);
  std::istringstream iss2{std::string(inputMap, s.st_size), std::ios_base::in};
  auto total2 = part2(iss2);
  std::printf("Total2: %d\n", total2);

  return 0;
}
