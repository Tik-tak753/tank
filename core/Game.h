#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QList>
#include <vector>
#include <memory>

#include "core/GameState.h"
#include "core/GameRules.h"

class Tank;
class PlayerTank;
class EnemyTank;
class Bullet;
class InputSystem;
class LevelLoader;
class Map;
class Base;
class PhysicsSystem;
class CollisionSystem;

/*
 * Game — центральний фасад, який зшиває усі підсистеми разом.
 * На цьому рівні відбувається ініціалізація рівня, запуск циклу
 * та комунікація між логікою, AI, фізикою й відтворенням.
 */
class Game : public QObject
{
    Q_OBJECT
public:
    explicit Game(QObject* parent = nullptr);
    ~Game();

    // Підготовка базового рівня та створення сутностей
    void initialize();
    void restart();

    // Крок оновлення (викликається MainWindow)
    void update(int deltaMs);

    const GameState& state() const { return m_state; }
    GameRules& rules() { return m_rules; }

    QList<Tank*> tanks() const { return m_tanks; }
    QList<Bullet*> bullets() const { return m_bullets; }

    Map* map() const { return m_map.get(); }
    Base* base() const { return m_base.get(); }

    void setInputSystem(InputSystem* input);
    PlayerTank* player() const { return m_player; }

private:
    void clearWorld();
    void updateTanks(int deltaMs);
    void spawnPendingBullets();
    void removeDeadTanks();

    GameRules m_rules;
    GameState m_state;

    std::unique_ptr<Map> m_map;
    std::unique_ptr<Base> m_base;
    std::unique_ptr<LevelLoader> m_levelLoader;

    QList<Tank*> m_tanks;
    QList<EnemyTank*> m_enemies;
    QList<Bullet*> m_bullets;
    std::vector<std::unique_ptr<Bullet>> m_pendingBullets;

    InputSystem* m_inputSystem = nullptr;
    PlayerTank* m_player = nullptr;

    std::unique_ptr<PhysicsSystem> m_physicsSystem;
    std::unique_ptr<CollisionSystem> m_collisionSystem;
};

#endif // GAME_H
