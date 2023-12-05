#include "Plane.hpp"
#include "iostream"

namespace GLOO {
Plane::Plane(const glm::vec3& normal, float d) {
	d_ = d;
	normal_ = normal;
}

bool Plane::Intersect(const Ray& ray, float t_min, HitRecord& record) const {
  // TODO: Implement ray-plane intersection.
	glm::vec3 R_o = ray.GetOrigin();
	glm::vec3 R_d = glm::normalize(ray.GetDirection());
	if (glm::dot(normal_, R_d) == 0) {
		return false;
	}
	float t = (d_ - glm::dot(normal_, R_o)) / glm::dot(normal_, R_d);
	if (t > t_min && t < record.time){
		record.normal = normal_;
		record.time = t;
		return true;
	}
  return false;
}
}  // namespace GLOO
