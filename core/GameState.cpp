#include "core/GameState.h"

GameState::GameState()
{
}

void GameState::reset(int playerLives, int enemies)
{
    m_playerLives = playerLives;
    m_enemiesLeft = enemies;
    m_baseDestroyed = false;
}

void GameState::setBaseDestroyed()
{
    m_baseDestroyed = true;
}

void GameState::registerSpawnedEnemy()
{
    // майбутні хвилі можуть збільшувати кількість ворогів у грі
    ++m_enemiesLeft;
}

void GameState::registerEnemyDestroyed()
{
    if (m_enemiesLeft > 0)
        --m_enemiesLeft;
}

void GameState::registerPlayerLostLife()
{
    if (m_playerLives > 0)
        --m_playerLives;
}

int GameState::remainingLives() const
{
    return m_playerLives;
}

int GameState::remainingEnemies() const
{
    return m_enemiesLeft;
}

bool GameState::isBaseDestroyed() const
{
    return m_baseDestroyed;
}

bool GameState::isGameOver() const
{
    return m_baseDestroyed || m_playerLives <= 0;
}

bool GameState::isVictory() const
{
    return !m_baseDestroyed && m_enemiesLeft == 0 && m_playerLives > 0;
}
