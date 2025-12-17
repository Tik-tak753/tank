#include "core/GameRules.h"

void GameRules::setMapSize(const QSize& size)
{
    m_mapSize = size;
}

void GameRules::setPlayerLives(int lives)
{
    m_playerLives = lives;
}

void GameRules::setEnemiesPerWave(int enemies)
{
    m_enemiesPerWave = enemies;
}

void GameRules::setTotalWaves(int waves)
{
    m_totalWaves = waves;
}

void GameRules::setBaseCell(const QPoint& cell)
{
    m_baseCell = cell;
}
