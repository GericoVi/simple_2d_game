#include "Game.h"

#include <iostream>

Game::Game(const std::string& config) { init(config); }

void Game::init(const std::string& path)
{
    // TODO: read in config file here
    //       place data into structs

    // setup window
    m_window.create(sf::VideoMode(1280, 720), "Assignment 2");
    m_window.setFramerateLimit(60);

    ImGui::SFML::Init(m_window);
    ImGui::GetStyle().ScaleAllSizes(2.0f);
    ImGui::GetIO().FontGlobalScale = 2.0f;

    spawnPlayer();
}

EntityPtr Game::player()
{
    auto& players = m_entities.getEntities("player");
    return players.front();     // should always be only one player
}

void Game::run()
{
    // TODO: add pause functionality in here
    //       some systems should function while paused (e.g. rendering)
    //       some shouldn't (e.g. movement/input)
    while (m_running)
    {
        m_entities.update();

        ImGui::SFML::Update(m_window, m_deltaClock.restart());

        // run systems
        sEnemySpawner();
        sMovement();
        sCollision();
        sUserInput();
        sGUI();
        sRender();

        // increment current frame (how is this affected by pause since used to decide when to spawn enemies?)
        m_currentFrame++;
    }
}

void Game::setPaused(bool paused)
{
    // TODO
}

void Game::spawnPlayer()
{
    // TODO: add all properties of the player from the config

    auto entity = m_entities.addEntity("player");

    // Spawn player by 'giving' (reinitialising) the transform component
    entity->add<CTransform>(Vec2f(200.0f, 200.0f), Vec2f(1.0f, 1.0f), 0.0f);

    // Give shape for rendering
    entity->add<CShape>(32.0f, 8, sf::Color(10, 10, 10), sf::Color(255, 0, 0), 4.0f);

    // For controls
    entity->add<CInput>();
}

void Game::spawnEnemy()
{
    // TODO: use m_enemyConfig when spawning, and be withing the bounds of window.

    m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(EntityPtr entity)
{
    // TODO: spawn small enemeis at the location of the input entity

    // when we create the smaller enemies, we have to read the values of the original e
    // - spawn enemies equal to number of vertices
    // - small ones have same colour but half the size
    // - they're worth double points
}

void Game::spawnBullet(EntityPtr entity, const Vec2f& target)
{
    // TODO: - bullet speed is given in config as scalar
    //       - set 2D vector via speed and angle obtained from entity to target
}

void Game::spawnSpecial(EntityPtr entity)
{
    // TODO
}

void Game::sMovement()
{
    // TODO: entity movement using cTransform (and cInput for player)

    // sample, before vec2 properly implemented
    auto& transform = player()->get<CTransform>();
    transform.pos.x += transform.velocity.x;
    transform.pos.y += transform.velocity.y;
}

void Game::sLifeSpan()
{
    // TODO: for all entities:
    //          skip if no lifespan
    //          if entity has > 0 remaining, decrement by 1
    //          if is has lifespan and is alive, scale its alpha channel
    //          if it has 0 lifespan remaining, destroy it
}

void Game::sCollision()
{
    // TODO: make sure to use collision radius and not the drawn shape's radius
    for (auto b : m_entities.getEntities("bullet"))
    {
        // split into big and small enemies??
        for (auto b : m_entities.getEntities("enemy"))
        {
            // destroy enemies hit by bullet
            // do we want bullets to pass through or only hit one 
            // (i.e. keep looping or break)
        }
    }
}

void Game::sEnemySpawner()
{
    // TODO: spawn enemy at appropriate times
}

void Game::sGUI()
{
    ImGui::Begin("Geometry Wars");
    ImGui::Text("Stuff Goes Here");
    ImGui::End();
}

void Game::sRender()
{
    // TODO: implement for all entites, only doing player atm
    m_window.clear();

    // essentially just reset sprite (circle), this doesn't have 'dynamics' per se,
    // just follows the 'actual' entity's position as defined by cTransform
    player()->get<CShape>().circle.setPosition(player()->get<CTransform>().pos);
    
    // Same goes for angle. Also we want to update the angle here to spin it
    // This angle update could be in movement but it affects nothing but
    // the visuals (no physical dynamics)
    player()->get<CTransform>().angle += 1.0f;
    player()->get<CShape>().circle.setRotation(player()->get<CTransform>().angle);

    m_window.draw(player()->get<CShape>().circle);

    // draw GUI over everythin else
    ImGui::SFML::Render(m_window);

    m_window.display();
}

void Game::sUserInput()
{
    // TODO: modify cInput of player based on actual input from user

    sf::Event event;
    while (m_window.pollEvent(event))
    {
        ImGui::SFML::ProcessEvent(m_window, event);

        if (event.type == sf::Event::Closed) { m_running = false; }
        
        if (event.type == sf::Event::KeyPressed)
        {
            // TODO do all input keys
            switch (event.key.code)
            {
                case sf::Keyboard::W:
                    std::cout << "W Key Pressed\n";
                    break;
                default: break;
            }
        }
        if (event.type == sf::Event::KeyReleased)
        {
            // TODO do all input keys
            switch (event.key.code)
            {
                case sf::Keyboard::W:
                    std::cout << "W Key Released\n";
                    break;
                default: break;
            }
        }

        if (event.type == sf::Event::MouseButtonPressed)
        {
            // Ignore mouse events if ImGUi being clicked, that will handle itself
            if (ImGui::GetIO().WantCaptureMouse) { continue; }

            // Shoot bullet
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                std::cout << "Left Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")" << std::endl;
            }

            // Do special
            if (event.mouseButton.button == sf::Mouse::Right)
            {
                std::cout << "Right Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")" << std::endl;
            }
        }
    }
}