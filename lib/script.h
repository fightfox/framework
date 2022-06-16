#ifndef WORLD2D_SCRIPT_H
#define WORLD2D_SCRIPT_H

#include "util/BB.h"
#include "io/Buffer.h"

inline BB getVisionBB(XY p) {
  return BB(p.x-SCREEN_HALF_WIDTH, p.y-SCREEN_HALF_HEIGHT,
          p.x+SCREEN_HALF_WIDTH, p.y+SCREEN_HALF_HEIGHT);
}

inline XY randXY(double len) {
  return XY(rand01()*len*2-len, rand01()*len*2-len);
}

inline unique_ptr<Buffer::Reader> getNicknameBuffer(string name) {
  Buffer::Writer writer;
  writer << (uint8_t)0 << name;
  return std::move(make_unique<Buffer::Reader>(writer.getBufferPtr(), writer.getBufferSize())); 
}

inline XY polarXY(double dir, double r) {
  return XY(r*cos(dir), r*sin(dir));
}

#endif // WORLD2D_SCRIPT_H
