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

using crypto_number_t = std::int64_t;
using crypto_entity_t = std::shared_ptr<crypto_number_t>;
using crypto_key_t = std::vector<crypto_entity_t>;


struct SolverBase {
  crypto_key_t crypto_key;

  SolverBase(const FileHelper &file_helper) {
  auto _ = TimeIt("SolverBase");
    crypto_number_t value;
    auto ss = file_helper.get_stringstream();
    while (ss >> value) {
      crypto_key.emplace_back(std::make_shared<crypto_number_t>(value));
    }
    std::cout << "Read all " << crypto_key.size() << " keys\n";
  }
};

struct Solver1 : public SolverBase{
  std::string solver_name = "Part1";
  crypto_number_t operator()() noexcept {
    auto _ = TimeIt(solver_name);
    crypto_number_t result = 0;

    std::printf("%s: %ld\n", solver_name.c_str(), result);

    return result;
  }

};


struct Solver2 :SolverBase {
  std::string solver_name = "Part2";
  crypto_number_t operator()() noexcept {
    auto _ = TimeIt(solver_name);
    crypto_number_t result = 0;

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
