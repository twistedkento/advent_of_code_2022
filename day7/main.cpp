#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>

constexpr size_t filesystem_size = 70000000;

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

  ~FileHelper() {
    if (fd >= 0) {
      close(fd);
    }
  }

  [[nodiscard]] std::istringstream get_stringstream() const {
    return std::istringstream{std::string(input_data, s.st_size),
                              std::ios_base::in};
  }
};

class ElfCrappyFileSystem
    : public std::enable_shared_from_this<ElfCrappyFileSystem> {
public:
  std::weak_ptr<ElfCrappyFileSystem> parent_ptr;

  std::shared_ptr<ElfCrappyFileSystem> get_ptr() { return shared_from_this(); }

  [[nodiscard]] static std::shared_ptr<ElfCrappyFileSystem> create() {
    return std::shared_ptr<ElfCrappyFileSystem>(new ElfCrappyFileSystem());
  }

  [[nodiscard]] static std::shared_ptr<ElfCrappyFileSystem>
  create(std::shared_ptr<ElfCrappyFileSystem> fs) {
    return std::shared_ptr<ElfCrappyFileSystem>(new ElfCrappyFileSystem(fs));
  }

  void add_child(std::string name) {
    if (!children.contains(name)) {
      children[name] = ElfCrappyFileSystem::create(shared_from_this());
    }
  }

  void add_file(std::string name, size_t size) {
    if (!files.contains(name)) {
      files[name] = size;
    }
  }

  void print(size_t indent) {
    for (auto v : children) {
      std::cout << std::setw(indent) << "|+" << v.first << "\n";
      v.second->print(indent + 2);
    }
    for (auto v : files) {
      std::cout << std::setw(indent) << "|-" << v.first << " " << v.second
                << "\n";
    }
  }

  std::unordered_map<std::string, std::shared_ptr<ElfCrappyFileSystem>>
  get_all_directories() {
    std::unordered_map<std::string, std::shared_ptr<ElfCrappyFileSystem>>
        result;

    for (auto v : children) {
      assert(!result.contains(v.first));
      result[v.first] = v.second;
      for (auto v2 : v.second->get_all_directories()) {
        assert(!result.contains(v.first + "/" + v2.first));
        result[v.first + "/" + v2.first] = v2.second;
      }
    }

    return result;
  }

  size_t acc() {
    size_t total = 0;
    for (auto v : children) {
      total += v.second->acc();
    }
    for (auto v : files) {
      total += v.second;
    }
    return total;
  }

  std::unordered_map<std::string, std::shared_ptr<ElfCrappyFileSystem>>
      children;
  std::unordered_map<std::string, size_t> files;

  [[nodiscard]] static std::shared_ptr<ElfCrappyFileSystem>
  constuct_filesystem(const FileHelper &file_helper) {
    std::string cmd;

    auto iss = file_helper.get_stringstream();

    auto root = ElfCrappyFileSystem::create();
    std::shared_ptr<ElfCrappyFileSystem> current{root->get_ptr()};

    while (std::getline(iss, cmd)) {
      if (cmd[0] == '$') {
        if (cmd.starts_with("$ cd /")) {
          current = root;
        } else if (cmd.starts_with("$ cd ")) {
          std::string folder_name(std::next(cmd.begin(), 5), cmd.end());
          if (folder_name == "..") {
            if (auto temp_current = current.get()->parent_ptr.lock()) {
              current = temp_current;
            }
          } else {
            if (current->children.contains(folder_name)) {
              current = current->children[folder_name];
            }
          }
        } else if (cmd.starts_with("$ ls")) {
          while (iss.peek() != '$' && !iss.eof()) {
            if (iss.peek() == 'd') {
              std::getline(iss, cmd);
              current->add_child(
                  std::string(std::next(cmd.begin(), 4), cmd.end()));
            } else {
              size_t test;
              std::string filename;
              iss >> test >> filename;
              iss.get();
              current->add_file(filename, test);
            }
          }
        }
      }
    }
    return root;
  }

private:
  ElfCrappyFileSystem() = default;
  ElfCrappyFileSystem(std::shared_ptr<ElfCrappyFileSystem> fs) {
    parent_ptr = std::weak_ptr<ElfCrappyFileSystem>(fs);
  }
};

[[nodiscard]] size_t part1(const FileHelper &file_helper) noexcept {
  auto root = ElfCrappyFileSystem::constuct_filesystem(file_helper);

  auto all = root->get_all_directories();
  size_t total = 0;

  for (auto v : all) {
    size_t vtotal = v.second->acc();
    if (vtotal <= 100000) {
      total += vtotal;
    }
  }

  return total;
}

[[nodiscard]] size_t part2(const FileHelper &file_helper) noexcept {
  auto root = ElfCrappyFileSystem::constuct_filesystem(file_helper);

  auto all = root->get_all_directories();

  size_t target = filesystem_size - root->acc();

  target = 30000000 - target;

  size_t new_tot = filesystem_size;
  for (auto v : all) {
    size_t vtotal = v.second->acc();
    if (vtotal >= target) {
      new_tot = std::min(vtotal, new_tot);
    }
  }

  return new_tot;
}

int main(void) noexcept {
  FileHelper file_helper{"input.txt"};

  auto result1 = part1(file_helper);
  assert(result1 == 1582412);
  std::printf("Part1: %lu\n", result1);

  auto result2 = part2(file_helper);
  assert(result2 == 3696336);
  std::printf("Part2: %lu\n", result2);

  return EXIT_SUCCESS;
}
