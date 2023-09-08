#include <algorithm>
#include <bits/chrono.h>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <execution>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <ostream>
#include <pstl/glue_execution_defs.h>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

using map_value_t = std::int32_t;
using traversed_map_1d_t = std::vector<map_value_t>;
using traversed_map_2d_t = std::shared_ptr<std::vector<traversed_map_1d_t>>;
using map_coord_t = std::pair<map_value_t, map_value_t>;
using height_map_entry_t = std::pair<map_coord_t, map_value_t>;
using height_map_entry_container_t = std::vector<height_map_entry_t>;

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

class HeightMap {
public:
  std::vector<std::string> map;
  traversed_map_2d_t visited_map;
  map_value_t width;
  map_value_t height;
  char max_elevation;
  map_coord_t current_position;
  map_coord_t end_point;
  height_map_entry_container_t available_traverse;

  HeightMap(std::vector<std::string> map_input, height_map_entry_t entry) : map(map_input) {
    width = map[0].size();
    height = map.size();
    end_point = get_end_position();
    for (map_value_t y = 0; y < height; ++y) {
      for (map_value_t x = 0; x < width; ++x) {
        max_elevation = std::max(map[y][x], max_elevation);
      }
    }
    visited_map = std::make_shared<std::vector<traversed_map_1d_t>>(height, traversed_map_1d_t(width, -1));
    current_position = entry.first;
    (*visited_map)[current_position.second][current_position.first] = entry.second;
    available_traverse.emplace_back(entry.first,entry.second);
  }

  height_map_entry_t get_next_target() {
    auto should_move_check = [&map=map, &visited=visited_map](map_value_t x, map_value_t y, map_value_t target_x, map_value_t target_y) -> bool {
      if (((((map[y][x] - map[target_y][target_x]) >= -1) && (map[target_y][target_x] >= 'a')) || (map[target_y][target_x] == 'S') ) && // is_target_close
          (((*visited)[target_y][target_x] == -1) || // is_target_not_initialized
          ((*visited)[target_y][target_x] > (*visited)[y][x]))) { // is_target_bigger_then_current
        return true;
      }

      bool is_target_e = (map[target_y][target_x] == 'E');

      if (is_target_e) {
        bool is_max_elevation = map[y][x] <= 'z' || map[y][x] >= 'y';
        return is_max_elevation;
      }

      bool is_at_start = map[y][x] == 'S';

      if (is_at_start) {
        bool is_target_first_elevation =  map[target_y][target_x] >= 'a' && map[target_y][target_x] <= 'b' && (((*visited)[target_y][target_x] == -1) || ((*visited)[target_y][target_x] > (*visited)[y][x]));
        return is_target_first_elevation;
      }

      return false;
    };

    auto clean_and_emplace = [](map_value_t target_x, map_value_t target_y, map_value_t next_value, auto &container){
      container.erase(std::remove_if(container.begin(), container.end(), [&](const std::pair<map_coord_t, map_value_t> &v) -> bool {
        return (v.first.first == target_x && v.first.second == target_y && (next_value <= v.second));
      }), container.end());
      container.emplace_back(map_coord_t{target_x,target_y}, next_value);
    };

    map_value_t x = current_position.first;
    map_value_t y = current_position.second;
    map_value_t new_tile_weight = (*visited_map)[y][x] + 1;

    if (x < (width - 1) && should_move_check(x,y,x+1,y)) {
      assert(y != 41);
      clean_and_emplace(x + 1, y, new_tile_weight, available_traverse);
    }

    if (x > 0 && should_move_check(x, y, x-1, y)) {
      clean_and_emplace(x - 1, y, new_tile_weight, available_traverse);
    }

    if (y < (height - 1) && should_move_check(x,y,x,y+1)) {
      clean_and_emplace(x, y + 1, new_tile_weight, available_traverse);
    }

    if (y > 0 && should_move_check(x,y,x,y-1)) {
      clean_and_emplace(x, y - 1, new_tile_weight, available_traverse);
    }

    if (available_traverse.empty()) {
      return  height_map_entry_t{map_coord_t{end_point.first, end_point.second},((*visited_map)[end_point.second][end_point.first])};
    }

    std::sort(available_traverse.begin(), available_traverse.end(), [](const height_map_entry_t& t1, const height_map_entry_t& t2){
        return t1.second > t2.second;
    });

    auto t = available_traverse.back();
    available_traverse.pop_back();

    return t;
  }

  bool traverse_next() {
    height_map_entry_t target = get_next_target();
    current_position = target.first;

    auto x = current_position.first;
    auto y = current_position.second;

    map_value_t next = target.second;

    if ((*visited_map)[y][x] != -1) {
      next = std::min((*visited_map)[y][x], next);
    }
    (*visited_map)[y][x] = next;

    return (map[y][x] == 'E');
  }

  void print_traversed() {
    for (map_value_t y = 0; y < height; ++y) {
      for (map_value_t x = 0; x < width; ++x) {
        std::cout << (((*visited_map)[y][x] >= 0)? '*' : ' ');
      }
      std::cout << "\n";
    }
    std::cout << "\n";
  }

  map_coord_t get_start_position() {
    for (map_value_t y = 0; y < height; ++y) {
      for (map_value_t x = 0; x < width; ++x) {
        if (map[y][x] == 'S') {
          return map_coord_t(x,y);
        }
      }
    }
    return map_coord_t(0,0);
  }

  map_coord_t get_end_position() {
    for (map_value_t y = 0; y < height; ++y) {
      for (map_value_t x = 0; x < width; ++x) {
        if (map[y][x] == 'E') {
          return map_coord_t(x,y);
        }
      }
    }
    return map_coord_t(0,0);
  }

  map_value_t get_steps_to_end() {
    return (*visited_map)[end_point.second][end_point.first];
  }

  [[nodiscard]] map_value_t solve_map() noexcept {

    while (!traverse_next()) { }

    return get_steps_to_end();
  }

  [[nodiscard]] static HeightMap create_map(const FileHelper& file_helper) {
    std::string map_slice;

    auto iss = file_helper.get_stringstream();

    std::vector<std::string> map_input;
    while (std::getline(iss, map_slice)) {
      map_input.push_back(map_slice);
    }

    for (size_t y = 0; y < map_input.size(); ++y) {
      for (size_t x = 0; x < map_input[0].size(); ++x) {
        if (map_input[y][x] == 'S') {
          return HeightMap{map_input, height_map_entry_t{map_coord_t(x,y), 0}};
        }
      }
    }

    return HeightMap{map_input, height_map_entry_t{{0,0},0}};
  }

  [[nodiscard]] static HeightMap create_map(const FileHelper& file_helper, height_map_entry_t entry) {
    std::string map_slice;

    auto iss = file_helper.get_stringstream();

    std::vector<std::string> map_input;
    while (std::getline(iss, map_slice)) {
      map_input.push_back(map_slice);
    }

    return HeightMap{map_input, entry};
  }


};

[[nodiscard]] map_value_t part1(const FileHelper &file_helper) noexcept {
  HeightMap map{HeightMap::create_map(file_helper)};
  auto start_time = std::chrono::system_clock::now();

  auto result = map.solve_map();

  auto end_time = std::chrono::system_clock::now();
  std::chrono::duration<double> diff = end_time - start_time;
  std::cout << "Part1 took: " << std::chrono::duration_cast<std::chrono::milliseconds>(diff) << "\n";

  std::printf("Part1: %d\n", result);

  return result;
}

[[nodiscard]] map_value_t part2(const FileHelper &file_helper) noexcept {
  HeightMap map{HeightMap::create_map(file_helper)};
  height_map_entry_container_t poi;
  for (map_value_t idy = 0; idy < map.height; ++idy) {
    for (map_value_t idx = 0; idx < map.width; ++idx) {
      if (map.map[idy][idx] == 'a') {
        poi.emplace_back(map_coord_t{idx, idy}, 0);
      }
    }
  }

  auto start_time = std::chrono::system_clock::now();

  map_value_t result = std::transform_reduce(std::execution::par, std::cbegin(poi), std::cend(poi), std::numeric_limits<map_value_t>::max(),
      [](map_value_t v1, map_value_t v2) -> map_value_t {
        if (v2 >= 0 && v1 >= 0) {
          return std::min(v1, v2);
        }
        if (v2 >= 0) {
          return v2;
        }
        return v1;
      },
      [&file_helper](const height_map_entry_t& entry) -> map_value_t {
        HeightMap map = HeightMap::create_map(file_helper, entry);

        auto val = map.solve_map();

        return val;
      });

  auto end_time = std::chrono::system_clock::now();
  std::chrono::duration<double> diff = end_time - start_time;
  std::cout << "Part2 took: " << std::chrono::duration_cast<std::chrono::milliseconds>(diff) << "\n";

  std::printf("Part2: %d\n", result);

  return result;
}


int main(void) noexcept {
  {
    std::cout << "Starting day12\n";
    FileHelper file_helper{"input.txt"};

    {
      assert(part1(file_helper) == 481);
    }

    {
      assert(part2(file_helper) == 480);
    }
  }

  return EXIT_SUCCESS;
}
