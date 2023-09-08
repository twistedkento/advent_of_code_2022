#include <algorithm>
#include <boost/functional/hash.hpp>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <sstream>
#include <string_view>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

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
};

using point_t = std::int16_t;
using coordinates_t = std::pair<point_t, point_t>;
using visited_t = std::unordered_map<coordinates_t, bool, boost::hash<coordinates_t>>;

struct Instruction {
  char direction;
  int steps;
  [[nodiscard]] Instruction(std::string line) noexcept {
    direction = line[0];
    steps = std::stoi(line.substr(2));
  }
};

class Rope {
public:
  std::vector<Instruction> cmds;
  visited_t visited = visited_t{};

  coordinates_t head;
  std::vector<coordinates_t> tails;

  [[nodiscard]] Rope(const std::vector<Instruction> &cmd_input, size_t tail_size) noexcept : cmds(cmd_input) {
    for (size_t i = 0; i < tail_size; ++i) {
      tails.emplace_back(0,0);
    }
  }

  inline void update_tail(const Instruction &instruction) noexcept {
    if (need_to_step(head, tails[0])) {
      tails[0].first = head.first - ((instruction.direction == 'U' ? 1 : 0) + (instruction.direction == 'D' ? -1 : 0));
      tails[0].second = head.second - ((instruction.direction == 'R' ? 1 : 0) + (instruction.direction == 'L' ? -1 : 0));
    }
    update_rest_of_tail();
    visited[tails.back()] = true;
  }

  [[nodiscard]] bool need_to_step(const coordinates_t& target, const coordinates_t& tail) const noexcept {
      return (std::abs(target.first - tail.first) > 1 || std::abs(target.second - tail.second) > 1);
  }

  inline void update_rest_of_tail() noexcept {

    for (size_t idx = 1; idx < tails.size(); ++idx) {
      if (!need_to_step(tails[idx -1], tails[idx])) {
        break;
      }

      singel_step(tails[idx - 1], tails[idx]);
    }
  }

  inline void singel_step(coordinates_t& t1, coordinates_t& t2) noexcept {
  /* t1 = parent
   * t2 = child
   */
    auto get_step_direction = [](point_t t1, point_t t2){
      return ((t1 > t2) ? 1 : -1);
    };

    auto yDist = std::abs(t1.first - t2.first);
    auto xDist = std::abs(t1.second - t2.second);;

    t2.first = yDist >= xDist ? t1.first + get_step_direction(t2.first, t1.first) : t1.first;
    t2.second = xDist >= yDist ? t1.second + get_step_direction(t2.second, t1.second) : t1.second;
  }

  inline void step_head(const Instruction &instruction) noexcept {
      head.first = head.first + ((instruction.direction == 'U' ? 1 : 0) + (instruction.direction == 'D' ? -1 : 0));
      head.second = head.second + ((instruction.direction == 'R' ? 1 : 0) + (instruction.direction == 'L' ? -1 : 0));
  }

  [[nodiscard]] size_t solve() noexcept {
    visited[coordinates_t{0,0}] = true;
    for (auto v : cmds) {
      for (size_t i = 0; static_cast<point_t>(i) < v.steps; ++i) {
        step_head(v);
        update_tail(v);
      }
    }

    return visited.size();
  }

  void draw() const noexcept {
    size_t max_size = 8;

    for (size_t x = max_size; x > 0; --x) {
      for (size_t y = 0; y < max_size; ++y) {
        if (head.first == static_cast<point_t>(x) && head.second == static_cast<point_t>(y)) {
          std::printf("H");
          continue;
        }
        bool found = false;
        for (size_t i = 0; i < tails.size(); ++i) {
          if (tails[i].first == static_cast<point_t>(x) && tails[i].second == static_cast<point_t>(y)) {
            std::printf("%lu",i);
            found = true;
            break;
          }
        }
        if (!found) {
          std::printf("*");
        }
      }
      std::printf("\n");
    }
      std::printf("\n");
  }

  [[nodiscard]] static Rope create_command_map(const FileHelper& file_helper, size_t tail_size) noexcept {
    std::string cmd;

    auto iss = file_helper.get_stringstream();

    std::vector<Instruction> cmd_input;
    while (std::getline(iss, cmd)) {
      cmd_input.emplace_back(cmd);
    }

    return Rope{cmd_input, tail_size};
  }


};

[[nodiscard]] size_t part1(const FileHelper &file_helper) noexcept {
  Rope commands = Rope::create_command_map(file_helper, 1);

  return commands.solve();
}

[[nodiscard]] size_t part2(const FileHelper &file_helper) noexcept {
  Rope commands = Rope::create_command_map(file_helper, 9);

  return commands.solve();
}

int main(void) noexcept {
  {
  FileHelper file_helper{"input.txt"};

  auto result = part1(file_helper);
  std::printf("Part1: %lu\n", result);

  auto result2 = part2(file_helper);
  std::printf("Part2: %lu\n", result2);
  }

  return EXIT_SUCCESS;
}
