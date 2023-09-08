#include <algorithm>
#include <cassert>
#include <compare>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <numeric>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <variant>
#include <vector>
#include <array>

using point_t = std::uint32_t;
using coordinates_t = std::pair<point_t, point_t>;
using result_t = std::uint32_t;

enum class SpaceType : std::int8_t {
  air,
  stone,
  sand,
};

constexpr size_t height = 185;
constexpr size_t width = 687;

using map_t = std::array<std::array<SpaceType, width>, height>;

using wall_t = std::vector<coordinates_t>;

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

  ~FileHelper() noexcept {
    if (fd >= 0) {
      close(fd);
    }
  }

  [[nodiscard]] std::istringstream get_stringstream() const noexcept {
    return std::istringstream{std::string(input_data, s.st_size), std::ios_base::in};
  }

  [[nodiscard]] std::string get_string() const noexcept {
    return std::string(input_data, s.st_size);
  }
};

class Location {
public:
  map_t map;
  size_t sand_id = 0;
  point_t max_y = 0;

  Location(std::istringstream &&file_stream) {
    auto get_if_value = [](std::istringstream &stream, std::string_view sw){
      std::for_each(sw.cbegin(), sw.cend(), [&stream](auto c){
        if (stream.peek() == c) {
          stream.get();
        }
      });
    };
    std::for_each(map.begin(), map.end(), [](map_t::value_type& col){
      col.fill(SpaceType::air);
    });

    std::vector<wall_t> walls;
    wall_t wall;

    while (!file_stream.eof()) {
      if (file_stream.peek() >= '0' && file_stream.peek() <= '9') {
        point_t n1;
        file_stream >> n1;

        auto artifact = file_stream.get();
        if (artifact != ',' && artifact != -1) {
          assert(false);
        }

        point_t n2;
        file_stream >> n2;

        wall.push_back(coordinates_t{n1, n2});

        get_if_value(file_stream, " -> ");

        continue;
      }

      if (file_stream.peek() == 10) {
        walls.push_back(wall);
        wall.clear();
        file_stream.get();
        continue;
      }

      if (file_stream.eof() || file_stream.peek() == -1) {
        break;
      }

      assert(false);
    }

    for (auto w : walls) {
      populate_stone_walls(w);
    }
    max_y = get_highest_y();

  }

  void populate_stone_walls(wall_t w) {
    coordinates_t start;
    start = w.front();

    for (auto it = std::next(w.cbegin(), 1); it < w.cend(); ++it) {
      size_t start_y = std::min(start.second, ((*it).second));
      size_t end_y = std::max(start.second, ((*it).second));
      size_t start_x =  std::min(start.first, ((*it).first));
      size_t end_x =  std::max(start.first, ((*it).first));

      for (size_t y = start_y; y <= end_y; ++y) {
        for (size_t x = start_x; x <= end_x; ++x) {
          map[y][x] = SpaceType::stone;
        }
      }
    start = (*it);
    }
  }

  void create_bottom_floor() {
    for (size_t x = 0; x < width; ++x) {
      assert(max_y + 2 < height && x >= 0 && x < width);
      map[max_y + 2][x] = SpaceType::stone;
    }

  }

  void print_thing() {
    for (size_t y = 0; y < max_y + 2; ++y) {
      for (size_t x = 316; x < width; ++x) {
        if (map[y][x] == SpaceType::air) {
          std::cout << ".";
        } else if (map[y][x] == SpaceType::stone) {
          std::cout << "#";
        } else if (map[y][x] == SpaceType::sand) {
          std::cout << "o";
        }
      }
      std::cout << "\n";
    }
    std::cout << "\n";
  }

  point_t get_highest_y() {
    point_t result = 0;
    for (size_t y = 0; y < height; ++y) {
      for (size_t x = 0; x < width; ++x) {
        if (map[y][x] == SpaceType::stone) {
          result = std::max<point_t>(result, y);
        }
      }
    }
    return result;
  }

  size_t solve_part1() {
    while (insert_sand()) {
    }
    return sand_id;
  }

  bool insert_sand() {
    size_t x = 500;
    size_t y = 0;
    if (map[y][x] == SpaceType::sand) {
      return false;
    }
    while (true) {
      if (map[y + 1][x] == SpaceType::air && !(y + 1 > 184)) {
        ++y;
      } else if (map[y + 1][x - 1] == SpaceType::air && !(y + 1 > 184)) {
        ++y;
        --x;
      } else if (map[y + 1][x + 1] == SpaceType::air && !(y + 1 > 184)) {
        ++y;
        ++x;
      } else {
        assert(y < height && x > 0 && x < width);
        map[y][x] = SpaceType::sand;
        ++sand_id;
        return true;
      }
      if (is_above_void(x,y)) {
        return false;
      }
    }
  }

  bool is_above_void(size_t x, size_t y) {
    for (size_t y_pos = y; y_pos < height; ++y_pos) {
      if (map[y_pos][x] != SpaceType::air) {
        return false;
      }
    }
    return true;
  }
};


result_t part1(const FileHelper &file_helper) noexcept {
  Location l{file_helper.get_stringstream()};

  auto result = l.solve_part1();

  std::printf("Part1: %lu\n", result);

  assert(result == 964);
  return result;
}

result_t part2(const FileHelper &file_helper) noexcept {
  Location l{file_helper.get_stringstream()};
  l.create_bottom_floor();
  l.max_y = l.get_highest_y();

  auto result = l.solve_part1();

  std::printf("Part2: %lu\n", result);

  assert(result == 32041);
  return result;
}



int main(void) noexcept {
  {
    FileHelper file_helper{"input.txt"};

    {
      part1(file_helper);
    }

    {
      part2(file_helper);
    }
  }

  return EXIT_SUCCESS;
}
