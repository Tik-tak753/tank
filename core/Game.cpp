#include "core/Game.h"

#include "gameplay/EnemyTank.h"
#include "gameplay/PlayerTank.h"
#include "gameplay/Bullet.h"
#include "rendering/Renderer.h"
#include "systems/CollisionSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/InputSystem.h"
#include "world/Base.h"
#include "world/LevelLoader.h"
#include "world/Map.h"

Game::Game(QObject* parent)
    : QObject(parent),
      m_loop(this)
{
    m_levelLoader = std::make_unique<LevelLoader>();
    m_collisionSystem = std::make_unique<CollisionSystem>();
    m_physicsSystem = std::make_unique<PhysicsSystem>();

    m_loop.setTickCallback([this]() { update(); });
}

Game::~Game()
{
    for (Bullet* bullet : m_bullets)
        delete bullet;
    for (Tank* tank : m_tanks)
        delete tank;
}

void Game::initialize()
{
    auto level = m_levelLoader->loadDefaultLevel(m_rules);
    m_map = std::move(level.map);
    m_base = std::make_unique<Base>(level.baseCell);

    m_state.reset(m_rules.playerLives(),
                  m_rules.enemiesPerWave() * m_rules.totalWaves());

    auto player = new PlayerTank(level.playerSpawn);
    m_tanks.append(player);

    for (const QPoint& spawn : level.enemySpawns) {
        auto enemy = new EnemyTank(spawn);
        m_tanks.append(enemy);
        m_enemies.append(enemy);
    }
}

void Game::start()
{
    m_loop.start();
}

void Game::stop()
{
    m_loop.stop();
}

void Game::update()
{
    if (!m_map)
        return;

    for (Tank* tank : m_tanks)
        tank->update();

    if (m_physicsSystem)
        m_physicsSystem->update(m_bullets, *m_map);

    if (m_collisionSystem)
        m_collisionSystem->resolve(*m_map, m_tanks, m_bullets, m_base.get(), m_state);

    if (m_renderer)
        m_renderer->renderFrame(*this);
}

void Game::setRenderer(Renderer* renderer)
{
    m_renderer = renderer;
}

void Game::setInputSystem(InputSystem* input)
{
    m_inputSystem = input;
}
