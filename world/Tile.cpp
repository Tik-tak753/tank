#include "world/Tile.h"

#include <QtGlobal>
#include <limits>

int Tile::maxDamage() const
{
    switch (type) {
    case TileType::Brick:
        return 4;
    case TileType::Steel:
        // Сталева стіна повинна витримувати будь-яку кількість влучань,
        // тому повертаємо "нескінченний" ліміт замість спеціальних умов у логіці.
        return std::numeric_limits<int>::max();
    case TileType::Empty:
    case TileType::Base:
    case TileType::Forest:
    case TileType::Water:
    case TileType::Ice:
        return 0;
    }

    Q_UNREACHABLE();
}

void Tile::takeDamage(int amount)
{
    if (amount <= 0)
        return;

    const int limit = maxDamage();
    if (limit <= 0)
        return;

    m_damage = qBound(0, m_damage + amount, limit);
}

bool Tile::isDestroyed() const
{
    const int limit = maxDamage();
    if (limit <= 0)
        return false;

    return m_damage >= limit;
}

Tile TileFactory::brick()
{
    // Цегла блокує і танки, і кулі, але має кінцевий запас міцності.
    // CollisionSystem лише читає mask+destructible, тому поведінка задається даними.
    Tile t;
    t.type = TileType::Brick;
    t.blockMask = BlockTank | BlockBullet;
    t.destructible = true;
    t.walkable = false;
    return t;
}

Tile TileFactory::steel()
{
    // Сталь теж блокує все, але не руйнується: це гарантує фіксовані бар'єри,
    // не потребуючи спеціальних умов у системі колізій.
    Tile t;
    t.type = TileType::Steel;
    t.blockMask = BlockTank | BlockBullet;
    t.destructible = false;
    t.walkable = false;
    return t;
}

Tile TileFactory::empty()
{
    Tile t;
    t.type = TileType::Empty;
    t.blockMask = BlockNone;
    t.destructible = false;
    t.walkable = true;
    return t;
}

Tile TileFactory::base()
{
    Tile t;
    t.type = TileType::Base;
    t.blockMask = BlockTank | BlockBullet;
    t.destructible = true;
    t.walkable = false;
    return t;
}

Tile TileFactory::forest()
{
    // Лісова клітинка прозора для куль і прохідна для техніки:
    // маска BlockNone дозволяє залишити рішення в даних, а не в коді.
    Tile t;
    t.type = TileType::Forest;
    t.blockMask = BlockNone;
    t.destructible = false;
    t.walkable = true;
    return t;
}

Tile TileFactory::water()
{
    // Вода зупиняє рух і снаряди, але не руйнується, що дозволяє стабільно контролювати маршрути.
    Tile t;
    t.type = TileType::Water;
    t.blockMask = BlockTank | BlockBullet;
    t.destructible = false;
    t.walkable = false;
    return t;
}

Tile TileFactory::ice()
{
    Tile t;
    t.type = TileType::Ice;
    t.blockMask = BlockNone;
    t.destructible = false;
    t.walkable = true;
    return t;
}
