#include "sigen/builder/builder.h"
#include "sigen/common/disjoint_set.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <utility>
#include <vector>
#include <queue>
#include <stack>
#include <set>
#include <map>
#include <boost/foreach.hpp>
namespace sigen {
Builder::Builder(const std::vector<boost::shared_ptr<Cluster> > &data,
                 const double scale_xy, const double scale_z)
    : is_radius_computed_(false), data_(data), scale_xy_(scale_xy), scale_z_(scale_z) {}
void Builder::connect_neighbor() {
  std::multimap<IPoint, int> coord_to_index;
  for (int i = 0; i < (int)data_.size(); ++i) {
    BOOST_FOREACH (const IPoint &p, data_[i]->points_) {
      coord_to_index.insert(std::make_pair(p, i));
    }
  }
  for (int i = 0; i < (int)data_.size(); ++i) {
    BOOST_FOREACH (const IPoint &p, data_[i]->points_) {
      for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
          for (int dz = -1; dz <= 1; ++dz) {
            const IPoint q(p.x_ + dx, p.y_ + dy, p.z_ + dz);
            auto range = coord_to_index.equal_range(q);
            for (auto it = range.first; it != range.second; ++it) {
              if (it->second != i) {
                data_[i]->add_connection(data_[it->second].get());
              }
            }
          }
        }
      }
    }
  }
}

void Builder::cut_loops() {
  assert(is_radius_computed_);
  // use kruskal like algorithm
  // see https://en.wikipedia.org/wiki/Kruskal%27s_algorithm
  DisjointSet<Cluster *> U;
  std::vector<std::pair<double, std::pair<Cluster *, Cluster *> > > E;
  BOOST_FOREACH (boost::shared_ptr<Cluster> cls, data_) {
    U.add(cls.get());
    BOOST_FOREACH (Cluster *adj, cls->adjacent_) {
      double strength = (cls->radius_ + adj->radius_) / 2.0;
      Cluster *a = cls.get(), *b = adj;
      if (a < b)
        E.push_back(std::make_pair(strength, std::make_pair(cls.get(), adj)));
    }
  }
  U.setup();
  std::sort(E.begin(), E.end());
  std::reverse(E.begin(), E.end());
  for (const std::pair<double, std::pair<Cluster *, Cluster *> > &it : E) {
    Cluster *a = it.second.first;
    Cluster *b = it.second.second;
    if (U.same(a, b)) {
      a->remove_connection(b);
      b->remove_connection(a);
    } else {
      U.merge(a, b);
    }
  }
}

static NeuronNode *find_edge(NeuronNode *node) {
  std::queue<NeuronNode *> que;
  NeuronNode *last = node;
  for (int i = 0; i < 2; ++i) {
    std::set<NeuronNode *> used;
    que.push(last);
    used.insert(last);
    while (!que.empty()) {
      NeuronNode *cur = que.front();
      que.pop();
      last = cur;
      BOOST_FOREACH (NeuronNode *next, cur->adjacent_) {
        if (used.count(next))
          continue;
        que.push(next);
        used.insert(next);
      }
    }
  }
  return last;
}

void Builder::compute_gravity_point() {
  BOOST_FOREACH (boost::shared_ptr<Cluster> cls, data_) {
    assert(!cls->points_.empty());
    double sx = 0, sy = 0, sz = 0;
    BOOST_FOREACH (const IPoint &p, cls->points_) {
      sx += p.x_;
      sy += p.y_;
      sz += p.z_;
    }
    cls->gx_ = sx / cls->points_.size();
    cls->gy_ = sy / cls->points_.size();
    cls->gz_ = sz / cls->points_.size();
  }
}

void Builder::compute_radius() {
  BOOST_FOREACH (boost::shared_ptr<Cluster> cls, data_) {
    double mdx = 0, mdy = 0, mdz = 0;
    BOOST_FOREACH (const IPoint &p, cls->points_) {
      mdx = std::max(mdx, scale_xy_ * std::abs(p.x_ - cls->gx_));
      mdy = std::max(mdy, scale_xy_ * std::abs(p.y_ - cls->gy_));
      mdz = std::max(mdz, scale_z_ * std::abs(p.z_ - cls->gz_));
    }
    cls->radius_ = std::sqrt(mdx * mdx + mdy * mdy + mdz * mdz);
  }
  is_radius_computed_ = true;
}

std::vector<NeuronNodePtr>
Builder::convert_to_neuron_node(std::vector<boost::shared_ptr<Cluster> > &data,
                                const double scale_xy, const double scale_z) {
  std::vector<NeuronNodePtr> neuron_nodes;
  std::vector<std::pair<int, int> > edges;
  for (int i = 0; i < (int)data.size(); ++i) {
    NeuronNodePtr n = boost::make_shared<NeuronNode>();
    n->coord(data[i]->gx_ * scale_xy,
             data[i]->gy_ * scale_xy,
             data[i]->gz_ * scale_z);
    n->radius_ = data[i]->radius_;
    neuron_nodes.push_back(n);
    for (int j = i + 1; j < (int)data.size(); ++j) {
      if (data[i]->is_connecting_with(data[j])) {
        edges.push_back(std::make_pair(i, j));
      }
    }
  }
  for (std::pair<int, int> item : edges) {
    int a = item.first, b = item.second;
    neuron_nodes[a]->add_connection(neuron_nodes[b].get());
    neuron_nodes[b]->add_connection(neuron_nodes[a].get());
  }
  return neuron_nodes;
}

std::vector<Neuron>
Builder::convert_to_neuron(std::vector<boost::shared_ptr<Cluster> > &data,
                           const double scale_xy, const double scale_z) {
  std::vector<NeuronNodePtr> neuron_nodes =
      convert_to_neuron_node(data, scale_xy, scale_z);
  // split into some neurons
  std::set<NeuronNode *> used;
  std::vector<Neuron> neurons;
  BOOST_FOREACH (NeuronNodePtr node, neuron_nodes) {
    if (used.count(node.get()))
      continue;
    neurons.push_back(Neuron());
    Neuron &n = neurons.back();
    n.root_ = find_edge(node.get());
    n.add_node(node);
    std::stack<NeuronNode *> stk;
    stk.push(node.get());
    used.insert(node.get());
    while (!stk.empty()) {
      NeuronNode *cur = stk.top();
      stk.pop();
      BOOST_FOREACH (NeuronNode *next, cur->adjacent_) {
        if (!used.count(next)) {
          // FIXME TOOOOOO SLOW
          BOOST_FOREACH (NeuronNodePtr inst, neuron_nodes) {
            if (inst.get() == next)
              n.add_node(inst);
          }
          stk.push(next);
          used.insert(next);
        }
      }
    }
  }
  return neurons;
}

static void compute_id_inner(NeuronNode *cur, NeuronNode *prev, int &id) {
  cur->id_ = id++;
  BOOST_FOREACH (NeuronNode *next, cur->adjacent_) {
    if (next != prev) {
      compute_id_inner(next, cur, id);
    }
  }
}

void Builder::compute_id(std::vector<Neuron> &ns) {
  int id = 1;
  for (int i = 0; i < (int)ns.size(); ++i) {
    compute_id_inner(ns[i].root_, NULL, id);
  }
}

static void compute_node_type_inner(NeuronNode *cur, NeuronNode *prev) {
  neuron_type type;
  if (cur->adjacent_.size() >= 3)
    type = neuron_type::BRANCH;
  else if (cur->adjacent_.size() == 2)
    type = neuron_type::CONNECT;
  else
    type = neuron_type::EDGE;
  cur->type_ = type;
  BOOST_FOREACH (NeuronNode *next, cur->adjacent_) {
    if (next != prev) {
      compute_node_type_inner(next, cur);
    }
  }
}

void Builder::compute_node_type(std::vector<Neuron> &neu) {
  for (int i = 0; i < (int)neu.size(); ++i) {
    compute_node_type_inner(neu[i].root_, NULL);
  }
}

std::vector<Neuron> Builder::build() {
  bool print_progress = true;
  compute_gravity_point();
  if (print_progress)
    std::cerr << "compute_gravity_point" << std::endl;
  compute_radius();
  if (print_progress)
    std::cerr << "compute_radius" << std::endl;
  connect_neighbor();
  if (print_progress)
    std::cerr << "connect_neighbor" << std::endl;
  cut_loops();
  if (print_progress)
    std::cerr << "cut_loops" << std::endl;
  std::vector<Neuron> neu = convert_to_neuron(data_, scale_xy_, scale_z_);
  if (print_progress)
    std::cerr << "convert_to_neuron" << std::endl;
  compute_id(neu);
  if (print_progress)
    std::cerr << "compute_id" << std::endl;
  compute_node_type(neu);
  if (print_progress)
    std::cerr << "compute_node_type" << std::endl;
  return neu;
}
} // namespace sigen
