#include "core/GameState.h"

#include <QtGlobal>

GameState::GameState()
{
}

void GameState::reset(int playerLives, int enemies)
{
    m_playerLives = playerLives;
    m_enemiesLeft = enemies;
    m_aliveEnemies = 0;
    m_baseDestroyed = false;
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

int GameState::aliveEnemies() const
{
    return m_aliveEnemies;
}

int GameState::enemiesToSpawn() const
{
    return qMax(0, m_enemiesLeft - m_aliveEnemies);
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
