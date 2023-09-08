#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/regex.hpp>
#include <boost/regex/v5/error_type.hpp>
#include <boost/regex/v5/match_flags.hpp>
#include <boost/regex/v5/regex_match.hpp>
#include <boost/type_traits/is_same.hpp>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <regex>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <variant>
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

  [[nodiscard]] std::string get_string() const noexcept {
    return std::string(input_data, s.st_size);
  }
};


struct Monkey;

using num_t = std::uint64_t;
using monkey_id_t = std::string;
using monkey_group_t = std::map<std::string, Monkey>;
num_t factor = 1;

const boost::regex monkey_regex("Monkey (?<monkey_id>\\d+):\r?\n?" \
    "\\s+Starting items: (?<starting_items>(?:\\d+,? ?)+)\r?\n?" \
    "\\s+Operation: new = (?<first_op_statement>old|\\d+) (?<op_modifier>.) (?<second_op_statement>old|\\d+)\r?\n?" \
    "\\s+Test: divisible by (?<test_div_by>\\d+)\r?\n?" \
    "\\s+If true: throw to monkey (?<test_if_true>\\d+)\r?\n?" \
    "\\s+If false: throw to monkey (?<test_if_false>\\d+)\r?\n?");

struct MonkeyOp {
  enum class Op {
   add,
   mul,
  };
  Op op;
  std::variant<bool, num_t> op_target1;
  std::variant<bool, num_t> op_target2;
  std::unordered_map<std::string, Op> op_map{{"*", Op::mul}, {"+", Op::add}};
  MonkeyOp(std::string op_t1, std::string op_type, std::string op_t2) {
    op = op_map[op_type];

    set_op_target_value(op_t1, op_target1);
    set_op_target_value(op_t2, op_target2);
  }

  num_t calc(num_t item) {
    num_t value1 = get_op_target(item, op_target1);;
    num_t value2 = get_op_target(item, op_target2);;

    if (op == Op::add) {
      return value1 + value2;
    }
    if (op == Op::mul) {
      return value1 * value2;
    }
    return value1;
  }

  void set_op_target_value(const std::string &t,  std::variant<bool, num_t> &target) {
    if (t.starts_with("old")) {
      target = true;
    } else {
      target = static_cast<num_t>(std::stoi(t));
    }
  }

  num_t get_op_target(num_t item, std::variant<bool, num_t> &target) {
    num_t value1;
    if (std::holds_alternative<bool>(target)) {
      value1 = item;
    } else {
      value1 = std::get<num_t>(target);
    }
    return value1;
  }
};

struct Monkey {
  monkey_id_t monkey_id;
  std::vector<num_t> items;
  num_t test_div_by;
  std::string test_target_if_true;
  std::string test_target_if_false;
  MonkeyOp calc_op;

  size_t inspected_amount = 0;

  std::weak_ptr<monkey_group_t> parent_group;

  [[nodiscard]] Monkey(const Monkey&) = default;
  [[nodiscard]] Monkey(std::string monkey_nr, std::string start_items, MonkeyOp op, std::string test_div, std::string target_if_true, std::string target_if_false) noexcept : monkey_id(monkey_nr), test_target_if_true(target_if_true), test_target_if_false(target_if_false), calc_op(op) {

    std::string starting_items = start_items;
    std::vector<std::string> split_values;
    boost::split(split_values, start_items, boost::is_any_of(","));

    std::transform(split_values.cbegin(), split_values.cend(), std::back_inserter(items), [](const std::string &v) -> num_t{
      std::string temp_level = v;
      boost::trim(temp_level);
      return static_cast<num_t>(std::stoi(temp_level));
    });

    test_div_by = static_cast<num_t>(std::stoi(test_div));
  }
  void add_parent(std::shared_ptr<monkey_group_t> parent) {
    parent_group = parent;
  }

  void inspect() {
    if (items.size() == 0) {
      return;
    }
    auto p = parent_group.lock();

    for (num_t item: items) {
      //std::cout << "Monkey " << monkey_id << ":\n";
      ++inspected_amount;
      //std::cout << "Monkey inspects an item with a worry level of " << item << "\n";
      auto worry_level = calc_op.calc(item);
      //std::cout << "Worry level: " << calc_op.get_op_target(item, calc_op.op_target1) << " " << ((calc_op.op == MonkeyOp::Op::mul) ? "multiplied" : "increased") << " by " << calc_op.get_op_target(item, calc_op.op_target2) << " to " << worry_level <<  "\n";
      worry_level /= 3;
      //std::cout << "Monkey gets bored with item. Worry level is divided by 3 to " << worry_level << "\n";
      if (worry_level % test_div_by == 0 && worry_level != 0) {
        //std::cout << "Current worry level is divisible by " << test_div_by << "\n";
        p.get()->at(test_target_if_true).items.push_back(worry_level);
        //std::cout << "Item with worry level " << worry_level << " is thrown to monkey " << test_target_if_true << "\n";
      } else {
        //std::cout << "Current worry level is not divisible by " << test_div_by << "\n";
        p.get()->at(test_target_if_false).items.push_back(worry_level);
        //std::cout << "Item with worry level " << worry_level << " is thrown to monkey " << test_target_if_false << "\n";
      }
    }
    items.clear();
  }

  void inspect2() {
    if (items.size() == 0) {
      return;
    }

    auto p = parent_group.lock();

    for (num_t item: items) {
      ++inspected_amount;
      auto worry_level = calc_op.calc(item) % factor;
      if (worry_level % test_div_by == 0) {
        p.get()->at(test_target_if_true).items.push_back(worry_level);
      } else {
        p.get()->at(test_target_if_false).items.push_back(worry_level);
      }
    }
    items.clear();
  }

  [[nodiscard]]
  static std::shared_ptr<monkey_group_t> build_group(const FileHelper& file_helper) {

  std::shared_ptr<monkey_group_t> monkies = std::make_shared<monkey_group_t>();
  auto monkey_input = file_helper.get_string();

  boost::sregex_iterator cmd_begin(monkey_input.begin(), monkey_input.end(),
                                   monkey_regex);
  boost::sregex_iterator cmd_end;

  std::for_each(cmd_begin, cmd_end, [&monkies](const boost::match_results<std::string::const_iterator>& m) {
      MonkeyOp op{m["first_op_statement"], m["op_modifier"], m["second_op_statement"]};
      Monkey mr_monkey = Monkey(m["monkey_id"].str(), m["starting_items"].str(), op, m["test_div_by"].str(), m["test_if_true"].str(), m["test_if_false"].str());
      mr_monkey.add_parent(monkies);
      monkies->emplace(std::make_pair(m["monkey_id"].str(), mr_monkey));
      });

    return monkies;
  }
};


[[nodiscard]] size_t part1(const FileHelper &file_helper) noexcept {
  auto monkies = Monkey::build_group(file_helper);

  for (int idx = 0; idx < 20; ++idx) {
    for (auto& m : *monkies) {
      m.second.inspect();
    }
  }

  std::vector<num_t> amounts;
  std::transform(std::cbegin(*monkies), std::cend(*monkies), std::back_inserter(amounts), [](const auto &v) -> num_t {
    return v.second.inspected_amount;
  });

  std::sort(amounts.begin(), amounts.end(),[](const num_t &a, const num_t &b ) -> bool {
    return a > b;
  });

  return static_cast<size_t>(amounts[0] * amounts[1]);
}

[[nodiscard]] size_t part2(const FileHelper &file_helper) noexcept {
  auto monkies = Monkey::build_group(file_helper);

  factor = std::accumulate(std::begin(*monkies), std::end(*monkies), 1, [](const num_t& init, const auto &m){
    return init * m.second.test_div_by;
  });

  for (int idx = 0; idx < 10000; ++idx) {
    for (auto& m : *monkies) {
      m.second.inspect2();
    }
  }

  std::vector<num_t> amounts;
  std::transform(std::cbegin(*monkies), std::cend(*monkies), std::back_inserter(amounts), [](const auto &v) -> num_t {
    return v.second.inspected_amount;
  });

  std::sort(amounts.begin(), amounts.end(), [](const num_t &a, const num_t &b ) -> bool {
    return a > b;
  });

  return static_cast<size_t>(amounts[0] * amounts[1]);
}


int main(void) noexcept {
  FileHelper file_helper{"input.txt"};

  {
    auto result = part1(file_helper);
    assert(result == 56120);
    std::printf("Part1: %lu\n", result);
  }

  {
    auto result = part2(file_helper);
    assert(result == 24389045529);
    std::printf("Part2: %lu\n", result);
  }

  return EXIT_SUCCESS;
}
