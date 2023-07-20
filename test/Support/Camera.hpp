#pragma once

#include "Core/Core.hpp"

#include "glm/common.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include "glm/vec3.hpp"

struct Camera {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 right;
    glm::vec3 up;

    explicit constexpr Camera() noexcept = default;

    explicit constexpr Camera(glm::vec3 pos, glm::vec3 dir, glm::vec3 right_vec, glm::vec3 up_vec) noexcept
        : position(pos)
        , direction(dir)
        , right(right_vec)
        , up(up_vec)
    {}

    [[nodiscard]]
    constexpr glm::mat4x4 look_at(glm::vec3 target_point) const noexcept {
        return glm::lookAt(position, target_point, up);
    }

    /*
    * Calculates Camera's direction and right vector from given target point.
    */
    static constexpr Camera from_target_point(glm::vec3 pos, glm::vec3 target_point) noexcept {
        constexpr glm::vec3 kUp = glm::vec3(0.f, 1.f, 0.f);
        glm::vec3 direction = pos - target_point;
        glm::vec3 right_vec = glm::normalize(glm::cross(kUp, direction));
        glm::vec3 up_vec = glm::normalize(glm::cross(direction, right_vec));

        return Camera(pos, direction, right_vec, up_vec);
    }
};