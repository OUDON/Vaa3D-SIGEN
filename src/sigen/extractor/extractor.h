#pragma once
#include "sigen/common/binary_cube.h"
#include "sigen/common/cluster.h"
#include "sigen/common/voxel.h"
#include <vector>
#include <boost/utility.hpp>
namespace sigen {
class Extractor : boost::noncopyable {
  void labeling();

public:
  BinaryCube cube_;
  std::vector<std::vector<VoxelPtr> > components_;
  explicit Extractor(const BinaryCube &cube);
  std::vector<ClusterPtr> extract();
};
} // namespace sigen
