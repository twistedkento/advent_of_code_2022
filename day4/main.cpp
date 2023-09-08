#include <bitset>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
namespace {
const char *inputMap = nullptr;

constexpr size_t range_length = 100;

[[nodiscard]] inline std::bitset<range_length>
create_assignment(std::uint16_t start, std::uint16_t end) noexcept {
  std::bitset<range_length> ass;
  for (size_t idx = start; idx <= end; ++idx) {
    ass.set(idx);
  }
  return ass;
}

struct [[nodiscard]] AssRange {
  [[nodiscard]] AssRange(const char *in, size_t len) noexcept {
    int i = 0;

    auto parseUntil = [&i, &in, len](auto &val, auto delim) {
      for (; i < len; ++i) {
        if (in[i] >= '0' && in[i] <= '9') {
          val = (val * 10) + (in[i] - '0');
          continue;
        }
        if (in[i] == delim) {
          break;
        }
      }
    };

    parseUntil(range1_start, '-');
    parseUntil(range1_end, ',');
    parseUntil(range2_start, '-');
    parseUntil(range2_end, '\n');

    assignment1 = create_assignment(range1_start, range1_end);
    assignment2 = create_assignment(range2_start, range2_end);
  }

  std::uint16_t range1_start = 0;
  std::uint16_t range1_end = 0;

  std::uint16_t range2_start = 0;
  std::uint16_t range2_end = 0;

  std::bitset<range_length> assignment1;
  std::bitset<range_length> assignment2;
};

[[nodiscard]] std::uint16_t part1(std::vector<AssRange> asses) noexcept {
  size_t total = 0;
  for (auto &ass : asses) {
    auto compare_assignment = (ass.assignment1 & ass.assignment2);
    if (compare_assignment == ass.assignment2 ||
        compare_assignment == ass.assignment1) {
      ++total;
    }
  }

  return total;
}

[[nodiscard]] std::uint16_t part2(std::vector<AssRange> asses) noexcept {
  size_t total = 0;
  for (auto &ass : asses) {
    if ((ass.assignment1 & ass.assignment2) != 0) {
      ++total;
    }
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

  std::vector<AssRange> asses;
  size_t abspos = 0;
  for (size_t i = 0; i < s.st_size; ++i) {
    if (inputMap[i] == '\n') {
      asses.emplace_back(inputMap + abspos, i - abspos);
      abspos = i;
    }
  }

  auto total1 = part1(asses);
  std::printf("Total1: %d\n", total1);

  auto total2 = part2(asses);
  std::printf("Total2: %d\n", total2);

  close(fd);

  return 0;
}
