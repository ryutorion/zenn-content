#ifndef VECTOR3_H_INCLUDED
#define VECTOR3_H_INCLUDED

#include <cmath>

struct Vector3
{
    float x;
    float y;
    float z;

    Vector3() noexcept = default;

    constexpr Vector3(float vx, float vy, float vz) noexcept
        : x(vx)
        , y(vy)
        , z(vz)
    {}

    explicit constexpr Vector3(float v) noexcept
        : x(v)
        , y(v)
        , z(v)
    {}

    Vector3 & operator+=(const Vector3 & v) noexcept
    {
        x += v.x;
        y += v.y;
        z += v.z;

        return *this;
    }

    Vector3 & operator-=(const Vector3 & v) noexcept
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;

        return *this;
    }

    Vector3 & operator*=(const float s) noexcept
    {
        x *= s;
        y *= s;
        z *= s;

        return *this;
    }

    Vector3 & operator*=(const Vector3 & v) noexcept
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;

        return *this;
    }

    Vector3 & operator/=(const float s)
    {
        return *this *= (1.0f / s);
    }

    Vector3 & operator/=(const Vector3 & v)
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;

        return *this;
    }

    constexpr float dot(const Vector3 & v) const noexcept
    {
        return x * v.x + y * v.y + z * v.z;
    }

    float length() const
    {
        return std::sqrt(dot(*this));
    }

    inline const Vector3 unit() const;

    constexpr const Vector3 cross(const Vector3 & v) const noexcept
    {
        return Vector3(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }

    inline constexpr const Vector3 projection(const Vector3 & v) const;
    inline constexpr const Vector3 rejection(const Vector3 & v) const;

    inline constexpr const Vector3 reflection(const Vector3 & n) const noexcept;

    inline float distance(const Vector3 & v) const;
};

inline constexpr const Vector3 operator-(const Vector3 & v) noexcept;
inline constexpr const Vector3 operator+(const Vector3 & a, const Vector3 & b) noexcept;
inline constexpr const Vector3 operator-(const Vector3 & a, const Vector3 & b) noexcept;
inline constexpr const Vector3 operator*(const Vector3 & v, const float s) noexcept;
inline constexpr const Vector3 operator*(const float s, const Vector3 & v) noexcept;
inline constexpr const Vector3 operator*(const Vector3 & a, const Vector3 & b) noexcept;
inline constexpr const Vector3 operator/(const Vector3 & v, const float s);
inline constexpr const Vector3 operator/(const Vector3 & a, const Vector3 & b);

//==============================================================================
// Inline functions
//==============================================================================
inline const Vector3 Vector3::unit() const
{
    return *this / length();
}

inline constexpr const Vector3 Vector3::projection(const Vector3 & v) const
{
    return (dot(v) / v.dot(v)) * v;
}

constexpr const Vector3 Vector3::rejection(const Vector3 & v) const
{
    return *this - projection(v);
}

inline constexpr const Vector3 Vector3::reflection(const Vector3 & n) const noexcept
{
    return *this + (-2.0f * dot(n)) * n;
}

inline float Vector3::distance(const Vector3 & v) const
{
    return (v - *this).length();
}

inline constexpr const Vector3 operator-(const Vector3 & v) noexcept
{
    return Vector3(-v.x, -v.y, -v.z);
}

inline constexpr const Vector3 operator+(const Vector3 & a, const Vector3 & b) noexcept
{
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline constexpr const Vector3 operator-(const Vector3 & a, const Vector3 & b) noexcept
{
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline constexpr const Vector3 operator*(const Vector3 & v, const float s) noexcept
{
    return Vector3(v.x * s, v.y * s, v.z * s);
}

inline constexpr const Vector3 operator*(const float s, const Vector3 & v) noexcept
{
    return Vector3(s * v.x, s * v.y, s * v.z);
}

inline constexpr const Vector3 operator*(const Vector3 & a, const Vector3 & b) noexcept
{
    return Vector3(a.x * b.x, a.y * b.z, a.z * b.z);
}

inline constexpr const Vector3 operator/(const Vector3 & v, const float s)
{
    return v * (1.0f / s);
}

inline constexpr const Vector3 operator/(const Vector3 & a, const Vector3 & b)
{
    return Vector3(a.x / b.x, a.y / b.y, a.z / b.z);
}

#endif // VECTOR3_H_INCLUDED
