#include "Tracer.hpp"

#include <glm/gtx/string_cast.hpp>
#include <stdexcept>
#include <algorithm>

#include "gloo/Transform.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/lights/AmbientLight.hpp"

#include "gloo/Image.hpp"
#include "Illuminator.hpp"

namespace GLOO {
void Tracer::Render(const Scene& scene, const std::string& output_file) {
  scene_ptr_ = &scene;

  auto& root = scene_ptr_->GetRootNode();
  tracing_components_ = root.GetComponentPtrsInChildren<TracingComponent>();
  light_components_ = root.GetComponentPtrsInChildren<LightComponent>();


  Image image(image_size_.x, image_size_.y);


  for (size_t y = 0; y < image_size_.y; y++) {
    for (size_t x = 0; x < image_size_.x; x++) {
      // TODO: For each pixel, cast a ray, and update its value in the image.
        HitRecord record;
        float x_coord = (2.f * ((float)x / image_size_.x)) - 1.f;
        float y_coord = (2.f * ((float)y / image_size_.y)) - 1.f;
        Ray ray = camera_.GenerateRay(glm::vec2(x_coord, y_coord));
        image.SetPixel(x, y, TraceRay(ray, max_bounces_, record));
    }
  }

  if (output_file.size())
    image.SavePNG(output_file);
}


glm::vec3 Tracer::TraceRay(const Ray& ray,
    size_t bounces,
    HitRecord& record) const {
    // TODO: Compute the color for the cast ray.
    bool did_ray_miss = true;
    Material* hit_mat;
    float epsilon = 0.000001f;
    auto modify_dir = ray.GetDirection() + epsilon;
    for (int i = 0; i < tracing_components_.size(); i++)
    {
        auto& trace = *tracing_components_[i];
        auto& hittable = trace.GetHittable();
        glm::mat4 local_to_world = trace.GetNodePtr()->GetTransform().GetLocalToWorldMatrix();
        glm::mat4 world_to_local = glm::inverse(local_to_world);
        Ray local_ray = Ray(ray);
        local_ray.SetDirection(modify_dir);
        local_ray.ApplyTransform(world_to_local);

        bool did_ray_hit = hittable.Intersect(local_ray, 0.0001f, record);
        //std::cout << did_ray_hit << std::endl;
        
        if (did_ray_hit) {
            did_ray_miss = false;
            record.normal = glm::normalize(glm::vec3(glm::inverse(glm::transpose(local_to_world)) * glm::vec4(record.normal, 0.f)));
            hit_mat = &trace.GetNodePtr()->GetComponentPtr<MaterialComponent>()->GetMaterial();
        }

    }

    if (did_ray_miss){
        return GetBackgroundColor(ray.GetDirection());
    }

    const auto hit_point = ray.At(record.time);
    const auto ray_dir = ray.GetDirection();
    const glm::vec3 diffuse_color = hit_mat->GetDiffuseColor();
    const glm::vec3 specular_color = hit_mat->GetSpecularColor();
    const float shininess = hit_mat->GetShininess();
    glm::vec3 E = -glm::normalize(ray_dir);
    const glm::vec3 reflect_eye_ray = glm::normalize(-E + 2.f * (glm::dot(E, record.normal)*record.normal));

    glm::vec3 ret_color = glm::vec3(0.f);
    const glm::vec3 WHITE = glm::vec3(1.f);

    for each (const auto& light in light_components_)
    {
		auto light_type = light->GetLightPtr()->GetType();
        if (light_type == LightType::Ambient) {
            const glm::vec3 ambient_color = light->GetLightPtr()->GetDiffuseColor() * diffuse_color;
            ret_color += ambient_color;
            continue;
        }

        float light_dist;
        glm::vec3 light_dir;
        glm::vec3 light_intensity;
        Illuminator::GetIllumination(*light, hit_point, light_dir, light_intensity, light_dist);

        Ray light_ray(hit_point, light_dir);
        if (glm::dot(light_dir, record.normal) < 0.f) { //implement shadows later
			continue;
		}
        

        if (shadows_enabled_) {
            bool shadow = false;
            for (int i = 0; i < tracing_components_.size(); i++)
            {
				auto& trace = *tracing_components_[i];
				auto& hittable = trace.GetHittable();
				glm::mat4 local_to_world = trace.GetNodePtr()->GetTransform().GetLocalToWorldMatrix();
				glm::mat4 world_to_local = glm::inverse(local_to_world);
				Ray local_ray = Ray(light_ray);
				local_ray.ApplyTransform(world_to_local);

				HitRecord shadow_record;
				bool did_ray_hit = hittable.Intersect(local_ray, 0.0001f, shadow_record);
                if (did_ray_hit) {
					shadow = true;
					break;
				}
			}
            if (shadow) {
                continue;
            }
        }
        float diffuse_clamp = glm::max(glm::dot(record.normal, light_dir), 0.f);
        glm::vec3 diffuse_ret = diffuse_clamp * light_intensity * diffuse_color;
        ret_color += diffuse_ret;
        float specular_clamp = glm::max(glm::dot(reflect_eye_ray, light_dir), 0.f);
        glm::vec3 specular_ret = glm::pow(specular_clamp, shininess) * light_intensity * specular_color;
        ret_color += specular_ret;

        
	}   
    if (bounces > 0) {
		Ray reflect_ray(hit_point, reflect_eye_ray);
        HitRecord reflections;
		ret_color += specular_color * TraceRay(reflect_ray, bounces - 1, reflections);
	}
	return ret_color;
}


glm::vec3 Tracer::GetBackgroundColor(const glm::vec3& direction) const {
  if (cube_map_ != nullptr) {
    return cube_map_->GetTexel(direction);
  } else
    return background_color_;
}
}  // namespace GLOO
