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
#include "world/Tile.h"

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

    const int totalEnemies = m_rules.enemiesPerWave() * m_rules.totalWaves();
    m_state.reset(m_rules.playerLives(), totalEnemies);
    if (!m_levelLoader)
        m_levelLoader = std::make_unique<LevelLoader>();

    LevelData level = m_levelLoader->loadDefaultLevel(m_rules);
    m_map = std::move(level.map);
    m_base = std::make_unique<Base>(level.baseCell);
    m_enemySpawnPoints = level.enemySpawns;
    m_playerSpawn = level.playerSpawn;
    m_maxAliveEnemies = m_rules.enemiesPerWave();
    m_nextSpawnIndex = 0;
    m_enemySpawnCooldownMs = 0;
    m_playerRespawnCooldownMs = 0;

    auto player = std::make_unique<PlayerTank>(m_playerSpawn);
    player->setMap(m_map.get());
    player->setInput(m_inputSystem);
    player->setDirection(Direction::Up);

    m_player = player.get();
    m_tanks.append(player.release());

    updateEnemySpawning(0);
}

void Game::restart()
{
    initialize();
}

void Game::setInputSystem(InputSystem* input)
{
    m_inputSystem = input;
    if (m_player)
        m_player->setInput(m_inputSystem);
}

void Game::update(int deltaMs)
{
    // Прибираємо всі снаряди, що втратили життєздатність у попередніх тиках
    for (qsizetype i = m_bullets.size(); i > 0; --i) {
        Bullet* bullet = m_bullets.at(i - 1);
        if (bullet && !bullet->isAlive()) {
            delete bullet;
            m_bullets.removeAt(i - 1);
        }
    }

    if (m_state.isBaseDestroyed())
        return;

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
    updatePlayerRespawn(deltaMs);
    updateEnemySpawning(deltaMs);
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
    m_enemySpawnPoints.clear();
    m_playerSpawn = QPoint();
    m_playerRespawnCooldownMs = 0;
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
    bool enemyDestroyed = false;
    for (qsizetype i = m_tanks.size(); i > 0; --i) {
        Tank* tank = m_tanks.at(i - 1);
        if (!tank)
            continue;

        if (!tank->health().isAlive() && !tank->isDestroyed())
            tank->markDestroyed();

        if (!tank->isDestructionFinished())
            continue;

        if (tank == m_player) {
            m_player = nullptr;
            m_state.registerPlayerLostLife();
            if (m_state.remainingLives() > 0)
                m_playerRespawnCooldownMs = m_playerRespawnDelayMs;
        } else if (dynamic_cast<EnemyTank*>(tank)) {
            m_enemies.removeOne(static_cast<EnemyTank*>(tank));
            m_state.registerEnemyDestroyed();
            enemyDestroyed = true;
        }

        delete tank;
        m_tanks.removeAt(i - 1);
    }

    if (enemyDestroyed && m_state.enemiesToSpawn() > 0 && m_enemySpawnCooldownMs == 0)
        m_enemySpawnCooldownMs = m_enemyRespawnDelayMs;
}

void Game::updatePlayerRespawn(int deltaMs)
{
    if (m_player || !m_map)
        return;

    if (m_state.remainingLives() <= 0)
        return;

    if (m_playerRespawnCooldownMs > 0)
        m_playerRespawnCooldownMs = qMax(0, m_playerRespawnCooldownMs - deltaMs);

    if (m_playerRespawnCooldownMs > 0)
        return;

    if (!canSpawnEnemyAt(m_playerSpawn))
        return;

    auto player = std::make_unique<PlayerTank>(m_playerSpawn);
    player->setMap(m_map.get());
    player->setInput(m_inputSystem);
    player->setDirection(Direction::Up);

    m_player = player.get();
    m_tanks.append(player.release());
}

void Game::updateEnemySpawning(int deltaMs)
{
    if (!m_map)
        return;

    if (m_enemySpawnCooldownMs > 0) {
        m_enemySpawnCooldownMs = qMax(0, m_enemySpawnCooldownMs - deltaMs);
        if (m_enemySpawnCooldownMs > 0)
            return;
    }

    bool spawnedAny = false;
    while (m_state.aliveEnemies() < m_maxAliveEnemies && m_state.enemiesToSpawn() > 0) {
        if (!trySpawnEnemy())
            break;
        spawnedAny = true;
    }

    if (spawnedAny && m_state.enemiesToSpawn() > 0)
        m_enemySpawnCooldownMs = m_enemyRespawnDelayMs;
}

bool Game::trySpawnEnemy()
{
    if (!m_map || m_enemySpawnPoints.isEmpty())
        return false;

    const qsizetype spawnCount = m_enemySpawnPoints.size();
    for (qsizetype i = 0; i < spawnCount; ++i) {
        const QPoint cell = m_enemySpawnPoints.at(m_nextSpawnIndex);
        m_nextSpawnIndex = (m_nextSpawnIndex + 1) % spawnCount;

        if (!canSpawnEnemyAt(cell))
            continue;

        auto enemy = std::make_unique<EnemyTank>(cell);
        enemy->setDirection(Direction::Down);
        enemy->setMap(m_map.get());

        EnemyTank* enemyPtr = enemy.get();
        m_state.registerSpawnedEnemy();
        m_tanks.append(enemy.release());
        m_enemies.append(enemyPtr);
        return true;
    }

    return false;
}

bool Game::canSpawnEnemyAt(const QPoint& cell) const
{
    if (!m_map || !m_map->isInside(cell))
        return false;

    const Tile tile = m_map->tile(cell);
    if (tile.type != TileType::Empty)
        return false;

    for (Tank* tank : m_tanks) {
        if (!tank)
            continue;

        if (!tank->isDestructionFinished() && tank->cell() == cell)
            return false;
    }

    return true;
}
