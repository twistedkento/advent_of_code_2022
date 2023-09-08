#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

using value_t = std::int32_t;
using print_func_t = size_t(size_t, std::int32_t);

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

struct Instruction {
  enum class Op : std::uint8_t {
   noop,
   addx,
  };
  Op op;
  value_t value;
  std::unordered_map<std::string, Op> op_map{{"addx", Op::addx}, {"noop", Op::noop}};
  [[nodiscard]] Instruction(std::string o, std::int32_t val) noexcept {
    op = op_map[o];
    value = val;
  }

  [[nodiscard]]
  static std::vector<Instruction> parse_instructions(std::istream& iss) {
    std::vector<Instruction> op_list;

    while (!iss.eof()) {
      std::string op;
      value_t value = 0;
      iss >> op;

      if (iss.peek() != 10) {
        iss >> value;
      }

      op_list.emplace_back(op, value);

      iss.get();

      if (iss.peek() == -1) {
        break;
      }

    }

    return op_list;
  }
};

[[nodiscard]] size_t print_out_signal_strength(size_t pc, std::int32_t x) noexcept {
  if (pc >= 20 && ((pc % 40) == 20)) {
    std::cout << "pc: " << pc << ", x: " << x << ", value: " << pc * x  << "\n";
    return pc * x;
  }
  return 0;
}

[[nodiscard]] size_t print_out_display(size_t pc, std::int32_t x) noexcept {
  auto step = static_cast<std::int32_t>((pc - 1) % 40);
  if (step == 0 && pc != 1) {
    std::cout << "\n";
  }
  std::cout << ((std::abs(x - step) <= 1) ? "#" : ".");

  return 0;
}

[[nodiscard]] size_t solver(const FileHelper &file_helper, print_func_t print_func) noexcept {
  auto iss = file_helper.get_stringstream();
  auto ops = Instruction::parse_instructions(iss);

  std::int32_t x = 1;
  size_t pc = 1;
  size_t result = 0;

  for (std::uint32_t c = 0; c <= ops.size(); ++c) {
    result += print_func(pc, x);
    switch (ops[c].op) {
      case Instruction::Op::noop:
        ++pc;
        break;
      case Instruction::Op::addx:
        ++pc;
        result += print_func(pc, x);
        ++pc;
        x += ops[c].value;
        break;
    }
  }
  result += print_func(pc, x);

  return result;
}


[[nodiscard]] size_t part1(const FileHelper &file_helper) noexcept {
  size_t result = solver(file_helper, print_out_signal_strength);

  return result;
}

[[nodiscard]] size_t part2(const FileHelper &file_helper) noexcept {
  size_t result = solver(file_helper, print_out_display);

  return result;
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
