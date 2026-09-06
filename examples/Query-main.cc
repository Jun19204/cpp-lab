#include "Query/Query.h"
#include "rQuery/rQuery.h"

#include <cstdlib>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <file>\n";
    return EXIT_FAILURE;
  }

  std::ifstream infile(argv[1]);

  if (!infile) {
    std::cerr << "Cannot open file: "
              << argv[1] << '\n';
    return EXIT_FAILURE;
  }

  TextQuery text(infile);
  Query q1 =
    Query("fiery") & Query("bird") | Query("wind");

  Query q2 = 
    ~Query("Daddy");

  std::cout << q1 << '\n';
  const auto result1 = q1.eval(text);
  print(std::cout, result1);

  std::cout << q2 << '\n';
  const auto result2 = q2.eval(text);
  print(std::cout, result2);

  return EXIT_SUCCESS;
}
