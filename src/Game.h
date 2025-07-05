#pragma once

#include "Vec2.hpp"
#include "Entity.hpp"
#include "EntityManager.hpp"
#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"

#define PI 3.14159265358979323846

struct PlayerConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S; };
struct EnemyConfig  { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX; };
struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };
struct SpecialConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L, CL, CD; float S; };

using EntityPtr = std::shared_ptr<Entity>;

class Game
{
    sf::RenderWindow    m_window;
    EntityManager       m_entities;
    sf::Font            m_font;
    sf::Text            m_scoreText;
    PlayerConfig        m_playerConfig;
    EnemyConfig         m_enemyConfig;
    BulletConfig        m_bulletConfig;
    SpecialConfig       m_specialConfig;
    sf::Clock           m_deltaClock;
    int                 m_score = 0;
    int                 m_currentFrame = 0;
    int                 m_lastEnemySpawnTime = 0;
    bool                m_paused = false;
    bool                m_running = true;
    bool                m_LifeSpanOn = true;
    bool                m_EnemySpawnerOn = true;
    bool                m_MovementOn = true;
    bool                m_CollisionOn = true;
    bool                m_UserInputOn = true;
    bool                m_CooldownsOn = true;

    void init(const std::string& config);   // initialise game state with config file
    void setPaused(bool paused);            // pause game

    // Game systems
    void sMovement();
    void sLifeSpan();
    void sCollision();
    void sEnemySpawner();
    void sCooldowns();
    void sGUI();
    void sRender();
    void sUserInput();

    // Spawn player using config info
    void spawnPlayer();

    // Spawn enemy at random location within bounds of window
    void spawnEnemy();

    // Spawns small enemies when big one dies. We take the big one as input so we can destroy it
    void spawnSmallEnemies(EntityPtr entity);
    
    // Spawns bullet from given entity towards mouse location
    void spawnBullet(EntityPtr entity, const Vec2f& mousePos);
    
    // Spawns faster and larger bullet with much longer lifespan and can hit multiple enemies
    void spawnSpecial(EntityPtr entity, const Vec2f& mousePos);

    // Convenient helper function
    EntityPtr player();

public:
    // Constructor takes in game config file path
    Game(const std::string& config);

    // Main game loop
    void run();
};