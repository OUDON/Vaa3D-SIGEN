#include "../common/binary_cube.h"
#include "point.h"
#include <vector>
#include <memory>

namespace sigen {
class extractor {
public:
  binary_cube cube_;
  std::vector<std::vector<Point>> clusters_;
  extractor(const binary_cube &cube);
  void labeling();
  void extract();
};
}