#include <vector>
#include <fstream>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <string>

using Elf = std::vector<int>;
using ElfCrew = std::vector<Elf>;

std::istream& operator>>(std::istream& is, Elf& elf) {
  std::string number;
  while ( std::getline(is, number, '\n') ) {
    elf.emplace_back(std::stoi(number));
    if (is.peek() == '\n') {
      is.get();
      return is;
    }
  }
  return is;
}

int main(int argc, char* argv[])
{
  std::ifstream fs("elves.dat", fs.in);

  ElfCrew elves;
  Elf temp_elf;

  while (fs >> temp_elf) {
      elves.push_back(temp_elf);
      temp_elf.clear();
  }

  std::sort(elves.begin(), elves.end(), [](auto& elf1, auto& elf2) {
    return std::accumulate(elf1.begin(), elf1.end(), 0) > std::accumulate(elf2.begin(), elf2.end(), 0);
  });

  int top_alfa_elf = std::accumulate(std::begin(*elves.begin()), std::end(*elves.begin()), 0);
  std::cout << "Alfa elf numero uno: " << top_alfa_elf << "\n";

  int total = 0;
  for (auto elf_it = elves.begin(); elf_it != std::next(elves.begin(), 3); elf_it++) {
    total = std::accumulate(elf_it->begin(), elf_it->end(), total);
  }

  std::cout << "Top 3: " << total << "\n";

  return 0;
}
