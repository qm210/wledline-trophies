#include "DeadlineTrophy.h"

namespace DeadlineTrophy {

    Vec2 Vec2::operator-(const Vec2& other) const {
        return {x - other.x, y - other.y};
    }

    Vec2 Vec2::operator+(const Vec2& other) const {
        return {x + other.x, y + other.y};
    }

    Vec2 Vec2::operator*(float factor) const {
        return {factor * x, factor * y};
    }

    Vec2 operator*(float factor, const Vec2& vec) {
        return vec * factor;
    }

    float Vec2::dot(const Vec2& a, const Vec2 b) {
        return a.x * b.x + a.y * b.y;
    }

    float Vec2::length() const {
        return sqrtf(x * x + y * y);
    }

    Vec2& Vec2::shift(float dx, float dy) {
        x += dx;
        y += dy;
        return *this;
    };

    Vec2& Vec2::rotate(float phi) {
        float c = cos_t(phi);
        float s = sin_t(phi);
        float rotX = c * x - s * y;
        float rotY = s * x + c * y;
        x = rotX;
        y = rotY;
        return *this;
    }

    Vec2& Vec2::scale(float factorX, float factorY) {
        x *= factorX;
        y *= factorY;
        return *this;
    }

    float Vec2::distance(const Vec2& other) const {
        return (other - (*this)).length();
    }

    Vec2 Vec2::fromParameters(uint8_t x, uint8_t y) {
        // will then reach from [-0.5, +0.496] on both axes
        const float inv256 = 3.906e-3;
        return {
            (static_cast<float>(x) - 128.f) * inv256,
            (static_cast<float>(y) - 128.f) * inv256
        };
    }

    float Coord::sdLine(float x1, float y1, float x2, float y2) const {
            float px = uv.x - x1;
            float py = uv.y - y1;
            float dx = x2 - x1;
            float dy = y2 - y1;
            float h = constrain((px*dx + py*dy) / (dx*dx + dy*dy), 0.f, 1.f);
            float lx = px - dx * h;
            float ly = py - dy * h;
            return sqrtf(lx*lx + ly*ly);
    }

    float Coord::sdLine(Vec2 p1, Vec2 p2) const { // QM TOMAYBEDO: for now, implement both, but check the footprint later
        Vec2 p = uv - p1;
        Vec2 d = p2 - p1;
        float h = constrain(Vec2::dot(p,d) / Vec2::dot(d,d), 0.f, 1.f);
        Vec2 l = p - h * d;
        return l.length();
    }

}