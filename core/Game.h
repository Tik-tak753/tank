#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QList>
#include <memory>

#include "core/GameState.h"
#include "core/GameRules.h"
#include "core/GameLoop.h"

class Tank;
class PlayerTank;
class EnemyTank;
class Bullet;
class Renderer;
class InputSystem;
class LevelLoader;
class Map;
class Base;

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

    // Запуск ігрового циклу
    void start();
    void stop();

    // Крок оновлення (викликається GameLoop)
    void update();

    const GameState& state() const { return m_state; }
    GameRules& rules() { return m_rules; }

    QList<Tank*> tanks() const { return m_tanks; }
    QList<Bullet*> bullets() const { return m_bullets; }

    Map* map() const { return m_map.get(); }
    Base* base() const { return m_base.get(); }

    // Для інтеграції з UI
    void setRenderer(Renderer* renderer);
    void setInputSystem(InputSystem* input);

private:
    // Прапорці для розділення фаз життєвого циклу
    bool m_initialized = false;
    bool m_running = false;
    int m_tickCounter = 0;

    GameRules m_rules;
    GameState m_state;
    GameLoop m_loop;

    std::unique_ptr<Map> m_map;
    std::unique_ptr<Base> m_base;

    QList<Tank*> m_tanks;
    QList<EnemyTank*> m_enemies;
    QList<Bullet*> m_bullets;

    Renderer* m_renderer = nullptr;
    InputSystem* m_inputSystem = nullptr;

    std::unique_ptr<LevelLoader> m_levelLoader;
};

#endif // GAME_H
