#pragma once

#include <SFML/Graphics.hpp>
#include <math.h>

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

    // Math operators on vecs
    Vec2 operator + (const Vec2& rhs) const
    {

    }
    Vec2 operator - (const Vec2& rhs) const
    {
        
    }
    Vec2 operator / (const Vec2& rhs) const
    {
        
    }
    Vec2 operator * (const Vec2& rhs) const
    {
        
    }
    bool operator == (const Vec2& rhs) const
    {

    }
    bool operator != (const Vec2& rhs) const
    {
        
    }
    void operator += (const Vec2& rhs)
    {
        
    }
    void operator -= (const Vec2& rhs)
    {
        
    }
    void operator *= (const Vec2& rhs)
    {
        
    }
    void operator /= (const Vec2& rhs)
    {
        
    }

    // Other convenient calcs
    float dist(const Vec2& rhs) const
    {

    }
};

// Convenient shorthand for common type
using Vec2f = Vec2<float>;