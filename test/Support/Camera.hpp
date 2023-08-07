#pragma once

#include "Core/Core.hpp"

#include "glm/common.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include "glm/vec3.hpp"

struct Rotation {
    f32 head = 0.f;
    f32 pitch = 0.f;
    f32 roll = 0.f;

    explicit constexpr Rotation() noexcept = default;

    explicit constexpr Rotation(f32 h, f32 p, f32 r) noexcept 
        : head(h)
        , pitch(p)
        , roll(r)
    {}

    glm::mat4 get_mat() const noexcept {
        glm::mat4 head_r = glm::rotate(glm::identity<glm::mat4>(), head, glm::vec3(0.f, 1.f, 0.f));
        return glm::rotate(head_r, pitch, glm::vec3(1.f, 0.f, 0.f));
    }

    glm::mat4 get_mat_with_roll() const noexcept {
        auto head_pitch_r = get_mat();
        return glm::rotate(head_pitch_r, roll, glm::vec3(0.f, 0.f, 1.f));
    }
};

struct Camera {
    static constexpr glm::vec3 kForwardVec = glm::vec3(0.f, 0.f, -1.f);
    static constexpr glm::vec3 kUpVec = glm::vec3(0.f, 1.f, 0.f);

    glm::vec3 position = glm::vec3(0.f, 0.f, 0.f);
    glm::vec3 forward = kForwardVec;
    glm::vec3 up = kUpVec;
    Rotation rotation{};

    explicit constexpr Camera() noexcept = default;

    explicit constexpr Camera(glm::vec3 pos, Rotation rot = Rotation{}) noexcept
        : position(pos)
        , forward(kForwardVec)
        , up(kUpVec)
        , rotation(rot)
    {
        forward = calc_forward_vector();
    }

    [[nodiscard]]
    constexpr glm::mat4x4 look_at() noexcept {
        forward = calc_forward_vector();
        up = calc_up_vector();
        return glm::lookAt(position, position + forward, up);
    }

    [[nodiscard]]
    constexpr glm::vec3 calc_forward_vector() const noexcept {
        return glm::vec3(glm::normalize(glm::vec4(kForwardVec, 1.f) * rotation.get_mat()));
    }

    [[nodiscard]]
    constexpr glm::vec3 calc_up_vector() const noexcept {
        return glm::vec3(
            glm::normalize(
                glm::vec4(kUpVec, 1.f) * 
                glm::rotate(glm::identity<glm::mat4>(), rotation.pitch, glm::vec3(1.f, 0.f, 0.f))
            )
        );
    }

    [[nodiscard]]
    constexpr glm::vec3 calc_right_vector() const noexcept {
        return glm::cross(calc_forward_vector(), calc_up_vector());
    }
};