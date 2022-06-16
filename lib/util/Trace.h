#ifndef WORLD2D_TRACE
#define WORLD2D_TRACE

#include "Figure.h"
#include "Line.h"

struct Trace {
  Figure* figure;
  XY v;
  Time t;  // figure is depicted at time t

  Trace(Figure* figure_, XY v_, Time t_): figure(figure_), v(v_), t(t_) {}

  BB getBB(Time until) {
    BB bb = figure->getBB();
    return bb|(bb+(v*(until-t)));
  }
  Figure* evolve(Time time) {
    figure->p += v*(time-t);
    t = time;
    return figure;
  }
  
  pair<Time, Time> computeCollision(const Trace* trace) const;
  uint32_t id;
  inline void setId(uint32_t id_) {id = id_;}

  ~Trace() {
    delete figure;
  }
};

inline void updateOutcome(pair<Time, Time>& o1, pair<Time, Time>& o2) {
  if (o2.first!=INVALID_TIME) {
    if (o1.first==INVALID_TIME)
      o1.first = o2.first;
    else
      o1.first = min(o1.first, o2.first);
  }
  if (o2.second!=INVALID_TIME) {
    if (o1.second==INVALID_TIME)
      o1.second = o2.second;
    else
      o1.second = max(o1.second, o2.second);
  }
}

inline pair<Time, Time> Trace::computeCollision(const Trace* trace) const {

  if ((v-trace->v).norm()<EPSILON)
    return make_pair(INVALID_TIME, INVALID_TIME);
  if (figure->type == Figure::CIRCLE && trace->figure->type == Figure::CIRCLE) {
      Time t0 = max(t, trace->t);
      XY p = figure->p - trace->figure->p;
      if (t0 != t)
        p += v*(t0-t);
      if (t0 != trace->t)
        p -= trace->v*(t0-trace->t);
      double r0 = figure->r + trace->figure->r;
      //if (p.norm() < r0-EPSILON) return INVALID_TIME;  // collide at time t0
      XY v0 = v - trace->v;
      // now compute time s.t. norm(p+v0*time)=r0
      // let t denote time for now
      // (p.x+v0.x t)^2 + (p.y+v0.y t)^2 = r0^2
      // (p.x^2+p.y^2) + 2(p.x v0.x+p.y v0.y)t + (v0.x^2+v0.y^2)t^2 = r0^2
      // a t^2 + b y + c = 0, where a = v0.x^2+v0.y^2, b = 2(p.x v0.x+p.y v0.y)
      // and c = p.x^2+p.y^2-r0^2
      double a = v0.normSqr();
      double b = 2 * (p.x*v0.x+p.y*v0.y);
      double c = p.normSqr()-sqr(r0);
      double Delta = sqr(b)-4*a*c;
      if (Delta<DELTA_EPSILON) return make_pair(INVALID_TIME, INVALID_TIME);
      else 
        return make_pair(t0+(-b-sqrt(Delta))/(2*a), t0+(-b+sqrt(Delta))/(2*a));
    }
    
    if (figure->type == Figure::SEGMENT && trace->figure->type == Figure::CIRCLE) {
      auto seg = (Figure::Segment*)figure;
      Trace t1(new Figure::Circle(seg->p+seg->d_p, seg->r), v, t);
      Trace t2(new Figure::Circle(seg->p-seg->d_p, seg->r), v, t);
      pair<Time, Time> outcome(INVALID_TIME, INVALID_TIME); 
      auto tmp1 = t1.computeCollision(trace);
      updateOutcome(outcome, tmp1);
      auto tmp2 = t2.computeCollision(trace);
      updateOutcome(outcome, tmp2);
      double rr = seg->r+trace->figure->r;
      XY p1 = seg->p+seg->n_v*rr, p2 = seg->p-seg->n_v*rr;
      XY pc = trace->figure->p+trace->v*(t-trace->t);
      XY dv = trace->v-v;
      Line l1(p1, seg->t_v), l2(p2, seg->t_v), lc(pc, dv);
      pair<Time, Time> hits[2];
      hits[0] = l1.getCrashTime(lc);
      hits[1] = l2.getCrashTime(lc);
      if (hits[0].first!=INVALID_TIME) {
        uint32_t former = 0;
        if (hits[1].second<hits[0].second)
          former = 1;
        uint32_t latter = 1-former;
        if (abs(hits[former].first)<seg->half_len+EPSILON) {
          if (outcome.first == INVALID_TIME) 
            outcome.first =  t+hits[former].second;
          else
            outcome.first = min(outcome.first, t+hits[former].second);
        }
        if (abs(hits[latter].first)<seg->half_len+EPSILON) {
          if (outcome.second == INVALID_TIME) 
            outcome.second = t+hits[latter].second;
          else
            outcome.second = max(outcome.second, t+hits[latter].second);
        }
      }
      return outcome;
    }

    if (figure->type == Figure::CIRCLE && trace->figure->type == Figure::SEGMENT) {
      return trace->computeCollision(this);
    }

    if (figure->type == Figure::SEGMENT && trace->figure->type == Figure::SEGMENT) {
      // TODO
      return make_pair(INVALID_TIME, INVALID_TIME);
    } 
    return make_pair(INVALID_TIME, INVALID_TIME);
}

typedef unique_ptr<Trace> Trace_uptr;
typedef shared_ptr<Trace> Trace_ptr;

#endif // WORLD2D_TRACE
