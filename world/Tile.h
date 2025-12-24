#ifndef TILE_H
#define TILE_H

#include <QPoint>
#include <cstdint>

/*
 * Tile описує тип клітинки карти та базові властивості
 * прохідності/руйнування.
 */
enum class TileType {
    Empty,
    Brick,      // цегляна стіна — руйнується
    Steel,      // сталева стіна — не руйнується
    Base,       // база гравця
    Forest,
    Water,
    Ice
};

// Маски блокувань описують, які класи об'єктів може зупиняти клітинка.
// Логіка не потребує знати конкретний TileType: достатньо перевірити маски.
enum CollisionMask : std::uint8_t {
    BlockNone   = 0,
    BlockTank   = 1 << 0,
    BlockBullet = 1 << 1
};

inline constexpr CollisionMask operator|(CollisionMask lhs, CollisionMask rhs)
{
    return static_cast<CollisionMask>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
}

inline constexpr CollisionMask operator&(CollisionMask lhs, CollisionMask rhs)
{
    return static_cast<CollisionMask>(static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs));
}

class Tile
{
public:
    TileType type = TileType::Empty;
    CollisionMask blockMask = BlockNone;
    bool destructible = false;
    bool walkable = true;
    bool pierceable = false;
    int reinforcedMaxDamage = 0;

    int damage() const { return m_damage; }
    int maxDamage() const;
    void takeDamage(int amount);
    bool isDestroyed() const;

    bool isSteel() const { return type == TileType::Steel; }

private:
    int m_damage = 0;        // використовується для Brick / Steel
};

// Фабрика описує властивості тайлів як дані,
// щоб системи (рух/колізії) могли працювати лише з масками та прапорцями.
namespace TileFactory {
// Кожен генератор повертає готову конфігурацію тайлу певного типу
Tile brick();
Tile steel();
Tile empty();
Tile base();
Tile forest();
Tile water();
Tile ice();
}

#endif // TILE_H
