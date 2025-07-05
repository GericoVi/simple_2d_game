#include "Game.h"

#include <iostream>
#include <math.h>
#include <fstream>

// Get random float between 0 to 1.0
float randFloat()
{
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

Game::Game(const std::string& config) { init(config); }

void Game::init(const std::string& path)
{
    // read in config file and place data into structs
    std::ifstream fin("config.txt");
    if (!fin.is_open())
    {
        std::cerr << "Config file not found!";
        exit(-1);
    }
    std::string rowType, fontFile;
    int wWidth, wHeight, frameLimit, fullscreen;
    int fontSize, fontColorR, fontColorG, fontColorB;
    while (fin >> rowType)
    {
        if (rowType == "Window")
        {
            fin >> wWidth >> wHeight >> frameLimit >> fullscreen;
        }
        else if (rowType == "Font")
        {
            fin >> fontFile >> fontSize >> fontColorR >> fontColorG >> fontColorB;
        }
        else if (rowType == "Player")
        {
            fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S
            >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB
            >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB
            >> m_playerConfig.V;
        }
        else if (rowType == "Enemy")
        {
            fin >> m_enemyConfig.SR >> m_enemyConfig.CR
            >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX
            >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB
            >> m_enemyConfig.OT
            >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX
            >> m_enemyConfig.L >> m_enemyConfig.SI;
        }
        else if (rowType == "Bullet")
        {
            fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S
            >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB
            >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB
            >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
        }
    }

    std::cout << m_enemyConfig.SI;

    // setup window
    m_window.create(
        sf::VideoMode(wWidth, wHeight),
        "Assignment 2",
        fullscreen ? sf::Style::Fullscreen : sf::Style::Default
    );
    m_window.setFramerateLimit(frameLimit);

    ImGui::SFML::Init(m_window);
    ImGui::GetStyle().ScaleAllSizes(1.0f);
    ImGui::GetIO().FontGlobalScale = 1.0f;

    if (!m_font.loadFromFile(fontFile))
    {
        std::cerr << "Could not load font!\n";
        exit(-1);
    }

    m_scoreText = sf::Text("0", m_font, fontSize);
    m_scoreText.setColor(sf::Color(fontColorR, fontColorG, fontColorB));

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
            if (m_LifeSpanOn) sLifeSpan();
            if (m_EnemySpawnerOn) sEnemySpawner();
            if (m_MovementOn) sMovement();
        }
        if (m_CollisionOn) sCollision();
        if (m_UserInputOn) sUserInput();
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
    auto entity = m_entities.addEntity("player");

    // Spawn player by 'giving' (reinitialising) the transform component
    entity->add<CTransform>(
        Vec2f(m_window.getSize().x / 2, m_window.getSize().y / 2),
        Vec2f(0.f, 0.f),
        0.0f);

    // Give shape for rendering
    entity->add<CShape>(
        static_cast<float>(m_playerConfig.SR),
        m_playerConfig.V,
        sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB),
        sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB),
        4.0f);

    // For now, make collision radius same as shape radius
    entity->add<CCollision>(static_cast<float>(m_playerConfig.CR));

    // For controls
    entity->add<CInput>();
    // std::cout << "Player spawned at " << entity->get<CTransform>().pos << std::endl;
}

void Game::spawnEnemy()
{
    auto entity = m_entities.addEntity("big_enemy");

    // Spawnable region so it doesn't overlap with window
    int min_x = m_enemyConfig.SR, min_y = m_enemyConfig.SR;
    int max_x = m_window.getSize().x-m_enemyConfig.SR;
    int max_y = m_window.getSize().y-m_enemyConfig.SR;

    // Get random position
    Vec2f randomPos(
        static_cast<float>((rand() % (max_x-min_x)) + min_x),
        static_cast<float>((rand() % (max_y-min_y)) + min_y)
    );

    // Get random angle for movement
    float randAngleRads = randFloat() * PI*2;
    Vec2f randVel(cosf(randAngleRads), sinf(randAngleRads));
    // Scale to random speed
    randVel *= (
        randFloat()*(m_enemyConfig.SMAX-m_enemyConfig.SMIN)
        + m_enemyConfig.SMIN);

    entity->add<CTransform>(randomPos, randVel, 0.0f);

    // Get random num sides
    int randNumVertices = 
        (rand() % (m_enemyConfig.VMAX - m_enemyConfig.VMIN)) 
        + m_enemyConfig.VMIN;

    // Get random colour
    sf::Color randFillColor(rand() % 255, rand() % 255, rand() % 255);

    entity->add<CShape>(
        static_cast<float>(m_enemyConfig.SR),
        randNumVertices,
        randFillColor,
        sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB),
        static_cast<float>(m_enemyConfig.OT));
    
    entity->add<CCollision>(static_cast<float>(m_enemyConfig.CR));
    // Score should be dependent on number of vertices
    entity->add<CScore>(4*100);

    m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(EntityPtr entity)
{
    // when we create the smaller enemies, we have to read the values of the original e
    // - spawn enemies equal to number of vertices
    // - small ones have same colour but half the size
    // - they're worth double points
    int numVertices = entity->get<CShape>().circle.getPointCount();
    
    // Small enemies move away in equal angle increments
    float angleIncrement = 2.f*PI / numVertices;
    // Start from the angle of the original entity at the time
    float angleCurrent = entity->get<CTransform>().angle;

    for ( int n = 0; n < numVertices; n++ )
    {
        auto s_e = m_entities.addEntity("small_enemy");

        Vec2f vel(cosf(angleCurrent), sinf(angleCurrent));
        vel *= 2.5f;
        
        s_e->add<CTransform>(entity->get<CTransform>().pos, vel, 0.0f);

        auto& shapeOrig = entity->get<CShape>();
        s_e->add<CShape>(
            shapeOrig.circle.getRadius() / 2,
            numVertices,
            shapeOrig.circle.getFillColor(),
            shapeOrig.circle.getOutlineColor(),
            shapeOrig.circle.getOutlineThickness()
        );
        
        s_e->add<CCollision>(entity->get<CCollision>().radius / 2);
        s_e->add<CScore>(entity->get<CScore>().score * 2);

        // TODO: get lifespan from config
        s_e->add<CLifespan>(60*m_enemyConfig.L);

        angleCurrent += angleIncrement;
    }
}

void Game::spawnBullet(EntityPtr entity, const Vec2f& target)
{
    // Get unit direction based on target and origin
    Vec2f direction = (target - entity->get<CTransform>().pos).unit_vector();
    // std::cout << "target: " << target << " origin: " << entity->get<CTransform>().pos 
    // << " direction: " << direction << std::endl;

    auto b_entity = m_entities.addEntity("bullet");
    // Scale to velocity using config and spawn from entity (player)
    b_entity->add<CTransform>(
        entity->get<CTransform>().pos,
        direction * m_bulletConfig.S,
        0.0f);

    // Bullets are just circles, so just do enough points to look like one
    b_entity->add<CShape>(
        static_cast<float>(m_bulletConfig.SR),
        m_bulletConfig.V,
        sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB),
        sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB),
        static_cast<float>(m_bulletConfig.OT));

    b_entity->add<CCollision>(static_cast<float>(m_bulletConfig.CR));
    b_entity->add<CLifespan>(60*m_bulletConfig.L);
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
    player()->get<CTransform>().velocity = playerDirection.unit_vector() * m_playerConfig.S;

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
    auto pairwiseCollision = [this](EntityPtr entityA, EntityPtr entityB)
{
    float distanceBetweenCentres, collisionDistance;

        distanceBetweenCentres = entityA->get<CTransform>().pos.dist(entityB->get<CTransform>().pos);
        collisionDistance = entityA->get<CCollision>().radius + entityB->get<CCollision>().radius;

        if ( distanceBetweenCentres <= collisionDistance ) return true;

        return false;
    };

    auto collideUpdate = [](EntityPtr entity)
    {
        auto& eCollision = entity->get<CCollision>();
        eCollision.collisionsRemaining -= 1;
        
        if (eCollision.collisionsRemaining == 0) return true;
        return false;
    };

    // Enemy vs player collisions - reset score and respawn player
    // Conveniently loops through all enemy tags
    for (auto& vec : {m_entities.getEntities("big_enemy"), m_entities.getEntities("small_enemy")})
    {
        for (auto e : vec)
        {
            if (pairwiseCollision(player(), e))
            {
                if (collideUpdate(player()))
                {
                    player()->destroy();
                    // Make sure to reset score and respawn player
                m_score = 0;
                    spawnPlayer();
                }
                if (collideUpdate(e))
                {
                e->destroy();
                }
            }
        }
    }

    // Bullet collisions
    // We can loops through both normal bullets and specials
    for (auto& vec : {m_entities.getEntities("bullet"), m_entities.getEntities("special")})
    {
        for (auto b : vec)
        {
        for (auto b_e : m_entities.getEntities("big_enemy"))
        {
                if (pairwiseCollision(b, b_e))
                {
                    if (collideUpdate(b_e))
                    {
                        // explode enemy and add to score
                        b_e->destroy();
                        spawnSmallEnemies(b_e);
                m_score += b_e->get<CScore>().score;
                    }
                    if (collideUpdate(b))
                    {
                b->destroy();
                break;
                    }
            }
        }

        // because we're doing 2 separate loops, check if this bullet hasn't
            // already hit something
        if (!b->isActive()) continue;

        for (auto s_e : m_entities.getEntities("small_enemy"))
        {
                if (pairwiseCollision(b, s_e))
                {
                    if (collideUpdate(s_e))
            {
                        // add to score
                        s_e->destroy();
                m_score += s_e->get<CScore>().score;
                    }
                    if (collideUpdate(b))
                    {
                b->destroy();
                break;
                    }
                }
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
    if ( 
        (m_currentFrame == 0) ||
        ((m_currentFrame-m_lastEnemySpawnTime) >= 60*m_enemyConfig.SI) 
    )
    {
        spawnEnemy();
    }
}

void Game::sGUI()
{
    ImGui::Begin("Geometry Wars");

    ImGui::Text("System Toggles");
    ImGui::Checkbox("Movement Toggle", &m_MovementOn);
    ImGui::Checkbox("Enemy Spawner Toggle", &m_EnemySpawnerOn);
    ImGui::Checkbox("Collision Toggle", &m_CollisionOn);
    ImGui::Checkbox("Life Span Toggle", &m_LifeSpanOn);
    ImGui::Checkbox("User Input Toggle", &m_UserInputOn);
    ImGui::Separator();

    ImGui::BeginListBox("Entities");
    for (const auto& e : m_entities.getEntities())
    {
        if (ImGui::Button(
            ("Destroy###"+std::to_string(e->id())).c_str(), {0,0})) 
        {
            e->destroy();

            // Need to make sure to immediately respawn player if it gets
            // manually destroyed here. Or else we seg fault and crash
            if (e->tag() == "player") 
            {
                spawnPlayer();
                // Let's reset the score too
                m_score = 0;
            }

        }
        ImGui::SameLine();

        auto position = e->get<CTransform>().pos;
        ImGui::Text(
            "%04d: %s at [%.0f,%.0f]",
            e->id(), e->tag().c_str(), position.x, position.y);
    }
    ImGui::EndListBox();
    ImGui::Separator();

    ImGui::Text("Enemy Spawning");
    ImGui::SliderInt("Spawn Interval (s)", &m_enemyConfig.SI, 0, 10);
    if (ImGui::Button("Spawn One")) spawnEnemy();

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
            switch (event.key.code)
            {
                case sf::Keyboard::W:
                    // std::cout << "W Key Pressed\n";
                    player()->get<CInput>().up = true;
                    break;
                case sf::Keyboard::A:
                    // std::cout << "A Key Pressed\n";
                    player()->get<CInput>().left = true;
                    break;
                case sf::Keyboard::S:
                    // std::cout << "S Key Pressed\n";
                    player()->get<CInput>().down = true;
                    break;
                case sf::Keyboard::D:
                    // std::cout << "D Key Pressed\n";
                    player()->get<CInput>().right = true;
                    break;

                case sf::Keyboard::P:
                    // std::cout << "P Key Pressed\n";
                    setPaused(!m_paused);
                    break;

                default: break;
            }
        }
        if (event.type == sf::Event::KeyReleased)
        {
            switch (event.key.code)
            {
                case sf::Keyboard::W:
                    // std::cout << "W Key Released\n";
                    player()->get<CInput>().up = false;
                    break;
                case sf::Keyboard::A:
                    // std::cout << "A Key Released\n";
                    player()->get<CInput>().left = false;
                    break;
                case sf::Keyboard::S:
                    // std::cout << "S Key Released\n";
                    player()->get<CInput>().down = false;
                    break;
                case sf::Keyboard::D:
                    // std::cout << "D Key Released\n";
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
                // std::cout << "Left Mouse Button Clicked at (" << event.mouseButton.x << "," << event.mouseButton.y << ")" << std::endl;
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