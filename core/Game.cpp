#include "core/Game.h"

#include "gameplay/EnemyTank.h"
#include "gameplay/PlayerTank.h"
#include "gameplay/Bullet.h"
#include "rendering/Renderer.h"
#include "systems/InputSystem.h"
#include "world/Base.h"
#include "world/LevelLoader.h"
#include "world/Map.h"

Game::Game(QObject* parent)
    : QObject(parent),
      m_loop(this)
{
    // Під’єднуємо callback до GameLoop, але поки не запускаємо його
    m_loop.setTickCallback([this]() { update(); });
}

Game::~Game()
{
    for (Bullet* bullet : m_bullets)
        delete bullet;
    for (Tank* tank : m_tanks)
        delete tank;
}

void Game::initialize()
{
    // На цьому кроці в майбутньому будемо створювати карту, танки та базу.
    // Поки що готуємо лише базовий стан сесії.

    // Очистка попередніх даних, якщо ініціалізація виконується повторно
    for (Bullet* bullet : m_bullets)
        delete bullet;
    m_bullets.clear();
    for (Tank* tank : m_tanks)
        delete tank;
    m_tanks.clear();
    m_enemies.clear();
    m_map.reset();
    m_base.reset();

    m_state.reset(m_rules.playerLives(), m_rules.enemiesPerWave());
    m_tickCounter = 0;
    m_initialized = true;
}

void Game::start()
{
    if (!m_initialized)
        initialize();

    if (m_running)
        return;

    // Запуск циклу — окрема фаза після ініціалізації
    m_loop.start();
    m_running = true;
}

void Game::stop()
{
    if (!m_running)
        return;

    m_loop.stop();
    m_running = false;
}

void Game::update()
{
    // Не виконуємо оновлення, якщо гра ще не стартувала
    if (!m_running)
        return;

    ++m_tickCounter;

    // Легка емуляція прогресу: кожні 30 тік ворог "знищується"
    if (m_tickCounter % 30 == 0 && m_state.remainingEnemies() > 0)
        m_state.registerEnemyDestroyed();

    // Перевірка завершення — окрема фаза, де ми зупиняємо цикл
    if (m_state.isGameOver() || m_state.isVictory()) {
        stop();
        return;
    }

    // Майбутнє місце для інтеграції з рендерингом та системами
    // if (m_renderer)
    //     m_renderer->renderFrame(*this);
}

void Game::setRenderer(Renderer* renderer)
{
    m_renderer = renderer;
}

void Game::setInputSystem(InputSystem* input)
{
    m_inputSystem = input;
}
