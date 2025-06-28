#pragma once

#include <SFML/Graphics.hpp>
#include <math.h>
#include <ostream>

template <typename T>
class Vec2
{
public:
    T x = 0;
    T y = 0;

    Vec2() = default;

    Vec2(T xin, T yin)
        : x(xin), y(yin) { }

    // Convenient conversion from SFML 2D vector
    Vec2(const sf::Vector2<T>& vec)
        : x(vec.x), y(vec.y) { }

    // Allows us to pass this into SFML functions 
    // and tells it how to convert to type correctly
    operator sf::Vector2<T>()
    {
        return sf::Vector2<T>(x, y);
    }

    // Math operators on vecs - we're going to assume that the multiply and divides
    // are simply elementwise, and not dot product or something.

    Vec2 operator + (const Vec2& rhs) const
    {
        return Vec2(x+rhs.x, y+rhs.y);
    }
    Vec2 operator - (const Vec2& rhs) const
    {
        return Vec2(x-rhs.x, y-rhs.y);
    }
    Vec2 operator / (const Vec2& rhs) const
    {
        return Vec2(x/rhs.x, y/rhs.y);
    }
    Vec2 operator * (const Vec2& rhs) const
    {
        return Vec2(x*rhs.x, y*rhs.y);
    }
    bool operator == (const Vec2& rhs) const
    {
        return (x == rhs.x) && (y == rhs.y);
    }
    bool operator != (const Vec2& rhs) const
    {
        return (x != rhs.x) || (y != rhs.y);
    }
    void operator += (const Vec2& rhs)
    {
        x += rhs.x;
        y += rhs.y;
    }
    void operator -= (const Vec2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
    }
    void operator *= (const Vec2& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
    }
    void operator /= (const Vec2& rhs)
    {
        x /= rhs.x;
        y /= rhs.y;
    }

    // Convenient math operators with scalars

    Vec2 operator + (const T rhs) const
    {
        return Vec2(x+rhs, y+rhs);
    }
    Vec2 operator - (const T rhs) const
    {
        return Vec2(x-rhs, y-rhs);
    }
    Vec2 operator / (const T rhs) const
    {
        return Vec2(x/rhs, y/rhs);
    }
    Vec2 operator * (const T rhs) const
    {
        return Vec2(x*rhs, y*rhs);
    }
    void operator += (const T rhs)
    {
        x += rhs; y += rhs;
    }
    void operator -= (const T rhs)
    {
        x -= rhs; y -= rhs;
    }
    void operator *= (const T rhs)
    {
        x *= rhs; y *= rhs;
    }
    void operator /= (const T rhs)
    {
        x /= rhs; y /= rhs;
    }

    // Distance between two vectors (i.e. points)
    float dist(const Vec2& rhs) const
    {
        return sqrtf( ((x-rhs.x)*(x-rhs.x)) + ((y-rhs.y)*(y-rhs.y)) );
    }

    // Normalise to unit vector
    Vec2 unit_vector() const
    {
        T magnitude = sqrt(x*x + y*y);

        if (magnitude == 0) { return Vec2(0,0); }

        return Vec2(x/magnitude, y/magnitude);
    }

    // Convenient print
    // Just declaring it here, but must be defined out of scope of class
    // Using 'friend' makes so that there's no implicit 'this' argument,
    // kind of like it's not a member function.
    // Technically we can just omit this declaration here and only have
    // the definition outside because it's just printing public members,
    // but good practice and future proofing.
    template <typename U>
    friend std::ostream& operator << (std::ostream& out, const Vec2<U>& vec);
};

// Convenient shorthand for common type
using Vec2f = Vec2<float>;

// Vector pretty printing
template <typename T>
std::ostream& operator << (std::ostream& out, const Vec2<T>& vec)
{
    return out << "[" << vec.x << "," << vec.y << "]";
}