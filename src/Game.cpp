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

    if (!m_font.loadFromFile("fonts/04B_03__.TTF"))
    {
        std::cerr << "Could not load font!\n";
        exit(-1);
    }

    m_scoreText = sf::Text("0", m_font, 50);

    spawnPlayer();
}

EntityPtr Game::player()
{
    auto& players = m_entities.getEntities("player");
    return players.front();     // should always be only one player
}

void Game::run()
{
    while (m_running)
    {
        m_entities.update();

        ImGui::SFML::Update(m_window, m_deltaClock.restart());

        // run systems
        if (!m_paused)
        {
            sLifeSpan();
            sEnemySpawner();
            sMovement();
        }
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
    m_paused = paused;
}

void Game::spawnPlayer()
{
    // TODO: add all properties of the player from the config

    auto entity = m_entities.addEntity("player");

    // Spawn player by 'giving' (reinitialising) the transform component
    entity->add<CTransform>(
        Vec2f(m_window.getSize().x / 2, m_window.getSize().y / 2),
        Vec2f(5.0f, 5.0f),
        0.0f);

    // Give shape for rendering
    entity->add<CShape>(32.0f, 8, sf::Color(10, 10, 10), sf::Color(255, 0, 0), 4.0f);

    // For now, make collision radius same as shape radius
    entity->add<CCollision>(32.0f);

    // For controls
    entity->add<CInput>();

    std::cout << "Player spawned at " << entity->get<CTransform>().pos << std::endl;
}

void Game::spawnEnemy()
{
    // TODO: use m_enemyConfig when spawning, and be withing the bounds of window.
    auto entity = m_entities.addEntity("big_enemy");
    entity->add<CTransform>(Vec2f(100.0f, 50.0f), Vec2f(2.0f, 1.0f), 0.0f);
    entity->add<CShape>(16.0f, 4, sf::Color(10, 10, 10), sf::Color(0, 0, 255), 4.0f);
    entity->add<CCollision>(16.0f);
    // Score should be dependent on number of vertices
    entity->add<CScore>(4*100);

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

    // Get unit direction based on target and origin
    Vec2f direction = (target - entity->get<CTransform>().pos).unit_vector();
    std::cout << "target: " << target << " origin: " << entity->get<CTransform>().pos 
    << " direction: " << direction << std::endl;

    auto b_entity = m_entities.addEntity("bullet");
    // Scale to velocity using config and spawn from entity (player)
    b_entity->add<CTransform>(entity->get<CTransform>().pos, direction*10.0f, 0.0f);
    // Bullets are just circles, so just do enough points to look like one
    b_entity->add<CShape>(2.0f, 32, sf::Color(200, 200, 200), sf::Color(200, 200, 200), 1.0f);
    b_entity->add<CCollision>(2.0f);
    b_entity->add<CLifespan>(60*1);
}

void Game::spawnSpecial(EntityPtr entity)
{
    // TODO
}

void Game::sMovement()
{
    // Update velocity of player based on current input.
    // First get 'activations' to get direction 
    Vec2f playerDirection;
    if (player()->get<CInput>().up) playerDirection.y -= 1;
    if (player()->get<CInput>().down) playerDirection.y += 1;
    if (player()->get<CInput>().left) playerDirection.x -= 1;
    if (player()->get<CInput>().right) playerDirection.x += 1;

    // Normalise to unit vector then scale with desired speed magnitued in config
    // So we can avoid side straffing speed up
    float speed = 5.0f;
    player()->get<CTransform>().velocity = playerDirection.unit_vector() * speed;

    // Move based on velocity
    for (auto entity : m_entities.getEntities())
    {
        entity->get<CTransform>().pos += entity->get<CTransform>().velocity;
    }
}

void Game::sLifeSpan()
{
    for (auto entity : m_entities.getEntities())
    {
        auto& lifespan = entity->get<CLifespan>();

        if (!lifespan.exists) continue;

        lifespan.remaining -= 1;

        // Update alpha channel too
        int alpha = int(float(lifespan.remaining)*255 / float(lifespan.lifespan));
        sf::Color currentColor = entity->get<CShape>().circle.getFillColor();
        entity->get<CShape>().circle.setFillColor(
            sf::Color(currentColor.r, currentColor.g, currentColor.b, alpha)
        );
        currentColor = entity->get<CShape>().circle.getOutlineColor();
        entity->get<CShape>().circle.setOutlineColor(
            sf::Color(currentColor.r, currentColor.g, currentColor.b, alpha)
        );

        if (lifespan.remaining == 0)
        {
            entity->destroy();
        }
    }
}

void Game::sCollision()
{
    float distanceBetweenCentres, collisionDistance;

    // Enemy vs player collisions - reset score and respawn player
    for (auto& vec : {m_entities.getEntities("big_enemy"), m_entities.getEntities("small_enemy")})
    {
        for (auto e : vec)
        {
            distanceBetweenCentres = player()->get<CTransform>().pos.dist(e->get<CTransform>().pos);
            collisionDistance = player()->get<CCollision>().radius + e->get<CCollision>().radius;

            if ( distanceBetweenCentres <= collisionDistance )
            {
                m_score = 0;
                player()->destroy();
                e->destroy();
                spawnPlayer();
            }
        }
    }

    // Bullet collisions
    for (auto b : m_entities.getEntities("bullet"))
    {
        // split into big and small enemies??
        for (auto b_e : m_entities.getEntities("big_enemy"))
        {
            distanceBetweenCentres = b->get<CTransform>().pos.dist(b_e->get<CTransform>().pos);
            collisionDistance = b->get<CCollision>().radius + b_e->get<CCollision>().radius;

            if ( distanceBetweenCentres <= collisionDistance )
            {
                // update score
                m_score += b_e->get<CScore>().score;

                // explode enemy
                spawnSmallEnemies(b_e);
                b_e->destroy();

                // Also destroy the bullet and move on
                b->destroy();
                break;
            }
        }

        // because we're doing 2 separate loops, check if this bullet hasn't
        // alreayd hit something
        if (!b->isActive()) continue;

        for (auto s_e : m_entities.getEntities("small_enemy"))
        {
            distanceBetweenCentres = b->get<CTransform>().pos.dist(s_e->get<CTransform>().pos);
            collisionDistance = b->get<CCollision>().radius + s_e->get<CCollision>().radius;

            if ( distanceBetweenCentres <= collisionDistance )
            {
                // update score and destroy
                m_score += s_e->get<CScore>().score;
                s_e->destroy();

                // Also destroy the bullet and move on
                b->destroy();
                break;
            }
        }
    }

    // Wall collisions
    for (auto entity : m_entities.getEntities())
    {
        Vec2<unsigned int> windowSize(m_window.getSize());

        auto& transform = entity->get<CTransform>();
        float collisionRadius = entity->get<CCollision>().radius;

        // Left wall
        if ( (transform.pos.x-collisionRadius) <= 0 )
        {
            transform.velocity.x *= -1;
            transform.pos.x = collisionRadius;
        }
        // Right wall - other side can be else since both won't happen??
        else if ( (transform.pos.x+collisionRadius) >= windowSize.x )
        {
            transform.velocity.x *= -1;
            transform.pos.x = windowSize.x-collisionRadius;
        }
        
        // Top wall - independent checks for each dimension
        if ( (transform.pos.y-collisionRadius) <= 0 )
        {
            transform.velocity.y *= -1;
            transform.pos.y = collisionRadius;
        }
        // Bottom wall
        else if ( (transform.pos.y+collisionRadius) >= windowSize.y )
        {
            transform.velocity.y *= -1;
            transform.pos.y = windowSize.y-collisionRadius;
        }

    }
}

void Game::sEnemySpawner()
{
    // TODO: spawn enemy at appropriate times
    // Should be related to m_enemyConfig.SP, right now just every 2s
    if ( (m_currentFrame == 0) || ((m_currentFrame-m_lastEnemySpawnTime) >= 60*2) )
    {
        spawnEnemy();
    }
}

void Game::sGUI()
{
    ImGui::Begin("Geometry Wars");
    ImGui::Text("Stuff Goes Here");
    ImGui::End();
}

void Game::sRender()
{
    m_window.clear();

    // Render all entities
    for (auto entity : m_entities.getEntities())
    {
        // essentially just reset sprite (circle), this doesn't have 'dynamics' per se,
        // just follows the 'actual' entity's position as defined by cTransform
        entity->get<CShape>().circle.setPosition(entity->get<CTransform>().pos);
        
        // Same goes for angle. Also we want to update the angle here to spin it
        // This angle update could be in movement but it affects nothing but
        // the visuals (no physical dynamics)
        // Plus no need to spin bullets since they just look circular
        if ( entity->tag() != "bullet" )
        {
            entity->get<CTransform>().angle += 1.0f;
            entity->get<CShape>().circle.setRotation(entity->get<CTransform>().angle);
        }

        m_window.draw(entity->get<CShape>().circle);
    }

    m_window.draw(player()->get<CShape>().circle);

    // update score and draw over entities
    m_scoreText.setString(std::to_string(m_score));
    // Update position so it's 'right aligned'
    sf::FloatRect textBBox = m_scoreText.getLocalBounds();
    m_scoreText.setPosition(1270-textBBox.width, 20);

    m_window.draw(m_scoreText);

    // draw GUI over everything else
    ImGui::SFML::Render(m_window);

    m_window.display();
}

void Game::sUserInput()
{
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
                    player()->get<CInput>().up = true;
                    break;
                case sf::Keyboard::A:
                    std::cout << "A Key Pressed\n";
                    player()->get<CInput>().left = true;
                    break;
                case sf::Keyboard::S:
                    std::cout << "S Key Pressed\n";
                    player()->get<CInput>().down = true;
                    break;
                case sf::Keyboard::D:
                    std::cout << "D Key Pressed\n";
                    player()->get<CInput>().right = true;
                    break;

                case sf::Keyboard::P:
                    std::cout << "P Key Pressed\n";
                    setPaused(!m_paused);
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
                    player()->get<CInput>().up = false;
                    break;
                case sf::Keyboard::A:
                    std::cout << "A Key Released\n";
                    player()->get<CInput>().left = false;
                    break;
                case sf::Keyboard::S:
                    std::cout << "S Key Released\n";
                    player()->get<CInput>().down = false;
                    break;
                case sf::Keyboard::D:
                    std::cout << "D Key Released\n";
                    player()->get<CInput>().right = false;
                    break;
                default: break;
            }
        }

        if (event.type == sf::Event::MouseButtonPressed)
        {
            // Ignore mouse events if ImGUi being clicked, that will handle itself
            if (ImGui::GetIO().WantCaptureMouse) { continue; }

            // Let's not let the player stack up shots in pause mode
            if (m_paused) { continue; }

            // Shoot bullet
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                std::cout << "Left Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")" << std::endl;
                spawnBullet(player(), Vec2f(event.mouseButton.x, event.mouseButton.y));
            }

            // Do special
            if (event.mouseButton.button == sf::Mouse::Right)
            {
                std::cout << "Right Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")" << std::endl;
            }
        }
    }
}