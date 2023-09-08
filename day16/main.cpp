#include <algorithm>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/regex/v5/error_type.hpp>
#include <boost/regex/v5/match_flags.hpp>
#include <boost/regex/v5/regex_iterator.hpp>
#include <boost/regex/v5/regex_match.hpp>
#include <cassert>
#include <compare>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
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
#include <unordered_map>

class Valve;

using valve_ptr_t = std::shared_ptr<Valve>;
using weak_valve_ptr_t = std::weak_ptr<Valve>;
using valves_t = std::vector<Valve>;
using valve_map_t = std::unordered_map<std::string, valve_ptr_t>;
using weak_valve_map_t = std::unordered_map<std::string, weak_valve_ptr_t>;
using flow_rate_t = std::int32_t;
using duration_t = std::int32_t;
using steps_t = std::int32_t;
using fluid_amount_t = std::int32_t;

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

const boost::regex valve_regex("Valve (?<valve_name>\\w{2}) has flow rate=(?<flow_rate>\\d+); tunnels? leads? to valves? (?<linked_to>(?:\\w{2},? ?)+)");

template<typename K, typename V>
class AssignIterator {
  public:
using iterator_category = std::output_iterator_tag;
using difference_type   = void;
using value_type        = typename std::unordered_map<K,V>::value_type;
using pointer           = typename std::unordered_map<K,V>::value_type*;

  std::unordered_map<K,V>& target_map;

  AssignIterator(std::unordered_map<K,V>& map) :target_map(map) {
  }

  AssignIterator& operator=(const value_type& val){
    target_map[val.first] = val.second;
    return *this;
  }

  AssignIterator& operator=(const value_type&& val){
    target_map[val.first] = val.second;
    return *this;
  }

  AssignIterator& operator++() {
    return *this;
  }

  AssignIterator& operator*() {
    return *this;
  }
};

class Valve {
public:
  std::string name;
  flow_rate_t flow_rate;
  weak_valve_map_t linked_valves;

  Valve(const std::string& valve_name, flow_rate_t valve_flow_rate): name(valve_name), flow_rate(valve_flow_rate) {
  }

  void add_child_valve(weak_valve_ptr_t valve_ptr) {
    auto v = std::shared_ptr<Valve>(valve_ptr);
    linked_valves[v->name] = valve_ptr;
  }

  weak_valve_map_t get_working_valve_targets() {
    weak_valve_map_t working_valves;

    for (auto v_ptr : linked_valves) {
      auto p = std::shared_ptr<Valve>(v_ptr.second);
      if (p->flow_rate > 0) {
        working_valves[p->name] = p;
      }
    }

    return working_valves;
  }

  static valve_map_t build_valves(const FileHelper& file_helper) {
    auto input = file_helper.get_string();
    boost::sregex_iterator cmd_begin(input.begin(), input.end(), valve_regex);
    boost::sregex_iterator cmd_end;

    valve_map_t valves;
    std::transform(cmd_begin, cmd_end, AssignIterator(valves), [](const boost::sregex_iterator::value_type& m) -> valve_map_t::value_type {
      return std::pair(m["valve_name"], std::make_unique<Valve>(m["valve_name"], std::stoi(m["flow_rate"])));
    });

    std::for_each(cmd_begin, cmd_end, [&valves](const boost::sregex_iterator::value_type& m) {
      std::vector<std::string> split_values;
      boost::split(split_values, m["linked_to"], boost::is_any_of(","));
      for (std::string val : split_values) {
        boost::trim(val);
        //std::cout << val << "\n";
        assert(valves.contains(val));
        valves[m["valve_name"]]->add_child_valve(valves[val]);
      }
    });

    return valves;
  }
};

struct TimeIt {
  std::string time_name;
  std::chrono::system_clock::time_point start_time;

  TimeIt(std::string name) :time_name(name) {
    std::cout << "Starting timing " << time_name << "\n";
    start_time = std::chrono::system_clock::now();
  }

  ~TimeIt() {
    auto end_time = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end_time - start_time;

    std::cout << time_name << " took: " << std::chrono::duration_cast<std::chrono::milliseconds>(diff) << "\n";
  }
};

struct Pathfinder {
  using distance_t = std::int32_t;
  struct DistanceWithPtr {
    distance_t distance;
    valve_ptr_t valve;

    /*
    DistanceWithPtr() {
      assert(false);
    }*/

    /*
    DistanceWithPtr(distance_t d, valve_ptr_t v) : distance(d), valve(v) {}

    DistanceWithPtr& operator=(const DistanceWithPtr& d){
      distance = d.distance;
      valve = d.valve;
      return *this;
    }
    */
  };

  using distance_ptr_t = std::shared_ptr<DistanceWithPtr>;
  using distance_map_t = std::unordered_map<std::string, std::shared_ptr<DistanceWithPtr>>;

  using distance_vector_t = std::vector<std::shared_ptr<DistanceWithPtr>>;

  distance_map_t visited;
  distance_vector_t to_traverse;

  Pathfinder(const valve_ptr_t& v) {
    assert(v.get() != nullptr);
    //std::cout << v.get()->name << "\n";
    auto distance_to_start = DistanceWithPtr{0, v};
    assert(distance_to_start.valve.get() != nullptr);
    //std::cout << "Constructor Pathfinder: " << to_traverse.size() << "\n";
    to_traverse.emplace_back(std::make_shared<DistanceWithPtr>(distance_to_start));
  }

  void step() {
    std::sort(to_traverse.begin(), to_traverse.end(), [](const auto& v1, const auto& v2 ){
      return v1->distance > v2->distance;
    });

    auto d = to_traverse.back();
    assert(d->valve.get() != nullptr);
    //std::cout << "Pathfinder::step: " << to_traverse.size() << "\n";

    traverse(d);
  }

  std::shared_ptr<DistanceWithPtr> step_until_found(const valve_ptr_t& v) {
    while (true) {
      if(visited.contains(v->name)) {
        auto temp_dist = visited[v->name];
        return temp_dist;

      }

      assert(to_traverse.size() > 0);

      step();
    }

    assert(false);
  }

  void traverse(const distance_ptr_t& start) {
    for (auto child_ptr : start->valve->linked_valves) {
      auto child = std::shared_ptr<Valve>(child_ptr.second);
      auto distance_to_child = std::make_shared<DistanceWithPtr>(start->distance+1, child);
      to_traverse.emplace_back(distance_to_child);
      assert(distance_to_child.get() != 0 || distance_to_child.get() != nullptr);
      //std::cout << "Pathfinder::traverse: " << to_traverse.size() << "\n";
    }

    visited[start->valve->name] = start;

    to_traverse.erase(std::remove_if(to_traverse.begin(), to_traverse.end(), [name=start->valve->name](const distance_vector_t::value_type& d) -> bool {
      if (d->valve == nullptr) {
        return true;
      }
      return d->valve->name == name;
    }), to_traverse.end());

  }

};

struct hash_pair {
  template <class T1, class T2>
  size_t operator()(const std::pair<T1, T2>& p) const
  {
    auto hash1 = std::hash<T1>{}(p.first);
    auto hash2 = std::hash<T2>{}(p.second);

    if (hash1 != hash2) {
        return hash1 ^ hash2;
    }

    // If hash1 == hash2, their XOR is zero.
      return hash1;
  }
};

using hash_pair_t = std::string;
using cache_map_t = std::unordered_map<hash_pair_t, steps_t>;
cache_map_t cached_distance{};

struct SolverBase {
  valve_map_t valves;
  valve_ptr_t current_valve;
  weak_valve_map_t all_working_valves;

  SolverBase(const FileHelper &file_helper) {
    valves = Valve::build_valves(file_helper);
    current_valve = valves["AA"];
    all_working_valves = get_all_working_valves();
  }

  weak_valve_map_t get_all_working_valves() const {
    weak_valve_map_t working_valves;
    for (auto v_ptr : valves) {
      if (v_ptr.second->flow_rate > 0) {
        working_valves[v_ptr.first] = v_ptr.second;
      }
    }

    return working_valves;
  }

  steps_t get_distance_to(std::shared_ptr<Valve> start, std::shared_ptr<Valve> target) const {
    auto key = hash_pair_t{start->name + target->name};
    if (cached_distance.contains(key)) {
      return cached_distance.at(key);
    }
    Pathfinder p{start};

    auto d = p.step_until_found(target);

    cached_distance[key] = d->distance;
    return d->distance;
  }

  void get_most_yeild() {
    for (auto wv : get_all_working_valves()) {
      auto p = wv.second.lock();
      std::cout << "Distance to: " << p->name << " = "  << get_distance_to(current_valve, p) << " (" << p->flow_rate << ")" << "\n";
    }
  }
};

struct Solver1 : SolverBase{
  struct Runner {
    flow_rate_t flow_rate;
    weak_valve_ptr_t current_valve;
    weak_valve_map_t visited;
    duration_t time_left;
    weak_valve_map_t all_working_valves;
    fluid_amount_t amount = 0;
    Solver1& parent;

    Runner(weak_valve_ptr_t current_valve, weak_valve_map_t all_working_valves, Solver1& p) :parent(p) {
      this->current_valve = current_valve;
      time_left = 30;
      this->all_working_valves = all_working_valves;
      flow_rate = 0;
    }

    flow_rate_t recursive_solver(weak_valve_ptr_t to_visit) {
      std::shared_ptr target(to_visit);
      visited[target->name] = to_visit;
      auto travel_time = parent.get_distance_to(current_valve.lock(), target);
      if (travel_time + 1 > time_left) {
        //std::cout << "Finished: " << (time_left * flow_rate) + amount << "\n";
        return (time_left * flow_rate) + amount;
      }
      //std::cout << "time left: " << time_left << "\n";
      //std::cout << "amount: " << amount << "\n";
      time_left = time_left - (travel_time + 1);
      //std::cout << "time left: " << time_left << "\n";
      amount = amount + ((travel_time + 1) * flow_rate);
      flow_rate = flow_rate + target->flow_rate;
      //std::cout << "amount: " << amount << "\n";
      all_working_valves.erase(target->name);
      current_valve = target;

      if (all_working_valves.size() == 0 || time_left <= 0) {
        return amount;
      }
      flow_rate_t biggest = amount;
      for (auto v: all_working_valves) {
        auto new_ptr = v.second.lock();
        if (!visited.contains(new_ptr->name)) {
          Runner derp(*this);
          biggest = std::max(biggest, derp.recursive_solver(v.second));
        }
      }
      return biggest;

    }
  };

  flow_rate_t operator()() noexcept {
    auto _ = TimeIt("Part1");
    flow_rate_t result = 0;

    //get_most_yeild();

    for (auto v : all_working_valves) {
      Runner r(current_valve, all_working_valves, *this);
      r.parent = *this;

      auto f = r.recursive_solver(v.second);
      result = std::max(f, result);
    }

    std::printf("Part1: %d\n", result);

    return result;
  }

};

struct Solver2 :SolverBase {
  struct Runner2 {
    flow_rate_t flow_rate;
    weak_valve_map_t visited;
    duration_t time_left;
    weak_valve_map_t all_working_valves;
    fluid_amount_t amount = 0;
    Solver2& parent;

    weak_valve_ptr_t first_current_valve;
    weak_valve_ptr_t second_current_valve;

    duration_t first_finished = 0;
    duration_t second_finished = 0;

    Runner2(weak_valve_ptr_t current_valve, weak_valve_map_t all_working_valves, Solver2& p) : parent(p) {
      this->first_current_valve = current_valve;
      this->second_current_valve = current_valve;
      time_left = 26;
      this->all_working_valves = all_working_valves;
      flow_rate = 0;
    }

    flow_rate_t recursive_solver(weak_valve_ptr_t to_visit1, weak_valve_ptr_t to_visit2) {
      std::shared_ptr target(to_visit1);
      visited[target->name] = to_visit1;
      all_working_valves.erase(target->name);

      std::shared_ptr target2(to_visit2);
      visited[target2->name] = to_visit2;
      all_working_valves.erase(target2->name);

      if (first_finished > time_left && second_finished > time_left) {
        //std::cout << "Finished: " << (time_left * flow_rate) + amount << "\n";
        return (time_left * flow_rate) + amount;
      }

      if (!(first_finished == 0) && !(second_finished == 0)) {
      if (first_finished >= second_finished) {
        if (first_finished <= time_left) {
          //std::cout << "First_path: " << first_finished << " " << second_finished << " " << time_left << "\n";
          time_left = time_left - first_finished;
          second_finished = second_finished - first_finished;

          amount += ((first_finished) * flow_rate);
          flow_rate += target->flow_rate;

          first_current_valve = target;
          first_finished = 0;
        }
        if (second_finished <= time_left) {
          //std::cout << "Second_path: " << first_finished << " " << second_finished << " " << time_left << "\n";
          time_left = time_left - second_finished;
          first_finished = std::max(0, first_finished - second_finished);

          amount += ((second_finished) * flow_rate);
          flow_rate += target2->flow_rate;

          second_current_valve = target2;
          second_finished = 0;
        }
      } else if (second_finished >= first_finished) {
        if (second_finished <= time_left) {
          //std::cout << "Second_path: " << first_finished << " " << second_finished << " " << time_left << "\n";
          time_left = time_left - second_finished;
          first_finished = std::max(0, first_finished - second_finished);

          amount += ((second_finished) * flow_rate);
          flow_rate += target2->flow_rate;

          second_current_valve = target2;
          second_finished = 0;
        }
        if (first_finished <= time_left) {
          //std::cout << "First_path: " << first_finished << " " << second_finished << " " << time_left << "\n";
          time_left = time_left - first_finished;
          second_finished = second_finished - first_finished;

          amount += ((first_finished) * flow_rate);
          flow_rate += target->flow_rate;

          first_current_valve = target;
          first_finished = 0;
        }
      }
      }

      if (all_working_valves.size() == 0 && time_left <= 0) {
        return amount;
      }

      flow_rate_t biggest = amount;

      if (first_finished <= 0 || second_finished <= 0) {
        for (auto v: all_working_valves) {
          auto new_ptr = v.second.lock();
          if (!visited.contains(new_ptr->name)) {
            Runner2 derp(*this);
      if (first_finished <= 0) {
        first_finished = parent.get_distance_to(target, v.second.lock()) + 1;
        biggest = std::max(biggest, derp.recursive_solver(v.second, to_visit2));
      } else {
        second_finished = parent.get_distance_to(target2, v.second.lock()) + 1;
        biggest = std::max(biggest, derp.recursive_solver(to_visit1, v.second));
      }
          }
        }
      }


      return biggest;
    }
  };

  flow_rate_t operator()() noexcept {
    auto _ = TimeIt("Part2");
    flow_rate_t result = 0;

    for (auto it = all_working_valves.begin(); it != std::next(all_working_valves.begin(), all_working_valves.size() - 1); ++it) {
      for (auto it2 = std::next(it, 1); it2 != all_working_valves.end(); ++it2) {
        Runner2 r(current_valve, all_working_valves, *this);
        r.parent = *this;
        r.first_finished = get_distance_to(current_valve, (*it).second.lock());
        r.second_finished = get_distance_to(current_valve, (*it2).second.lock());
        r.first_current_valve = (*it).second.lock();
        r.second_current_valve = (*it2).second.lock();
        auto f = r.recursive_solver((*it).second, (*it2).second);
        assert(f <= 2536);
        std::cout << "FIN: " << f << "\n";
        result = std::max(f, result);
      }
    }

    std::printf("Part2: %d\n", result);

    return result;
  }
};


int main(void) noexcept {
  {
    FileHelper file_helper{"input.txt"};
    Solver1 s1{file_helper};
    assert(s1() == 1857);
    Solver2 s2{file_helper};
    assert(s2() == 2536);
  }

  return EXIT_SUCCESS;
}
