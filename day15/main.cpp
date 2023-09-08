#include <algorithm>
#include <boost/regex.hpp>
#include <boost/regex/v5/error_type.hpp>
#include <boost/regex/v5/match_flags.hpp>
#include <boost/regex/v5/regex_iterator.hpp>
#include <boost/regex/v5/regex_match.hpp>
#include <cassert>
#include <compare>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <ratio>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <typeinfo>
#include <unistd.h>
#include <variant>
#include <vector>
#include <array>
#include <execution>
#include <chrono>

class Beacon;
class Sensor;

using point_t = std::int64_t;
using coordinate_t = std::pair<point_t, point_t>;
using result_t = size_t;
using min_max_beacons_t = std::pair<point_t, point_t>;
using sensors_t = std::vector<Sensor>;


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

class Beacon {
public:
  point_t x;
  point_t y;
  Beacon(point_t x, point_t y) {
    this->x = x;
    this->y = y;
  }
};

const boost::regex sensor_regex("Sensor at x=(?<sensor_x>-?\\d+), y=(?<sensor_y>-?\\d+): closest beacon is at x=(?<beacon_x>-?\\d+), y=(?<beacon_y>-?\\d+)");

class Sensor {
public:

  point_t x;
  point_t y;

  point_t distance_to_beacon;
  std::shared_ptr<Beacon> beacon_ptr;


  Sensor(coordinate_t sensor, coordinate_t beacon) {
    x = sensor.first;
    y = sensor.second;
    beacon_ptr = std::make_shared<Beacon>(beacon.first, beacon.second);

    distance_to_beacon = calculate_distance_to_position(beacon_ptr->x, beacon_ptr->y);
  }

  point_t calculate_distance_to_position(point_t x2, point_t y2) const {
    return std::abs(x - x2) + std::abs(y - y2);
  }

  static min_max_beacons_t find_min_max_x_beacons(sensors_t sensors) {
    point_t smallest = std::numeric_limits<point_t>::max();
    point_t largest = std::numeric_limits<point_t>::min();
    for ( auto s : sensors) {
      smallest = std::min(smallest, s.beacon_ptr->x);
      largest = std::max(largest, s.beacon_ptr->x);
    }
    return min_max_beacons_t(smallest, largest);
  }

  static min_max_beacons_t find_min_max_sensors_range(sensors_t sensors) {
    point_t smallest = std::numeric_limits<point_t>::max();
    point_t largest = std::numeric_limits<point_t>::min();
    for ( auto s : sensors) {
      smallest = std::min(smallest, s.x - s.distance_to_beacon);
      largest = std::max(largest, s.x + s.distance_to_beacon);
    }
    return min_max_beacons_t(smallest, largest);
  }

  static sensors_t create_sensors(const FileHelper& file_helper) {
    auto input = file_helper.get_string();
    boost::sregex_iterator cmd_begin(input.begin(), input.end(), sensor_regex);
    boost::sregex_iterator cmd_end;

    sensors_t sensors;
    std::transform(cmd_begin, cmd_end, std::back_inserter(sensors), [](const boost::sregex_iterator::value_type& m) -> Sensor{
      return Sensor(coordinate_t{std::stoi(m["sensor_x"]), std::stoi(m["sensor_y"])}, coordinate_t{std::stoi(m["beacon_x"]), std::stoi(m["beacon_y"])});
    });

    return sensors;
  }


};

point_t get_to_end(const sensors_t& sensors,point_t x,point_t y) {
  for (const auto& s : sensors) {
    if (auto current_distance = s.calculate_distance_to_position(x,y); s.distance_to_beacon >= current_distance) {
      point_t current_y_distance = std::abs(s.y-y);
      point_t x_offset = ((s.x+(s.distance_to_beacon - current_y_distance))) - x;
      if (x_offset == 0) {
        return -1;
      }
      return x_offset;
    }
  }
  return 0;
}

point_t loop_da_loop(const sensors_t& sensors, point_t limit) {
  for (point_t y = 0; y <= limit; ++y) {
    for (point_t x = 0; x <= limit; ++x) {

      point_t x_offset = get_to_end(sensors,x,y);

      if (x_offset == 0) {
        return ((x * 4000000) + y);
      }

      if (x_offset >= 0) {
        x += x_offset;
      }
    }
  }
  return 0;
}

result_t part1(const FileHelper &file_helper, const point_t y_value) noexcept {
  std::cout << "Starting part1\n";
  result_t result = 0;
  sensors_t sensors = Sensor::create_sensors(file_helper);

  auto min_max_x = Sensor::find_min_max_sensors_range(sensors);

  auto start_time = std::chrono::system_clock::now();

  for (point_t x = min_max_x.first; x <= min_max_x.second; ++x) {
    if (std::none_of(sensors.cbegin(), sensors.cend(), [x=x,y=y_value](const sensors_t::value_type& s) -> bool {
      return s.beacon_ptr->x == x && s.beacon_ptr->y == y;
    }) && std::any_of(sensors.cbegin(), sensors.cend(), [x=x,y=y_value](const sensors_t::value_type& s) -> bool {
      return s.distance_to_beacon >= s.calculate_distance_to_position(x, y);
    })) {
      ++result;
    }
  }

  auto end_time = std::chrono::system_clock::now();
  std::chrono::duration<double> diff = end_time - start_time;
  std::cout << "Part1 took: " << std::chrono::duration_cast<std::chrono::microseconds>(diff) << "\n";

  std::printf("Part1: %lu\n", result);

  return result;
}

result_t part2(const FileHelper &file_helper, point_t x_and_y_limit) noexcept {
  std::cout << "Starting part2\n";
  result_t result = 0;
  sensors_t sensors = Sensor::create_sensors(file_helper);

  auto start_time = std::chrono::system_clock::now();

  result = loop_da_loop(sensors, x_and_y_limit);

  auto end_time = std::chrono::system_clock::now();
  std::chrono::duration<double> diff = end_time - start_time;

  std::cout << "Part2 took: " << std::chrono::duration_cast<std::chrono::milliseconds>(diff) << "\n";

  std::printf("Part2: %lu\n", result);

  return result;
}



int main(void) noexcept {
  {
    FileHelper file_helper{"input.txt"};
    constexpr point_t part1_y_value = 2000000;
    part1(file_helper, part1_y_value);
    constexpr point_t part2_x_and_y_limit = 4000000;
    part2(file_helper, part2_x_and_y_limit);
  }

  return EXIT_SUCCESS;
}
