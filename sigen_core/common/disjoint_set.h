// https://www.topcoder.com/community/data-science/data-science-tutorials/disjoint-set-data-structures/
#pragma once
#include <cassert>
#include <vector>
#include <map>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

namespace sigen {

class disjoint_set_internal {
  std::vector<int> data;
  int root(int x);

public:
  explicit disjoint_set_internal(int size);
  int size(int x);
  bool same(int x, int y);
  void merge(int x, int y);
};

template <class T>
class disjoint_set : boost::noncopyable {
  std::map<T, int> forward;
  boost::shared_ptr<disjoint_set_internal> U;

public:
  void add(T x) {
    int next_id = forward.size();
    forward[x] = next_id;
  }

  void setup() {
    U = boost::make_shared<disjoint_set_internal>(forward.size());
  }

  int size(T x) {
    assert((bool)U);
    return U->size(forward[x]);
  }

  bool same(T x, T y) {
    assert((bool)U);
    return U->same(forward[x], forward[y]);
  }

  void merge(T x, T y) {
    assert((bool)U);
    U->merge(forward[x], forward[y]);
  }
};

} // namespace sigen
