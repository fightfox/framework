#ifndef WORLD2D_QUADTREE
#define WORLD2D_QUADTREE

#include "BB.h"

struct QuadNode;
typedef dense_hash_map<uint32_t, QuadNode*> IdToQuad;

struct QuadNode {
  static const int MAX_LEVEL = 15;
  static const int MAX_SIZE_PER_NODE = 32;
  BB box;
  QuadNode* parent;
  QuadNode* childs[4];
  int level;
  bool isLeaf;
  double x, y; // the center of box
  vector<BB> content; // content.size()<= MAX_SIZE_PER_NODE
  IdToQuad& mp;
  int sz;
  
  QuadNode(const BB& bb, QuadNode* parent, int level, IdToQuad& mp) 
  : box(bb), parent(parent), level(level), mp(mp) {
    x = (box.x1+box.x2)/2;
    y = (box.y1+box.y2)/2;
    isLeaf = true;
    sz = 0;
    childs[0] = childs[1] = childs[2] = childs[3] = nullptr;
  }
  void insert(const BB& bb) {
    ++sz; 
    if (isLeaf && content.size() < MAX_SIZE_PER_NODE) {
      content.push_back(bb);
      mp[bb.id] = this;
    }
    else {
      if (isLeaf && level < MAX_LEVEL) {
        // split
        isLeaf = false;
        childs[0] = new QuadNode(BB(box.x1,box.y1,x,y),this,level+1,mp);
        childs[1] = new QuadNode(BB(x,box.y1,box.x2,y),this,level+1,mp);
        childs[2] = new QuadNode(BB(box.x1,y,x,box.y2),this,level+1,mp);
        childs[3] = new QuadNode(BB(x,y,box.x2,box.y2),this,level+1,mp);
        int tmp = 0;
        for (int i = 0; i < content.size(); ++i ) {
          bool flag = false;
          for (int j = 0; j < 4; ++j )
            if (childs[j]->box.contains(content[i])) {
              childs[j]->insert(content[i]);
              flag = true;
              break;
            }
          if (flag) continue;
          content[tmp++] = content[i];
        }
        content.resize(tmp);
      }
      if (!isLeaf) {
        for (int i = 0; i < 4; ++i )
          if (childs[i]->box.contains(bb)) {
            childs[i]->insert(bb);
            return;
          }
      }
      content.push_back(bb);
      mp[bb.id] = this;
    } 
  }
  // must be non-leaf
  void _clean() {
    --sz;
    if (sz == content.size() && !isLeaf) {
      for (int i = 0; i < 4; ++i ) {
        delete childs[i];
        childs[i] = nullptr;
      }
      isLeaf = true;
    }
    if (parent != nullptr) parent->_clean();
  }
  void remove(uint32_t id) {
    QuadNode* quad = mp[id];
    if (quad == nullptr) {
      cerr << "item " << id << " not found\n";
      return;
    } 
    auto& c = quad->content;
    for (int i = 0; i < c.size(); ++i )
      if (c[i].id == id) {
        c[i] = c[c.size()-1];
        c.pop_back();
        mp.erase(id);
        break;
      }
    quad->_clean();
  }
  void foreach(const BB& bb, const function<void(const BB&)>& fn) {
    for (const auto& item : content)
      if (intersect(bb,item)) {fn(item);}
    if (!isLeaf) 
      for (int i = 0; i < 4; ++i )
        if (intersect(bb, childs[i]->box))
          childs[i]->foreach(bb, fn);
  }
  void getAllIdIntersectBB(const BB& bb, vector<uint32_t>& v) const {
    for (const auto& item : content)
      if (intersect(bb,item)) v.push_back(item.id);
    if (!isLeaf) 
      for (int i = 0; i < 4; ++i )
        if (intersect(bb, childs[i]->box))
          childs[i]->getAllIdIntersectBB(bb, v);
  }
};

struct QuadTree {
  unique_ptr<QuadNode> root;
  IdToQuad mp;  
  QuadTree(const BB& bb) {
    mp.set_empty_key(-1);
    mp.set_deleted_key(-2);
    root = std::move(make_unique<QuadNode>(bb, nullptr, 0, mp));
  }
  QuadTree(double x1, double y1, double x2, double y2) {
    mp.set_empty_key(-1);
    mp.set_deleted_key(-2);
    root = std::move(make_unique<QuadNode>(BB(x1,y1,x2,y2), nullptr, 0, mp));
  }
  inline void insert(const BB& bb) { root->insert(bb); }
  inline void remove(uint32_t id) { root->remove(id); }
  inline void foreach(const BB& bb, const function<void(const BB&)>& fn) {
    root->foreach(bb, fn);
  }
  inline void getAllIdIntersectBB(const BB& bb, vector<uint32_t>& v) const {
    root->getAllIdIntersectBB(bb, v);
  }
};

#endif // WORLD2D_QUADTREE

// 3.6 ms points
/*
g++ -std=c++11 -o2 UnitManagerIO.cpp -o quad
valgrind --tool=callgrind ./quad
qcachegrind
callgrind_annotate --auto=yes >a.txt; nano a.txt
*/