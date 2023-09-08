#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <string_view>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sstream>
#include <vector>

constexpr size_t filesystem_size = 70000000;

struct FileHelper {
  const char *input_data;
  int fd = -1;
  struct stat s;

  [[nodiscard]] FileHelper(std::string filename) noexcept {
    fd = open(filename.c_str(), O_RDONLY);

    fstat(fd, &s);

    input_data = static_cast<const char *>(
        mmap(nullptr, s.st_size, PROT_READ, MAP_SHARED, fd, 0));
  }

  ~FileHelper() {
    if (fd >= 0) {
      close(fd);
    }
  }

  [[nodiscard]] std::istringstream get_stringstream() const {
    return std::istringstream{std::string(input_data, s.st_size), std::ios_base::in};
  }
};

class MapHeight {
public:
  std::vector<std::string> map;
  MapHeight(std::vector<std::string> map_input) : map(map_input) {
  }

  size_t solve_part1() {
    size_t map_length = map.size();
    size_t total = (map_length * 4) - 4;
    for (int x = 1; x < map_length - 1; ++x) {
      for (int y = 1; y < map_length - 1; ++y) {
        auto tree_len = map[x][y];
        bool longestx2 = true;
        for (int x2 = 0; x2 < x; ++x2) {
          if (map[x2][y] >= tree_len) {
            longestx2 = false;
          }
        }
        bool longestx3 = true;
        for (int x3 = x+1; x3 < map_length; ++x3) {
          if (map[x3][y] >= tree_len) {
            longestx3 = false;
          }
        }
        bool longesty2 = true;
        for (int y2 = 0; y2 < y; ++y2) {
          if (map[x][y2] >= tree_len) {
            longesty2 = false;
          }
        }
        bool longesty3 = true;
        for (int y3 = y+1; y3 < map_length; ++y3) {
          if (map[x][y3] >= tree_len) {
            longesty3 = false;
          }
        }
        if (longestx2 || longestx3 || longesty2 || longesty3) {
          ++total;
        }
      }
    }

    return total;
  }

  size_t solve_part2() {
    size_t map_length = map.size();
    size_t best = 0;
    for (int x = 1; x < map_length - 1; ++x) {
      for (int y = 1; y < map_length - 1; ++y) {
        auto tree_len = map[x][y];
        size_t scenic_score = 0;

        size_t scenic_score_x2 = 0;
        for (int x2 = x-1; x2 >= 0; --x2) {
          if (map[x2][y] >= tree_len || x2 == 0) {
            scenic_score_x2 = x - x2;
            break;
          }
        }

        size_t scenic_score_x3 = 0;
        for (int x3 = x+1; x3 < map_length; ++x3) {
          if (map[x3][y] >= tree_len || x3 == map_length - 1) {
            scenic_score_x3 = x3 - x;
            break;
          }
        }

        size_t scenic_score_y2 = 0;
        for (int y2 = y-1; y2 >= 0; --y2) {
          if (map[x][y2] >= tree_len || y2 == 0) {
            scenic_score_y2 = y - y2;
            break;
          }
        }

        size_t scenic_score_y3 = 0;
        for (int y3= y+1; y3 < map_length; ++y3) {
          if (map[x][y3] >= tree_len || y3 == map_length - 1) {
            scenic_score_y3 = y3 - y;
            break;
          }
        }
        scenic_score = scenic_score_x2 * scenic_score_x3 * scenic_score_y2 * scenic_score_y3;
        best = std::max(best, scenic_score);
      }
    }

    return best;
  }

  [[nodiscard]] static MapHeight create_map(const FileHelper& file_helper) {
    std::string map_slice;

    auto iss = file_helper.get_stringstream();

    std::vector<std::string> map_input;
    while (std::getline(iss, map_slice)) {
      map_input.push_back(map_slice);
    }

    return MapHeight{map_input};
  }


};

[[nodiscard]] size_t part1(const FileHelper &file_helper) noexcept {
  MapHeight map = MapHeight::create_map(file_helper);

  return map.solve_part1();
}

[[nodiscard]] size_t part2(const FileHelper &file_helper) noexcept {
  MapHeight map = MapHeight::create_map(file_helper);

  return map.solve_part2();
}

int main(void) noexcept {
  FileHelper file_helper{"input.txt"};

  auto result = part1(file_helper);
  std::printf("Part1: %lu\n", result);

  auto result2 = part2(file_helper);
  std::printf("Part2: %lu\n", result2);

  return EXIT_SUCCESS;
}
