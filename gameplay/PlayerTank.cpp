#include "gameplay/PlayerTank.h"

#include "systems/InputSystem.h"
#include "world/Map.h"
#include <optional>
#include <QtGlobal>
#include "world/Tile.h"

namespace {
bool shouldSlideOnIce(const Map* map, const QPoint& cell, Direction direction)
{
    if (!map)
        return false;

    const Tile tile = map->tile(cell);
    if (tile.type != TileType::Ice)
        return false;
    const QPoint nextCell = cell + Tank::directionDelta(direction);
    return map->isWalkable(nextCell);
}

// Рух і швидкість пострілів підігнані під NES-версію:
// базовий танк минає плитку приблизно за 256 мс (3.9 плитки/с),
// зірки піднімають темп до 4.5 / 5.2 / 5.8 плиток/с,
// кулі летять удвічі швидше (130 мс) і прискорюються до 110 / 95 / 85 мс зі зірками.
constexpr float kStar1SpeedTilesPerSecond = 4.5f;
constexpr float kStar2SpeedTilesPerSecond = 5.2f;
constexpr float kStar3SpeedTilesPerSecond = 5.8f;

constexpr int kStar1BulletStepIntervalMs = 110;
constexpr int kStar2BulletStepIntervalMs = 95;
constexpr int kStar3BulletStepIntervalMs = 85;
}

PlayerTank::PlayerTank(const QPoint& cell)
    : Tank(cell)
{
    m_health.setLives(3);
    setType(TankType::Player);
    setSpeed(movementSpeedTilesPerSecond());
    applyUpgrades();
}

void PlayerTank::addStar()
{
    if (m_stars >= kMaxStars)
        return;

    ++m_stars;
    applyUpgrades();
}

bool PlayerTank::canPierceSteel() const
{
    return m_stars >= kMaxStars;
}

void PlayerTank::setInput(InputSystem* input)
{
    m_input = input;
}

void PlayerTank::setMap(const Map* map)
{
    m_map = map;
}

void PlayerTank::update()
{
    updateWithDelta(16);
}

void PlayerTank::updateWithDelta(int deltaMs)
{
    Tank::updateWithDelta(deltaMs);

    if (isDestroyed())
        return;

    if (!m_input)
        return;

    const std::optional<Direction> desired = m_input->currentDirection();
    if (!m_sliding && !desired.has_value())
        return;

    if (!m_sliding && desired.has_value() && isAlignedToGrid())
        setDirection(*desired);

    if (stepIntervalMs() <= 0)
        return;

    m_stepAccumulatorMs += deltaMs;

    while (m_stepAccumulatorMs >= stepIntervalMs()) {
        m_stepAccumulatorMs -= stepIntervalMs();

        if (m_subTileProgress == 0 && desired.has_value() && direction() != *desired)
            setDirection(*desired);

        const QPoint step = Tank::directionDelta(direction());
        const QPoint nextCell = cell() + step;

        const bool canAdvance = (!m_map || m_map->isWalkable(nextCell));
        if (!canAdvance) {
            m_stepAccumulatorMs = 0;
            m_subTileProgress = 0;
            m_sliding = false;
            updateRenderPosition(direction());
            break;
        }

        ++m_subTileProgress;
        updateRenderPosition(direction());

        if (m_subTileProgress >= kStepsPerTile) {
            setCell(nextCell);
            m_sliding = shouldSlideOnIce(m_map, cell(), direction());
        }
    }

    if (m_input->consumeFire())
        requestFire();
}

int PlayerTank::bulletStepIntervalMs() const
{
    return bulletStepIntervalForStars();
}

bool PlayerTank::bulletCanPierceSteel() const
{
    return canPierceSteel();
}

void PlayerTank::applyUpgrades()
{
    setSpeed(movementSpeedTilesPerSecond());
    m_weapon.setReloadTime(reloadTimeMs());
}

int PlayerTank::reloadTimeMs() const
{
    if (m_stars >= 2)
        return 250;

    return 400;
}

bool PlayerTank::receiveDamage(int dmg)
{
    if (isInvincible())
        return false;

    return Tank::receiveDamage(dmg);
}

void PlayerTank::activateInvincibility(int durationMs)
{
    m_invincibilityTimerMs = qMax(m_invincibilityTimerMs, durationMs);
}

void PlayerTank::tickBonusEffects(int deltaMs)
{
    updateInvincibility(deltaMs);
}

float PlayerTank::movementSpeedTilesPerSecond() const
{
    if (m_stars >= 3)
        return kStar3SpeedTilesPerSecond;
    if (m_stars == 2)
        return kStar2SpeedTilesPerSecond;
    if (m_stars == 1)
        return kStar1SpeedTilesPerSecond;

    return kDefaultTilesPerSecond;
}

int PlayerTank::bulletStepIntervalForStars() const
{
    if (m_stars >= 3)
        return kStar3BulletStepIntervalMs;
    if (m_stars == 2)
        return kStar2BulletStepIntervalMs;
    if (m_stars == 1)
        return kStar1BulletStepIntervalMs;

    return Tank::bulletStepIntervalMs();
}

void PlayerTank::updateInvincibility(int deltaMs)
{
    if (m_invincibilityTimerMs <= 0)
        return;

    m_invincibilityTimerMs = qMax(0, m_invincibilityTimerMs - deltaMs);
}
