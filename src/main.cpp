#include <SFML/Graphics.hpp>

#include "Game.h"
#include <iostream>

int main()
{
    Vec2f x(1.0f,2.0f);
    Vec2f y(2.0f,6.0f);

    std::cout << "Added vecs = " << x+y << std::endl;
    std::cout << "Subtracted vecs = " << x-y << std::endl;
    std::cout << "Divided vecs = " << x/y << std::endl;
    std::cout << "Multiplied vecs = " << x*y << std::endl;
    std::cout << (x==y) << std::endl;
    std::cout << (x==x) << std::endl;
    std::cout << (x!=y) << std::endl;
    std::cout << (x!=x) << std::endl;

    x += y;
    std::cout << "Added vecs = " << x << std::endl;
    x -= y;
    std::cout << "Original vec = " << x << std::endl;
    x *= y;
    std::cout << "Multiplied vecs = " << x << std::endl;
    x /= y;
    std::cout << "Original vec = " << x << std::endl;

    std::cout << "Distance between = " << x.dist(y) << std::endl;
    
    // Game g("config.txt");
    // g.run();
}