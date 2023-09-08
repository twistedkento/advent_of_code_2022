#include <algorithm>
#include <boost/regex.hpp>
#include <boost/regex/v5/error_type.hpp>
#include <boost/regex/v5/match_flags.hpp>
#include <fcntl.h>
#include <iostream>
#include <iterator>
#include <map>
#include <regex>
#include <string_view>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
namespace {
const char *inputMap = nullptr;

[[nodiscard]] auto construct_inventory(std::string_view sv) noexcept {
  std::map<char, std::vector<char>> inventory;
  std::map<char, size_t> position;

  auto startln = sv.find_last_of(']');
  auto endln = sv.find_first_of('\n', startln + 2);
  std::for_each(std::next(sv.begin(), startln + 2),
                std::next(sv.begin(), endln),
                [&inventory, &position, idx = 0](char c) mutable {
                  if (c != ' ') {
                    position[c] = idx;
                    inventory[c] = std::vector<char>{};
                  }
                  ++idx;
                });

  size_t currentPos = 0;
  while (!(++currentPos > (startln))) {
    auto currentBegin = std::next(sv.begin(), currentPos);
    currentPos = sv.find('\n', currentPos);
    std::string_view current_line(currentBegin,
                                  std::next(sv.begin(), currentPos));
    std::for_each(position.begin(), position.end(),
                  [&current_line, &inventory](auto &v) {
                    if (current_line[v.second] != ' ') {
                      inventory[v.first].insert(inventory[v.first].begin(),
                                                current_line[v.second]);
                    }
                  });
  }

  return inventory;
}

void iterate_over_commands(std::string title, std::string_view sv,
                           auto &inventory, auto callback) noexcept {
  auto command_start = sv.find("\n\n");
  std::string command_text(std::next(sv.begin(), command_start + 2), sv.end());
  boost::regex command_regex("move (\\d+) from (\\d+) to (\\d+)");

  boost::sregex_iterator cmd_begin(command_text.begin(), command_text.end(),
                                   command_regex);
  boost::sregex_iterator cmd_end;

  std::for_each(cmd_begin, cmd_end, callback);

  std::printf("%s: ", title.c_str());
  for (auto entity : inventory) {
    std::printf("%c", entity.second.back());
  }
  std::printf("\n");
}

void part1(const char *in) noexcept {
  std::string_view sv{in};
  auto inventory = construct_inventory(sv);

  iterate_over_commands("Part1", sv, inventory, [&](const auto &what) -> bool {
    std::uint8_t amount = std::stoi(what[1].str());
    char from = what[2].str()[0];
    char to = what[3].str()[0];

    for (int i = 0; i < amount; ++i) {
      if (!(inventory.at(from).empty())) {
        char val = inventory[from].back();
        inventory[to].push_back(val);
        inventory[from].pop_back();
      }
    }
    return true;
  });
}

void part2(const char *in) noexcept {
  std::string_view sv{in};
  auto inventory = construct_inventory(sv);

  iterate_over_commands("Part2", sv, inventory, [&](const auto &what) -> bool {
    std::uint8_t amount = std::stoi(what[1].str());
    char from = what[2].str()[0];
    char to = what[3].str()[0];

    if (!(inventory.at(from).empty())) {
      auto begin = std::prev(inventory[from].end(), amount);
      std::move(begin, inventory[from].end(),
                std::back_inserter(inventory[to]));
      inventory[from].erase(begin, inventory[from].end());
    }

    return true;
  });
}

} // namespace

int main(void) noexcept {
  struct stat s;
  int fd = open("input.txt", O_RDONLY);
  fstat(fd, &s);

  inputMap = static_cast<const char *>(
      mmap(nullptr, s.st_size, PROT_READ, MAP_SHARED, fd, 0));

  part1(inputMap);
  part2(inputMap);

  close(fd);

  return 0;
}
