#include <cstdio>
#include <cstdlib>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    return EXIT_FAILURE;
  }
  if (::setenv("PATH", argv[1], 1) == -1) {
    std::perror("setenv");
    return EXIT_FAILURE;
  }

  ::execlp("process_lifecycle", "process_lifecycle",
           static_cast<char *>(nullptr));
  std::perror("execlp");
  return EXIT_FAILURE;
}
