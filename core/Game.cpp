#include "core/Game.h"

#include <QtGlobal>

#include "gameplay/EnemyTank.h"
#include "gameplay/PlayerTank.h"
#include "gameplay/Bullet.h"
#include "gameplay/Direction.h"
#include "gameplay/Bonus.h"
#include "core/GameRules.h"
#include "systems/InputSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/CollisionSystem.h"
#include "world/Base.h"
#include "world/LevelLoader.h"
#include "world/Map.h"
#include "world/Tile.h"
#include <QRandomGenerator>

namespace {
constexpr int kBonusSpawnIntervalMinMs = 15000;
constexpr int kBonusSpawnIntervalMaxMs = 20000;
constexpr int kEnemyKillsPerBonus = 4;
}
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

void Game::startNewGame()
{
    initialize();
}

void Game::initialize()
{
    // На цьому кроці в майбутньому будемо створювати карту, танки та базу.
    // Поки що готуємо лише базовий стан сесії.

    clearWorld();

    const int totalEnemies = m_rules.enemiesPerWave() * m_rules.totalWaves();
    m_state.reset(m_rules.playerLives(), totalEnemies);
    m_state.setGameMode(GameMode::Playing);
    prepareEnemyQueue(totalEnemies);
    if (!m_levelLoader)
        m_levelLoader = std::make_unique<LevelLoader>();

    LevelData level = m_levelLoader->loadDefaultLevel(m_rules);
    m_map = std::move(level.map);

    const QPoint iceStart(3, 3);
    for (int offset = 0; offset < 4; ++offset) {
        const QPoint cell = iceStart + QPoint(offset, 0);
        if (m_map && m_map->isInside(cell))
            m_map->setTile(cell, TileFactory::ice());
    }

    const QPoint waterStart(10, 8);
    for (int dy = 0; dy < 2; ++dy) {
        for (int dx = 0; dx < 2; ++dx) {
            const QPoint cell = waterStart + QPoint(dx, dy);
            if (m_map && m_map->isInside(cell))
                m_map->setTile(cell, TileFactory::water());
        }
    }

    m_base = std::make_unique<Base>(level.baseCell);
    m_enemySpawnPoints = level.enemySpawns;
    m_playerSpawnCell = level.playerSpawn;
    m_maxAliveEnemies = m_rules.enemiesPerWave();
    m_nextSpawnIndex = 0;
    m_enemySpawnCooldownMs = 0;
    m_playerRespawnTimerMs = 0;

    auto player = std::make_unique<PlayerTank>(level.playerSpawn);
    player->setMap(m_map.get());
    player->setInput(m_inputSystem);

    m_player = player.get();
    m_tanks.append(player.release());

    m_enemyKillsSinceBonus = 0;
    m_bonusSpawnTimerMs = rollBonusSpawnIntervalMs();
    m_enemyFreezeTimerMs = 0;

    updateEnemySpawning(0);
}

void Game::restart()
{
    initialize();
}

void Game::pause()
{
    if (m_state.gameMode() == GameMode::Playing)
        m_state.setGameMode(GameMode::Paused);
}

void Game::resume()
{
    if (m_state.gameMode() == GameMode::Paused)
        m_state.setGameMode(GameMode::Playing);
}

void Game::enterMainMenu()
{
    clearWorld();
    m_state.setSessionState(GameSessionState::Running);
    m_state.setGameMode(GameMode::MainMenu);
}

void Game::enterEditor()
{
    clearWorld();

    if (!m_levelLoader)
        m_levelLoader = std::make_unique<LevelLoader>();

    LevelData level = m_levelLoader->loadDefaultLevel(m_rules);
    m_map = std::move(level.map);
    m_base = std::make_unique<Base>(level.baseCell);

    m_state.reset(0, 0);
    m_state.setSessionState(GameSessionState::Running);
    m_state.setGameMode(GameMode::Editing);
}

void Game::setInputSystem(InputSystem* input)
{
    m_inputSystem = input;
    if (m_player)
        m_player->setInput(m_inputSystem);
}

int Game::playerStars() const
{
    if (!m_player)
        return 0;

    return m_player->stars();
}

void Game::update(int deltaMs)
{
    if (m_state.gameMode() != GameMode::Playing)
        return;

    cleanupDestroyed();

    evaluateSessionState();
    if (m_state.isGameOver() || m_state.isVictory()) {
        m_state.setGameMode(GameMode::GameOver);
        return;
    }

    if (!m_map)
        return;

    updatePlayerRespawn(deltaMs);
    updateTanks(deltaMs);
    spawnPendingBullets();

    if (m_physicsSystem)
        m_physicsSystem->update(m_bullets, deltaMs);

    if (m_collisionSystem && m_map)
        m_collisionSystem->resolve(*m_map, m_tanks, m_bullets, m_base.get(), m_state);

    m_pendingBullets.clear();

    cleanupDestroyed(false);
    updateBonuses(deltaMs);
    evaluateSessionState();
    updateEnemySpawning(deltaMs);
}

void Game::clearWorld()
{
    for (Bonus* bonus : m_bonuses)
        delete bonus;
    m_bonuses.clear();

    for (Bullet* bullet : m_bullets)
        delete bullet;
    m_bullets.clear();
    m_pendingBullets.clear();

    for (Tank* tank : m_tanks)
        delete tank;
    m_tanks.clear();
    m_enemies.clear();
    m_enemySpawnOrder.clear();
    m_map.reset();
    m_base.reset();
    m_player = nullptr;
    m_playerSpawnCell = QPoint();
    m_playerRespawnTimerMs = 0;
    m_enemySpawnPoints.clear();
    m_bonusSpawnTimerMs = 0;
    m_enemyKillsSinceBonus = 0;
    m_enemyFreezeTimerMs = 0;
    m_nextEnemyTypeIndex = 0;
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

int Game::rollBonusSpawnIntervalMs() const
{
    return QRandomGenerator::global()->bounded(kBonusSpawnIntervalMinMs, kBonusSpawnIntervalMaxMs + 1);
}

void Game::addScoreForBonus()
{
    m_state.addScore(m_rules.scoreRules().bonus);
}

void Game::freezeEnemies(int durationMs)
{
    m_enemyFreezeTimerMs = qMax(m_enemyFreezeTimerMs, durationMs);
    applyEnemyFreezeState();
}

void Game::detonateEnemies()
{
    for (EnemyTank* enemy : m_enemies) {
        if (!enemy || enemy->isDestroyed())
            continue;

        enemy->health().takeDamage(enemy->health().health());
        enemy->markDestroyed();
    }
}

void Game::updateBonuses(int deltaMs)
{
    updateBonusEffects(deltaMs);

    if (m_state.sessionState() != GameSessionState::Running)
        return;

    if (!m_map)
        return;

    if (m_bonusSpawnTimerMs > 0) {
        m_bonusSpawnTimerMs = qMax(0, m_bonusSpawnTimerMs - deltaMs);
    }

    if (m_bonusSpawnTimerMs == 0)
        trySpawnBonus();

    for (Bonus* bonus : m_bonuses) {
        if (!bonus)
            continue;

        bonus->update(deltaMs);
    }

    handleBonusCollection();
    cleanupBonuses();
}

void Game::updateBonusEffects(int deltaMs)
{
    if (m_state.sessionState() != GameSessionState::Running)
        return;

    if (m_player)
        m_player->tickBonusEffects(deltaMs);

    if (m_enemyFreezeTimerMs > 0)
        m_enemyFreezeTimerMs = qMax(0, m_enemyFreezeTimerMs - deltaMs);

    applyEnemyFreezeState();
}

void Game::cleanupDestroyed(bool removeBullets)
{
    bool playerDestroyed = false;
    if (removeBullets) {
        for (qsizetype i = m_bullets.size(); i > 0; --i) {
            Bullet* bullet = m_bullets.at(i - 1);
            if (bullet && bullet->isAlive())
                continue;

            delete bullet;
            m_bullets.removeAt(i - 1);
        }
    }

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
            playerDestroyed = true;
        } else if (auto enemy = dynamic_cast<EnemyTank*>(tank)) {
            m_enemies.removeOne(enemy);
            m_state.registerEnemyDestroyed();
            m_state.addScore(m_rules.scoreRules().enemyKill);
            onEnemyDestroyed(*enemy);
            enemyDestroyed = true;
        }

        delete tank;
        m_tanks.removeAt(i - 1);
    }

    if (enemyDestroyed && m_state.enemiesToSpawn() > 0 && m_enemySpawnCooldownMs == 0)
        m_enemySpawnCooldownMs = m_enemyRespawnDelayMs;

    if (playerDestroyed && m_state.remainingLives() > 0) {
        m_playerRespawnTimerMs = m_playerRespawnDelayMs;
    }
}

void Game::updateEnemySpawning(int deltaMs)
{
    if (m_state.sessionState() != GameSessionState::Running)
        return;

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

        const EnemyType type = nextEnemyType();
        auto enemy = std::make_unique<EnemyTank>(cell, type);
        enemy->setDirection(Direction::Down);
        enemy->setMap(m_map.get());
        enemy->setFrozen(m_enemyFreezeTimerMs > 0);

        EnemyTank* enemyPtr = enemy.get();
        m_state.registerSpawnedEnemy();
        m_tanks.append(enemy.release());
        m_enemies.append(enemyPtr);
        return true;
    }

    return false;
}

EnemyType Game::nextEnemyType()
{
    if (m_enemySpawnOrder.isEmpty())
        return EnemyType::Basic;

    const qsizetype index = m_nextEnemyTypeIndex % m_enemySpawnOrder.size();
    const EnemyType type = m_enemySpawnOrder.at(index);
    m_nextEnemyTypeIndex = (m_nextEnemyTypeIndex + 1) % m_enemySpawnOrder.size();
    return type;
}

void Game::prepareEnemyQueue(int totalEnemies)
{
    static const QList<EnemyType> kPattern = {EnemyType::Basic, EnemyType::Fast, EnemyType::Armored, EnemyType::Power};
    m_enemySpawnOrder.clear();
    m_nextEnemyTypeIndex = 0;

    if (totalEnemies <= 0)
        return;

    for (int i = 0; i < totalEnemies; ++i) {
        const EnemyType type = kPattern.at(i % kPattern.size());
        m_enemySpawnOrder.append(type);
    }
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

bool Game::canSpawnBonusAt(const QPoint& cell) const
{
    if (!m_map || !m_map->isInside(cell))
        return false;

    const Tile tile = m_map->tile(cell);
    if (tile.blockMask != BlockNone)
        return false;

    if (m_base && m_base->cell() == cell)
        return false;

    for (Tank* tank : m_tanks) {
        if (!tank)
            continue;

        if (!tank->isDestructionFinished() && tank->cell() == cell)
            return false;
    }

    for (Bullet* bullet : m_bullets) {
        if (!bullet)
            continue;

        if (bullet->isAlive() && bullet->cell() == cell)
            return false;
    }

    for (Bonus* bonus : m_bonuses) {
        if (!bonus || bonus->isCollected())
            continue;

        if (bonus->cell() == cell)
            return false;
    }

    return true;
}

bool Game::canSpawnPlayerAt(const QPoint& cell) const
{
    if (!m_map || !m_map->isInside(cell))
        return false;

    if (!m_map->isWalkable(cell))
        return false;

    for (Tank* tank : m_tanks) {
        if (!tank)
            continue;

        if (!tank->isDestructionFinished() && tank->cell() == cell)
            return false;
    }

    return true;
}

void Game::handleBonusCollection()
{
    if (!m_player)
        return;

    for (Bonus* bonus : m_bonuses) {
        if (!bonus || bonus->isCollected())
            continue;

        if (bonus->cell() != m_player->cell())
            continue;

        bonus->apply(*this, *m_player);
        bonus->collect();
    }
}

std::unique_ptr<Bonus> Game::createRandomBonus(const QPoint& cell) const
{
    const int bonusRoll = QRandomGenerator::global()->bounded(4);
    switch (bonusRoll) {
    case 0:
        return std::make_unique<StarBonus>(cell);
    case 1:
        return std::make_unique<HelmetBonus>(cell);
    case 2:
        return std::make_unique<ClockBonus>(cell);
    case 3:
    default:
        return std::make_unique<GrenadeBonus>(cell);
    }
}

void Game::trySpawnBonus()
{
    if (!m_map || m_state.sessionState() != GameSessionState::Running)
        return;

    if (hasActiveBonus())
        return;

    QList<QPoint> freeCells;
    const QSize mapSize = m_map->size();
    for (int y = 0; y < mapSize.height(); ++y) {
        for (int x = 0; x < mapSize.width(); ++x) {
            const QPoint cell(x, y);
            if (canSpawnBonusAt(cell))
                freeCells.append(cell);
        }
    }

    if (!freeCells.isEmpty()) {
        const int index = QRandomGenerator::global()->bounded(freeCells.size());
        const QPoint spawnCell = freeCells.at(index);

        std::unique_ptr<Bonus> bonus = createRandomBonus(spawnCell);
        if (bonus) {
            m_bonuses.append(bonus.release());
            m_enemyKillsSinceBonus = 0;
        }
    }

    m_bonusSpawnTimerMs = rollBonusSpawnIntervalMs();
}

void Game::spawnBonusAtCell(const QPoint& cell)
{
    if (!m_map || m_state.sessionState() != GameSessionState::Running)
        return;

    if (!canSpawnBonusAt(cell))
        return;

    std::unique_ptr<Bonus> bonus = createRandomBonus(cell);
    if (bonus) {
        m_bonuses.append(bonus.release());
        m_enemyKillsSinceBonus = 0;
        m_bonusSpawnTimerMs = rollBonusSpawnIntervalMs();
    }
}

void Game::applyEnemyFreezeState()
{
    const bool freezeActive = m_enemyFreezeTimerMs > 0;
    for (EnemyTank* enemy : m_enemies) {
        if (!enemy)
            continue;

        enemy->setFrozen(freezeActive);
    }
}

void Game::cleanupBonuses()
{
    for (qsizetype i = m_bonuses.size(); i > 0; --i) {
        Bonus* bonus = m_bonuses.at(i - 1);
        if (!bonus)
            continue;

        if (bonus->isCollected()) {
            delete bonus;
            m_bonuses.removeAt(i - 1);
        }
    }
}

void Game::onEnemyDestroyed(EnemyTank& enemy)
{
    ++m_enemyKillsSinceBonus;

    if (enemy.dropsBonus()) {
        spawnBonusAtCell(enemy.cell());
        m_enemyKillsSinceBonus = 0;
        return;
    }

    if (m_enemyKillsSinceBonus >= kEnemyKillsPerBonus)
        trySpawnBonus();
}

bool Game::hasActiveBonus() const
{
    for (Bonus* bonus : m_bonuses) {
        if (bonus && !bonus->isCollected())
            return true;
    }

    return false;
}

void Game::updatePlayerRespawn(int deltaMs)
{
    if (m_player || m_state.remainingLives() <= 0)
        return;

    if (m_playerRespawnTimerMs > 0) {
        m_playerRespawnTimerMs = qMax(0, m_playerRespawnTimerMs - deltaMs);
        if (m_playerRespawnTimerMs > 0)
            return;
    }

    trySpawnPlayer();
}

void Game::trySpawnPlayer()
{
    if (!m_map)
        return;

    if (!canSpawnPlayerAt(m_playerSpawnCell)) {
        return;
    }

    auto player = std::make_unique<PlayerTank>(m_playerSpawnCell);
    player->setMap(m_map.get());
    player->setInput(m_inputSystem);

    m_player = player.get();
    m_tanks.append(player.release());
}

void Game::evaluateSessionState()
{
    if (m_state.sessionState() != GameSessionState::Running)
        return;

    if (m_state.isBaseDestroyed()) {
        setSessionState(GameSessionState::GameOver);
        return;
    }

    if (!m_player && m_state.remainingLives() <= 0) {
        setSessionState(GameSessionState::GameOver);
        return;
    }

    if (m_state.destroyedEnemies() >= m_state.totalEnemies() && m_state.aliveEnemies() == 0) {
        setSessionState(GameSessionState::Victory);
    }
}

void Game::setSessionState(GameSessionState state)
{
    if (m_state.sessionState() == state)
        return;

    m_state.setSessionState(state);

    switch (state) {
    case GameSessionState::Running:
        break;
    case GameSessionState::GameOver:
        m_state.setGameMode(GameMode::GameOver);
        break;
    case GameSessionState::Victory:
        m_state.setGameMode(GameMode::GameOver);
        break;
    }
}
