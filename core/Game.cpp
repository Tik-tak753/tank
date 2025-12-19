#include "core/Game.h"

#include <QtGlobal>

#include "gameplay/EnemyTank.h"
#include "gameplay/PlayerTank.h"
#include "gameplay/Bullet.h"
#include "gameplay/Direction.h"
#include "systems/InputSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/CollisionSystem.h"
#include "world/Base.h"
#include "world/LevelLoader.h"
#include "world/Map.h"

Game::Game(QObject* parent)
    : QObject(parent),
      m_levelLoader(std::make_unique<LevelLoader>()),
      m_physicsSystem(std::make_unique<PhysicsSystem>()),
      m_collisionSystem(std::make_unique<CollisionSystem>())
{
}

Game::~Game()
{
    clearWorld();
}

void Game::initialize()
{
    // На цьому кроці в майбутньому будемо створювати карту, танки та базу.
    // Поки що готуємо лише базовий стан сесії.

    clearWorld();

    m_state.reset(m_rules.playerLives(), m_rules.enemiesPerWave());
    if (!m_levelLoader)
        m_levelLoader = std::make_unique<LevelLoader>();

    LevelData level = m_levelLoader->loadDefaultLevel(m_rules);
    m_map = std::move(level.map);
    m_base = std::make_unique<Base>(level.baseCell);

    auto player = std::make_unique<PlayerTank>(level.playerSpawn);
    player->setMap(m_map.get());
    player->setInput(m_inputSystem);

    m_player = player.get();
    m_tanks.append(player.release());

    const qsizetype enemiesToSpawn = qMin(level.enemySpawns.size(), static_cast<qsizetype>(3));
    for (qsizetype i = 0; i < enemiesToSpawn; ++i) {
        auto enemy = std::make_unique<EnemyTank>(level.enemySpawns.at(i));
        enemy->setDirection(Direction::Down);
        m_tanks.append(enemy.release());
    }
}

void Game::setInputSystem(InputSystem* input)
{
    m_inputSystem = input;
    if (m_player)
        m_player->setInput(m_inputSystem);
}

void Game::update(int deltaMs)
{
    if (!m_map)
        initialize();

    updateTanks(deltaMs);
    spawnPendingBullets();

    if (m_physicsSystem)
        m_physicsSystem->update(m_bullets, deltaMs);

    if (m_collisionSystem && m_map)
        m_collisionSystem->resolve(*m_map, m_tanks, m_bullets, m_base.get(), m_state);

    m_pendingBullets.clear();

    removeDeadTanks();
    // Прибираємо всі снаряди, що втратили життєздатність
    for (qsizetype i = m_bullets.size(); i > 0; --i) {
        Bullet* bullet = m_bullets.at(i - 1);
        if (bullet && !bullet->isAlive()) {
            delete bullet;
            m_bullets.removeAt(i - 1);
        }
    }
}

void Game::clearWorld()
{
    for (Bullet* bullet : m_bullets)
        delete bullet;
    m_bullets.clear();
    m_pendingBullets.clear();

    for (Tank* tank : m_tanks)
        delete tank;
    m_tanks.clear();
    m_enemies.clear();
    m_map.reset();
    m_base.reset();
    m_player = nullptr;
}

void Game::updateTanks(int deltaMs)
{
    for (Tank* tank : m_tanks) {
        if (!tank)
            continue;

        tank->updateWithDelta(deltaMs);
        std::unique_ptr<Bullet> bullet = tank->tryShoot();
        if (bullet)
            m_pendingBullets.push_back(std::move(bullet));
    }
}

void Game::spawnPendingBullets()
{
    while (!m_pendingBullets.empty()) {
        std::unique_ptr<Bullet> bullet = std::move(m_pendingBullets.back());
        m_pendingBullets.pop_back();
        m_bullets.append(bullet.release());
    }
}

void Game::removeDeadTanks()
{
    for (qsizetype i = m_tanks.size(); i > 0; --i) {
        Tank* tank = m_tanks.at(i - 1);
        if (tank && !tank->health().isAlive()) {
            if (tank == m_player) {
                m_player = nullptr;
                m_state.registerPlayerLostLife();
            }

            delete tank;
            m_tanks.removeAt(i - 1);
        }
    }
}
