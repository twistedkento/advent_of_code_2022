#ifndef UTILS_H_WS0AKMPW
#define UTILS_H_WS0AKMPW

#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <chrono>
#include <sstream>

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

#endif /* end of include guard: UTILS_H_WS0AKMPW */
