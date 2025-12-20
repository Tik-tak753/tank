#ifndef GAMESTATE_H
#define GAMESTATE_H

enum class GameSessionState {
    Running,
    GameOver,
    Victory,
};

/*
 * Клас GameState зберігає поточний стан гри
 * (життя гравця, залишок ворогів, стан бази тощо).
 */
class GameState
{
public:
    GameState();

    // Ініціалізація поточного стану новою сесією
    void reset(int playerLives, int enemies);

    // Оновлення подій гри
    void setBaseDestroyed();
    void registerSpawnedEnemy();
    void registerEnemyDestroyed();
    void registerPlayerLostLife();
    void setSessionState(GameSessionState state);

    // Доступ до агрегованих даних
    int remainingLives() const;
    int remainingEnemies() const;
    int aliveEnemies() const;
    int enemiesToSpawn() const;
    int totalEnemies() const;
    int destroyedEnemies() const;
    bool isBaseDestroyed() const;
    GameSessionState sessionState() const;

    // Перевірка умов завершення
    bool isGameOver() const;
    bool isVictory() const;

private:
    int m_playerLives = 0;
    int m_enemiesLeft = 0;
    int m_aliveEnemies = 0;
    int m_totalEnemies = 0;
    int m_destroyedEnemies = 0;
    bool m_baseDestroyed = false;
    GameSessionState m_sessionState = GameSessionState::Running;
};

#endif // GAMESTATE_H
