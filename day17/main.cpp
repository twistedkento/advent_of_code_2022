#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "utils.h"
#include <vector>
#include <array>
#include <iterator>
#include <memory>

using cavern_height_t = std::uint64_t;
using cave_point_t = std::int64_t;
using height_t = std::int64_t;
using width_t = std::int64_t;
using rock_particle_t = char;
using cave_t = std::vector<std::string>;

class Rock {
public:
  std::vector<std::string> rock_pattern;
  width_t width = 0;
  height_t height = 0;
  Rock(std::initializer_list<std::string> r) {
      std::copy(r.begin(), r.end(), std::back_inserter(rock_pattern));
      width = rock_pattern[0].size();
      height = rock_pattern.size();
  }

  virtual ~Rock() = default;

  height_t get_height() const {
    return height;
  }

  width_t get_width() const {
    return width;
  }

  void print_info() const {
    for (size_t y = 0; y < rock_pattern.size(); ++y) {
      for (size_t x = 0; x < rock_pattern[y].size(); ++x) {
        std::cout << rock_pattern[y][x];
      }
        std::cout << "\n";
    }

    std::cout << "\n";
  }

};

struct RockPosition {
  cave_point_t x;
  cave_point_t y;

  RockPosition& operator+(const RockPosition& r) {
    x = x + r.x;
    y = y + r.y;

    return *this;
  }
};

struct Cavern {
  enum class Direction : std::int8_t {
    Down,
    Right,
    Left,
  };
  RockPosition position;
  std::shared_ptr<Rock> rock;
  height_t top_rock_position = 0;

  cave_t cave;

  void add_rock(const std::shared_ptr<Rock> r) {
    if (r->get_height() + 3 > top_rock_position) {
      auto to_add = (r->get_height() + 3) - top_rock_position;
      for (size_t idx = 0; idx < to_add; ++idx) {
        cave.insert(cave.begin(), std::string(7, ' '));
        ++top_rock_position;
      }
    }
    rock = r;
    position = RockPosition{2, top_rock_position - (3 + r->get_height())};
  }

  height_t calculate_top_rock_position() const {
    height_t top = 0;
    for (size_t y = std::min(static_cast<size_t>(top_rock_position), cave.size() - 1); y >= 0; --y) {
      for (size_t x = 0; x < cave[0].size(); ++x) {
        if (cave[y][x] == '#') {
          top = y;
          break;
        }
      }
      if (top != static_cast<height_t>(y)) {
        return top;
      }
    }
    return top;
  }


  void place_rock() {
    for (size_t y = 0; y < rock->get_height(); ++y) {
      for (size_t x = 0; x < rock->get_width(); ++x) {
          if ((*rock).rock_pattern[y][x] == '#') {
            cave[y + position.y][x + position.x] = '#';
          }
      }
    }

    top_rock_position = calculate_top_rock_position();
  }

  bool move(Direction d) {
    if (can_move(d)) {
      position = position + get_offset(d);
    }
    if (can_move(Direction::Down)) {
      position = position + get_offset(Direction::Down);
      return true;
    }
    return false;
  }

  bool can_move(Direction d) {
    for (int y = 0; y < rock->get_height(); ++y) {
      for (int x = 0; x < rock->get_width(); ++x) {
        if ((*rock).rock_pattern[y][x] == '#' && !can_move(RockPosition{position.x+x,position.y+y}, d)) {
          return false;
        }
      }
    }
    return true;
  }

  bool can_move(RockPosition p, Direction d) {
    auto offset = get_offset(d);
    cave_point_t target_x = p.x + offset.x;
    cave_point_t target_y = p.y + offset.y;
    if (target_x > 7 || target_x < 0) {
      return false;
    }
    if (static_cast<size_t>(target_y) >= cave.size()) {
      return false;
    }
    if (cave[target_y][target_x] != ' ') {
      return false;
    }
    return true;
  }

  RockPosition get_offset(Direction d) {
    if (d == Direction::Down) {
      return RockPosition{0, 1};
    }
    if (d == Direction::Left) {
      return RockPosition{-1, 0};
    }
    if (d == Direction::Right) {
      return RockPosition{1, 0};
    }
    return RockPosition{0, 1};
  }

  void print_debug() {
    std::cout << "Begin print debug!\n";

    for (int y = 0; y < cave.size(); ++y) {
      std::cout << "|";
      for (int x = 0; x < cave[y].size(); ++x) {
        if ((x >= position.x && x < position.x + rock->get_width()) && (y >= position.y && y < position.y + rock->get_height())) {
          if ((*rock).rock_pattern[y - position.y][x - position.x] == '#') {
            std::cout << "#";
          } else {
            std::cout << " ";
          }

        } else {
          std::cout << cave[y][x];
        }
      }
      std::cout << "|\n";
    }

    std::cout << "End print debug!\n";
  }

  size_t find_repeating() const {
    for (size_t y = cave.size() - 1; y > 0; --y) {
      for (size_t offset = 1; offset < y; ++offset) {
        size_t match_count = 0;
        if (cave[y] == cave[y - offset]) {
          for (size_t idx = 0; idx < offset; ++idx) {
            if (cave[y - idx] == cave[y - offset - idx]) {
              ++match_count;
            } else {
              break;
            }
          }
        if (match_count > 1000) {
          std::cout << "Match: " << match_count << " offset: " << offset << " y: " << y <<  "\n";
          if (match_count == offset) {
            return match_count;
          }
        }
        }
      }
    }
    return 0;
  }

};

struct SolverBase {
  std::vector<std::shared_ptr<Rock>> rocks;
  std::string jet_stream;
  Cavern cave;

  size_t current_rock = 0;
  size_t current_jet_stream = 0;

  Cavern::Direction get_direction() {
    switch (jet_stream[current_jet_stream]) {
      case '<':
        current_jet_stream = (current_jet_stream + 1) % jet_stream.size();
        return Cavern::Direction::Left;
      case '>':
        current_jet_stream = (current_jet_stream + 1) % jet_stream.size();
        return Cavern::Direction::Right;
      default:
        assert(false);
    }
  }

  SolverBase(const FileHelper &file_helper) {
  auto _ = TimeIt("SolverBase");
  rocks.emplace_back(new Rock({
      {std::string{"####"}},
      }));
  rocks.emplace_back(new Rock({
        {std::string{".#."}},
        {std::string{"###"}},
        {std::string{".#."}},
      }));
  rocks.emplace_back(new Rock({
        {std::string{"..#"}},
        {std::string{"..#"}},
        {std::string{"###"}},
      }));
  rocks.emplace_back(new Rock({
        {std::string{"#"}},
        {std::string{"#"}},
        {std::string{"#"}},
        {std::string{"#"}},
      }));
  rocks.emplace_back(new Rock({
        {std::string{"##"}},
        {std::string{"##"}},
      }));

    std::getline(file_helper.get_stringstream(), jet_stream);
    //std::cout << jet_stream << "\n";
    for (const auto& r: rocks) {
      r->print_info();
    }
  }
};

struct Solver1 : public SolverBase{
  std::string solver_name = "Part1";
  cavern_height_t operator()() noexcept {
    constexpr size_t piece_count = 2022;
    auto _ = TimeIt(solver_name);
    height_t result = 0;

    for (size_t idx = 0; idx < piece_count; ++idx) {

      cave.add_rock(rocks[current_rock]);
      //cave.print_debug();

      current_rock = (current_rock + 1) % rocks.size();

      while(true) {
        if (!cave.move(get_direction())) {
          cave.place_rock();
          break;
        }
        //cave.print_debug();
      }
      //cave.print_debug();
    }

    result = cave.cave.size() - cave.calculate_top_rock_position();

    std::printf("%s: %ld\n", solver_name.c_str(), result);

    return result;
  }

};


struct Solver2 :SolverBase {
  std::string solver_name = "Part2";
  cavern_height_t operator()() noexcept {
    constexpr size_t piece_count = 84 + 1113;
    auto _ = TimeIt(solver_name);
    height_t result = 0;
    for (size_t idx = 0; idx < piece_count; ++idx) {
      if (cave.top_rock_position == (cave.cave.size() - 131 - 1) || cave.top_rock_position == (cave.cave.size() - (131 + 2702 - 1))) {
        std::cout << idx << "\n";
      }

      cave.add_rock(rocks[current_rock]);

      current_rock = (current_rock + 1) % rocks.size();

      while(true) {
        if (!cave.move(get_direction())) {
          cave.place_rock();
      if (cave.top_rock_position == 131 || cave.top_rock_position == 131 + 2702) {
        std::cout << idx << "\n";
        std::cin.get();
      }
          break;
        }
      }
    }

    cave.find_repeating();

    constexpr size_t piece_count_2 = 1000000000000;
    size_t kek_val = piece_count_2 - 83;
    size_t kappa_val = (kek_val - (kek_val % 1721)) / 1721;
    std::cout << kappa_val << "\n";
    std::cout << kappa_val * 2702 << "\n";
    result = cave.cave.size() - cave.calculate_top_rock_position();

    std::printf("%s: %ld\n", solver_name.c_str(), result);

    return result;
  }
};


int main(void) noexcept {
  {
    FileHelper file_helper{"input.txt"};
    Solver1 s1{file_helper};
    s1();
    Solver2 s2{file_helper};
    s2();
  }

  return EXIT_SUCCESS;
}
