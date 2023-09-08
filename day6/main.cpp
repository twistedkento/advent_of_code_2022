#include <algorithm>
#include <bitset>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <string_view>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>


struct FileHelper {
  const char *input_data;
  int fd = -1;
  struct stat s;

  [[nodiscard]] FileHelper(std::string filename) noexcept {
    fd = open("input.txt", O_RDONLY);

    fstat(fd, &s);

    input_data = static_cast<const char *>(
        mmap(nullptr, s.st_size, PROT_READ, MAP_SHARED, fd, 0));
  }

  ~FileHelper() {
    if (fd >= 0) {
      close(fd);
    }
  }
};

[[nodiscard]] size_t subroutine(const char *in, size_t in_length,
                                size_t data_msg_length) noexcept {
  std::string_view check{in, in_length};

  for (size_t idx = 0; idx < check.length(); ++idx) {
    bool failed = false;
    auto begin = std::next(check.begin(), idx);
    auto end = std::next(begin, data_msg_length);
    std::for_each(begin, end, [&failed, check_bitset = std::bitset<123>()](const char &c) mutable {
        failed |= check_bitset[c];
        check_bitset.set(c);
      });
    if (!failed) {
      return std::distance(check.begin(), end);
    }
  }

  return 0;
}

int main(void) noexcept {
  FileHelper file_helper{"input.txt"};

  auto offset1 = subroutine(file_helper.input_data, file_helper.s.st_size, 4);
  std::printf("Part1: %lu\n", offset1);

  auto offset2 = subroutine(file_helper.input_data, file_helper.s.st_size, 14);
  std::printf("Part2: %lu\n", offset2);

  return EXIT_SUCCESS;
}
