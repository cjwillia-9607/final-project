#ifndef PLANE_H_
#define PLANE_H_

#include "HittableBase.hpp"

namespace GLOO {
class Plane : public HittableBase {
 public:
  Plane(const glm::vec3& normal, float d);
  bool Intersect(const Ray& ray, float t_min, HitRecord& record) const override;
  float d_;
  glm::vec3 normal_;
};
}  // namespace GLOO

#endif
