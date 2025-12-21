#include "gameplay/Bonus.h"

#include <QPointF>
#include <QtGlobal>

#include "core/Game.h"
#include "gameplay/PlayerTank.h"

namespace {
constexpr int kTimedBonusDurationMs = 10000;
}

Bonus::Bonus(const QPoint& cell, BonusType type)
    : GameObject(QPointF(cell)),
      m_type(type)
{
}

void Bonus::collect()
{
    m_collected = true;
}

void Bonus::update(int deltaMs)
{
    GameObject::update(deltaMs);
}

StarBonus::StarBonus(const QPoint& cell)
    : Bonus(cell, BonusType::Star)
{
}

void StarBonus::apply(Game& game, PlayerTank& player)
{
    player.addStar();
    game.addScoreForBonus();
}

HelmetBonus::HelmetBonus(const QPoint& cell)
    : Bonus(cell, BonusType::Helmet)
{
}

void HelmetBonus::apply(Game& game, PlayerTank& player)
{
    player.activateInvincibility(kTimedBonusDurationMs);
    game.addScoreForBonus();
}

ClockBonus::ClockBonus(const QPoint& cell)
    : Bonus(cell, BonusType::Clock)
{
}

void ClockBonus::apply(Game& game, PlayerTank& player)
{
    Q_UNUSED(player);
    game.freezeEnemies(kTimedBonusDurationMs);
    game.addScoreForBonus();
}

GrenadeBonus::GrenadeBonus(const QPoint& cell)
    : Bonus(cell, BonusType::Grenade)
{
}

void GrenadeBonus::apply(Game& game, PlayerTank& player)
{
    Q_UNUSED(player);
    game.detonateEnemies();
    game.addScoreForBonus();
}
