#include "Triangle.hpp"

#include <iostream>
#include <stdexcept>

#include <glm/common.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Plane.hpp"

namespace GLOO {
Triangle::Triangle(const glm::vec3& p0,
                   const glm::vec3& p1,
                   const glm::vec3& p2,
                   const glm::vec3& n0,
                   const glm::vec3& n1,
                   const glm::vec3& n2) {
    positions_.push_back(p0);
    positions_.push_back(p1);
    positions_.push_back(p2);
    normals_.push_back(n0);
    normals_.push_back(n1);
    normals_.push_back(n2);
}

Triangle::Triangle(const std::vector<glm::vec3>& positions,
                   const std::vector<glm::vec3>& normals) {
    positions_ = positions;
    normals_ = normals;
}

bool Triangle::Intersect(const Ray& ray, float t_min, HitRecord& record) const {
  // TODO: Implement ray-triangle intersection.
    glm::vec3 R_o = ray.GetOrigin();
    glm::vec3 R_d = ray.GetDirection();
    glm::vec3 a = positions_[0];
    glm::vec3 b = positions_[1];
    glm::vec3 c = positions_[2];

    glm::mat3 A = glm::mat3(a - b, a - c, R_d);
    glm::vec3 B = glm::vec3(a - R_o);

    glm::vec3 x = glm::inverse(A) * B;
    float beta = x.x;
    float gamma = x.y;
    if (beta < 0 || gamma < 0 || beta + gamma > 1) {
		return false;
	}
    float alpha = 1 - beta - gamma;
    float t = x.z;
    if (t > t_min && t < record.time) {
        record.time = t;
        record.normal = alpha * normals_[0] + beta * normals_[1] + gamma * normals_[2];
        return true;
    }
  return false;
}
}  // namespace GLOO
