#include "core/GameState.h"

#include <QtGlobal>

GameState::GameState()
{
}

void GameState::reset(int playerLives, int enemies)
{
    m_playerLives = playerLives;
    m_enemiesLeft = enemies;
    m_totalEnemies = enemies;
    m_destroyedEnemies = 0;
    m_aliveEnemies = 0;
    m_baseDestroyed = false;
    m_sessionState = GameSessionState::Running;
}

void GameState::setBaseDestroyed()
{
    m_baseDestroyed = true;
}

void GameState::registerSpawnedEnemy()
{
    ++m_aliveEnemies; // spawning an enemy only affects alive count
}

void GameState::registerEnemyDestroyed()
{
    if (m_aliveEnemies > 0)
        --m_aliveEnemies;

    if (m_enemiesLeft > 0)
        --m_enemiesLeft;

    ++m_destroyedEnemies;
}

void GameState::registerPlayerLostLife()
{
    if (m_playerLives > 0)
        --m_playerLives;
}

void GameState::setSessionState(GameSessionState state)
{
    m_sessionState = state;
}

int GameState::remainingLives() const
{
    return m_playerLives;
}

int GameState::remainingEnemies() const
{
    return m_enemiesLeft;
}

int GameState::aliveEnemies() const
{
    return m_aliveEnemies;
}

int GameState::enemiesToSpawn() const
{
    return qMax(0, m_enemiesLeft - m_aliveEnemies);
}

int GameState::totalEnemies() const
{
    return m_totalEnemies;
}

int GameState::destroyedEnemies() const
{
    return m_destroyedEnemies;
}

bool GameState::isBaseDestroyed() const
{
    return m_baseDestroyed;
}

GameSessionState GameState::sessionState() const
{
    return m_sessionState;
}

bool GameState::isGameOver() const
{
    return m_sessionState == GameSessionState::GameOver || m_baseDestroyed || m_playerLives <= 0;
}

bool GameState::isVictory() const
{
    return m_sessionState == GameSessionState::Victory || (!m_baseDestroyed && m_enemiesLeft == 0 && m_playerLives > 0);
}
