#include "Animation.h"
#include "Figure.h"

int main() {
  Animation am;
  am.setUntil(20);
  unique_ptr<Figure> f1 = make_unique<Figure::Circle>(XY(5,5), 5);
  unique_ptr<Trace> tr1 = make_unique<Trace>(std::move(f1), XY(0,0), 0);
  unique_ptr<Figure> f2 = make_unique<Figure::Circle>(XY(5,100), 5);
  unique_ptr<Trace> tr2 = make_unique<Trace>(std::move(f2), XY(0,0), 0);
  am[1] = tr1;
  am[2] = tr2;
  unique_ptr<Figure> f3 = make_unique<Figure::Circle>(XY(5,-10), 5);
  unique_ptr<Trace> tr3 = make_unique<Trace>(std::move(f3), XY(0,1), 0);
  am.foreach(tr3.get(), [](uint32_t id, Time t){
    cout << id << "@" << t << endl;
  });

  am.setUntil(200);
  am.foreach(tr3.get(), [](uint32_t id, Time t){
    cout << id << "@" << t << endl;
  });

  am[300] = tr3;
  am.foreach(2, [](uint32_t id, Time t) {
    cout << id << "@" << t << endl;
  });
  am.setUntil(100);
  am.foreach(2, [](uint32_t id, Time t) {
    cout << id << "@" << t << endl;
  });
}