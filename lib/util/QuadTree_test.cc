
#include "QuadTree.h"

int cnt;
int ANS;
void mohaha(const BB& bb){ANS += 1;}

void test4() {
  const int maxn = 1000;
  const double width = 36000;
  const double height = 36000;
  const int m = 0.00004 * width * height;
  const double padding = 4000;
  
  cout << "Food: " << 0.00004 * 1400 * 800 << endl;

  IdToQuad mp;
  
  auto q = QuadTree(0-padding,0-padding,width+padding,height+padding);
  for (int i = 1; i <= m; ++i ) {
    double x1 = rand01() * width;
    double y1 = rand01() * height;
    double x2 = x1 + 20;
    double y2 = y1 + 20;
    q.insert(BB(x1,y1,x1,y1,i));
  }
  cout << "total count before deletion: " << q.root->sz << endl;  

  auto t1 = getTime();
  for (int j = 0; j < 1000; ++j )
    for (int i = 0; i < maxn; ++i ) {
      double x1 = rand01() * width;
      double y1 = rand01() * height;
      double x2 = x1 + 1400*2;
      double y2 = y1 + 800*2;
      vector<uint32_t> haha;
      haha.reserve(200);
      q.getAllIdIntersectBB(BB(x1,y1,x2,y2), haha);
      cnt += haha.size();
    }
    auto t2 = getTime();
    cout << cnt << " using " << t2-t1 << " (ms)" << endl;
    cout << "ANS = " << ANS << endl;
  }

  int main(int argc, char *argv[]) {
    test4();
  }