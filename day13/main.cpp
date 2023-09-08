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


class Packets;
class PacketsData;

using packets_value_t = std::int32_t;
using packets_sum_t = std::int32_t;
using packets_pair_t = std::pair<Packets, Packets>;
using packets_pair_container_t = std::vector<std::pair<Packets, Packets>>;
using packets_variant_t = std::variant<packets_value_t, std::shared_ptr<Packets>>;

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

class Packets {
public:
  std::vector<packets_variant_t> data;

  Packets(const packets_value_t &v) {
    data.emplace_back(v);
  }

  Packets(const std::string &packets_data) {
    std::stringstream ss{std::string{std::next(packets_data.begin(), 1), std::prev(packets_data.end(), 1)}};

    while (!ss.eof()) {
      if (ss.peek() >= '0' && ss.peek() <= '9') {
        //packets_variant_t num;
        packets_value_t num;
        ss >> num;
        auto artifact = ss.get();
        if (artifact != ',' && artifact != '[' && artifact != ']' && artifact != -1) {
          assert(false);
        }
        data.push_back(num);
        continue;
      }
      if (ss.peek() == '[') {
        size_t nest = 1;
        char c;
        std::string moo;
        while (true) {
          c = ss.get();
          moo.push_back(c);
          if (c == '[') {
            ++nest;
          }
          if (c == ']') {
            --nest;
          }
          if (nest == 1) {
            data.emplace_back(std::make_shared<Packets>(moo));
            break;
          }
        }
        continue;
      }
      if (ss.peek() == ',') {
        ss.get();
        continue;
      }
      if (ss.eof() || ss.peek() == -1) {
        break;
      }
      assert(false);
    }
  }

  std::strong_ordering operator<=>(const Packets& p1) const {
    size_t len = std::max(p1.data.size(), data.size());
    for (size_t i = 0; i < len; ++i) {
      if (i >= data.size() && !(i >= p1.data.size())) {
        return std::strong_ordering::less;
      }
      if (i >= p1.data.size() && !(i >= data.size())) {
        return std::strong_ordering::greater;
      }
      if (std::holds_alternative<std::shared_ptr<Packets>>(p1.data[i])) {
        auto data_ptr = std::get<std::shared_ptr<Packets>>(p1.data[i]);
        if (std::holds_alternative<std::shared_ptr<Packets>>(data[i])) {
          if (auto cmp = (*std::get<std::shared_ptr<Packets>>(data[i])) <=> (*data_ptr); cmp != 0) {
            return cmp;
          }
        } else {
          Packets temp_package{std::get<packets_value_t>(data[i])};
          if (auto cmp = temp_package <=> (*data_ptr); cmp != 0) {
            return cmp;
          }
        }
      } else if (std::holds_alternative<std::shared_ptr<Packets>>(data[i])) {
        auto data_ptr = std::get<std::shared_ptr<Packets>>(data[i]);
        Packets temp_package{std::get<packets_value_t>(p1.data[i])};
        if (auto cmp =  (*data_ptr) <=> temp_package; cmp != 0) {
          return cmp;
        }
      } else {
        assert(std::holds_alternative<packets_value_t>(data[i]) && std::holds_alternative<packets_value_t>(p1.data[i]));
        if (auto cmp = std::get<packets_value_t>(data[i]) <=> std::get<packets_value_t>(p1.data[i]); cmp != 0) {
          return cmp;
        }
      }
    }

    return std::strong_ordering::equal;
  }

  void print_list() {
    std::cout << "[";
    for (auto &entry : data) {
      if (std::holds_alternative<std::shared_ptr<Packets>>(entry)) {
        auto data_ptr = std::get<std::shared_ptr<Packets>>(entry);
        data_ptr->print_list();
      } else {
        std::cout << std::get<packets_value_t>(entry) << ", ";
      }
    }
    std::cout << "]";

  }

  static packets_pair_container_t create_all_packets(const FileHelper &file_helper) {
    auto packets = packets_pair_container_t{};
    auto stream = file_helper.get_stringstream();
    while (!stream.eof()) {
      std::string first;
      std::getline(stream, first);

      std::string second;
      std::getline(stream, second);

      packets.emplace_back(first, second);

      stream.get();
      stream.peek();
    }

    return packets;
  }
};


packets_sum_t part1(const FileHelper &file_helper) noexcept {
  auto result = 0;
  auto packets = Packets::create_all_packets(file_helper);
  packets_sum_t idx = 0;
  for (packets_pair_t p : packets) {
    ++idx;
    if (p.first < p.second) {
      std::cout << "Winner: ";
      p.first.print_list();
      result += idx;
    } else {
      std::cout << "Winner: ";
      p.second.print_list();
    }
    std::cout << "\n";
  }

  std::printf("Part1: %d\n", result);
  return result;
}

packets_sum_t part2(const FileHelper &file_helper) noexcept {
  auto result = 0;
  auto packets = Packets::create_all_packets(file_helper);
  std::vector<Packets> all_packets;

  for (packets_pair_t p : packets) {
      all_packets.push_back(p.first);
      all_packets.push_back(p.second);
  }
  all_packets.emplace_back(Packets{"[[2]]"});
  all_packets.emplace_back(Packets{"[[6]]"});

  std::sort(all_packets.begin(), all_packets.end());
  for (Packets p : all_packets) {
      p.print_list();
      std::cout << "\n";
  }

  auto key1 = std::find_if(all_packets.begin(), all_packets.end(), [](const Packets& p){
      if (p.data.size() >= 1) {
        if (std::holds_alternative<std::shared_ptr<Packets>>(p.data[0])) {
          auto data_ptr = std::get<std::shared_ptr<Packets>>(p.data[0]);
          if (((*data_ptr).data.size()) == 1 && (std::holds_alternative<packets_value_t>((*data_ptr).data[0]))) {
           return (packets_value_t{2} == std::get<packets_value_t>((*data_ptr).data[0]));
          }
        }
      }
      return false;
    });

  auto key2 = std::find_if(all_packets.begin(), all_packets.end(), [](const Packets& p){
      if (p.data.size() >= 1) {
        if (std::holds_alternative<std::shared_ptr<Packets>>(p.data[0])) {
          auto data_ptr = std::get<std::shared_ptr<Packets>>(p.data[0]);
          if (((*data_ptr).data.size()) == 1 && (std::holds_alternative<packets_value_t>((*data_ptr).data[0]))) {
           return (packets_value_t{6} == std::get<packets_value_t>((*data_ptr).data[0]));
          }
        }
      }
      return false;
    });

  result = ((1+std::distance(all_packets.begin(), key1)) * (1+std::distance(all_packets.begin(), key2)));


  std::printf("Part2: %d\n", result);
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
