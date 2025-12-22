#include "gameplay/EnemyTank.h"

#include <QRandomGenerator>
#include <QVector>
#include <algorithm>
#include <array>

#include "world/Map.h"
#include "world/Tile.h"

namespace {
constexpr int kFireJitterMs = 200;
} // namespace

EnemyTank::EnemyTank(const QPoint& cell, EnemyType type)
    : Tank(cell)
    , m_enemyType(type)
    , m_stats(statsForType(type))
{
    setDirection(Direction::Down);
    setType(TankType::Enemy);
    applyStats();
    resetFireInterval();
}

void EnemyTank::update()
{
    updateWithDelta(16);
}

void EnemyTank::updateWithDelta(int deltaMs)
{
    Tank::updateWithDelta(deltaMs);

    if (m_hitFeedbackTimerMs > 0)
        m_hitFeedbackTimerMs = std::max(0, m_hitFeedbackTimerMs - deltaMs);

    if (isDestroyed())
        return;

    if (m_frozen)
        return;

    m_moveElapsedMs += deltaMs;

    m_fireElapsedMs += deltaMs;
    if (m_fireElapsedMs >= m_fireIntervalMs) {
        requestFire();
        m_fireElapsedMs = 0;
        resetFireInterval();
    }

    if (stepIntervalMs() <= 0)
        return;

    m_stepAccumulatorMs += deltaMs;

    while (m_stepAccumulatorMs >= stepIntervalMs()) {
        m_stepAccumulatorMs -= stepIntervalMs();

        const bool needsDirectionUpdate = isAlignedToGrid() && ((m_moveElapsedMs >= m_moveIntervalMs && !m_sliding) || !canMove(direction()));
        if (needsDirectionUpdate) {
            m_moveElapsedMs = 0;
            tryMove();
        }

        const QPoint step = directionDelta();
        const QPoint nextCell = cell() + step;
        const bool canAdvance = (m_map && m_map->isWalkable(nextCell));
        if (!canAdvance) {
            m_stepAccumulatorMs = 0;
            m_subTileProgress = 0;
            m_sliding = false;
            updateRenderPosition(direction());
            m_moveElapsedMs = m_moveIntervalMs; // force new direction check on next aligned tick
            break;
        }

        ++m_subTileProgress;
        updateRenderPosition(direction());

        if (m_subTileProgress >= kStepsPerTile) {
            setCell(nextCell);
            m_sliding = shouldSlide();
        }
    }
}

QPoint EnemyTank::directionDelta() const
{
    return Tank::directionDelta(direction());
}

QPoint EnemyTank::directionDelta(Direction direction) const
{
    return Tank::directionDelta(direction);
}

bool EnemyTank::receiveDamage(int dmg)
{
    const int previousHealth = health().health();
    const bool damaged = Tank::receiveDamage(dmg);
    if (damaged && health().health() < previousHealth && health().isAlive())
        triggerHitFeedback();
    return damaged;
}

Direction EnemyTank::oppositeDirection() const
{
    switch (direction()) {
    case Direction::Up:    return Direction::Down;
    case Direction::Down:  return Direction::Up;
    case Direction::Left:  return Direction::Right;
    case Direction::Right: return Direction::Left;
    }
    return Direction::Down;
}

Direction EnemyTank::randomDirection(Direction exclude) const
{
    Direction newDirection = direction();
    do {
        const int value = QRandomGenerator::global()->bounded(4);
        switch (value) {
        case 0: newDirection = Direction::Up; break;
        case 1: newDirection = Direction::Down; break;
        case 2: newDirection = Direction::Left; break;
        case 3: newDirection = Direction::Right; break;
        }
    } while (newDirection == exclude);

    return newDirection;
}

bool EnemyTank::canMove(Direction direction) const
{
    if (!m_map)
        return false;

    const QPoint nextCell = cell() + directionDelta(direction);
    if (!m_map->isInside(nextCell))
        return false;

    return m_map->isWalkable(nextCell);
}

bool EnemyTank::shouldSlide() const
{
    if (!m_map)
        return false;

    const Tile tile = m_map->tile(cell());
    if (tile.type != TileType::Ice)
        return false;

    const QPoint nextCell = cell() + directionDelta();
    return m_map->isWalkable(nextCell);
}

void EnemyTank::tryMove()
{
    if (!m_map)
        return;

    QVector<Direction> availableDirections;
    availableDirections.reserve(4);
    for (Direction dir : {Direction::Up, Direction::Down, Direction::Left, Direction::Right}) {
        if (canMove(dir))
            availableDirections.append(dir);
    }

    if (availableDirections.isEmpty())
        return;

    Direction current = direction();
    const bool blocked = !canMove(current);
    if (blocked || QRandomGenerator::global()->bounded(100) < 25) {
        QVector<Direction> candidates = availableDirections;

        // уникаємо розворотів на 180 градусів, якщо є інші опції
        const Direction opposite = oppositeDirection();
        if (candidates.size() > 1)
            candidates.removeOne(opposite);

        if (!candidates.isEmpty())
            current = candidates.at(QRandomGenerator::global()->bounded(candidates.size()));
        else
            current = opposite;
        setDirection(current);
    }

    setDirection(current);
    m_sliding = shouldSlide();
}

void EnemyTank::resetFireInterval()
{
    const int jitter = qMax(0, kFireJitterMs);
    const int minInterval = qMax(50, m_stats.fireCooldownMs - jitter);
    const int maxInterval = m_stats.fireCooldownMs + jitter + 1;
    m_fireIntervalMs = QRandomGenerator::global()->bounded(minInterval, maxInterval);
}

void EnemyTank::triggerHitFeedback()
{
    constexpr int kHitFeedbackDurationMs = 120;
    m_hitFeedbackTimerMs = kHitFeedbackDurationMs;
}

void EnemyTank::applyStats()
{
    const float tilesPerSecond = tilesPerSecondFromStats(m_stats);
    setSpeed(tilesPerSecond);
    m_fireIntervalMs = m_stats.fireCooldownMs;
    m_weapon.setReloadTime(m_stats.fireCooldownMs);

    m_health.setMaxHealth(m_stats.armorHits);
    m_health.setLives(1);
}

const EnemyStats& EnemyTank::statsForType(EnemyType type)
{
    static const std::array<EnemyStats, 4> kEnemyStatsTable = {{
        // Basic
        {kStepsPerTile, 16, 1, false, 1000, QColor(100, 120, 180), QColor(120, 140, 200)},
        // Fast
        {kStepsPerTile, 12, 1, false, 1000, QColor(180, 140, 120), QColor(200, 160, 140)},
        // Armored
        {kStepsPerTile, 16, 3, false, 1200, QColor(90, 90, 120), QColor(200, 170, 110)},
        // Power
        {kStepsPerTile, 16, 1, true, 1000, QColor(120, 150, 90), QColor(140, 170, 110)},
    }};

    const int index = static_cast<int>(type);
    const int maxIndex = static_cast<int>(kEnemyStatsTable.size() - 1);
    const int clampedIndex = std::clamp(index, 0, maxIndex);
    return kEnemyStatsTable.at(clampedIndex);
}

float EnemyTank::tilesPerSecondFromStats(const EnemyStats& stats)
{
    if (stats.stepsPerTile <= 0 || stats.stepIntervalMs <= 0)
        return kDefaultTilesPerSecond;

    constexpr float millisPerSecond = 1000.0f;
    return millisPerSecond / (static_cast<float>(stats.stepsPerTile * stats.stepIntervalMs));
}

QColor EnemyTank::currentColor() const
{
    if (isHitFeedbackActive())
        return QColor(230, 230, 230);

    const bool armorDamaged = m_stats.armorHits > 1 && health().health() < m_stats.armorHits;
    if (armorDamaged && m_stats.damagedColor.isValid())
        return m_stats.damagedColor;

    if (m_stats.baseColor.isValid())
        return m_stats.baseColor;

    return QColor(100, 120, 180);
}
