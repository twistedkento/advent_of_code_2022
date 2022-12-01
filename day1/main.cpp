#include <vector>
#include <fstream>
#include <iostream>
#include <numeric>
#include <algorithm>

int main(int argc, char* argv[])
{
  std::string filename{"elves.dat"};
  std::ifstream fs(filename, fs.in);
  std::string thing;

  std::vector<std::vector<int>> elves;
  std::vector<int> temp_vector;
  while (fs >> thing)
  {
    temp_vector.emplace_back(std::stoi(thing));
    if  (fs.peek() == '\n')
    {
      fs.get();
      if  (fs.peek() == '\n' || fs.eof())
      {
        elves.push_back(temp_vector);
        temp_vector.clear();
      }
    }
  }


  std::sort(elves.begin(), elves.end(), [](auto& e1, auto& e2) {
      int e1total = std::accumulate(e1.begin(), e1.end(), 0);
      int e2total = std::accumulate(e2.begin(), e2.end(), 0);
      return e1total > e2total;
      });

  int top_alfa_elf = std::accumulate(std::begin(elves.at(0)), std::end(elves.at(0)), 0);
  std::cout << "Alfa elf numero uno: " << top_alfa_elf << "\n";

  int total = 0;
  for (int idx: {0,1,2}) {
    auto& target_elf = elves.at(idx);
    total = std::accumulate(std::begin(target_elf), std::end(target_elf), total);
  }

  std::cout << "Top 3: " << total << "\n";

  return 0;
}
