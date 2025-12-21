#ifndef GAMERULES_H
#define GAMERULES_H

#include <QSize>
#include <QPoint>

struct ScoreRules
{
    int enemyKill = 100;
    int bonus = 500;
    int stageClear = 1000;
};

/*
 * GameRules визначають параметри сесії: розмір поля,
 * кількість життів, генерацію хвиль ворогів.
 */
class GameRules
{
public:
    QSize mapSize() const { return m_mapSize; }
    int playerLives() const { return m_playerLives; }
    int enemiesPerWave() const { return m_enemiesPerWave; }
    int totalWaves() const { return m_totalWaves; }
    QPoint baseCell() const { return m_baseCell; }
    const ScoreRules& scoreRules() const { return m_scoreRules; }

    void setMapSize(const QSize& size);
    void setPlayerLives(int lives);
    void setEnemiesPerWave(int enemies);
    void setTotalWaves(int waves);
    void setBaseCell(const QPoint& cell);
    void setScoreRules(const ScoreRules& rules);

private:
    QSize m_mapSize = QSize(26, 26);
    int m_playerLives = 3;
    int m_enemiesPerWave = 4;
    int m_totalWaves = 5;
    QPoint m_baseCell = QPoint(12, 24);
    ScoreRules m_scoreRules;
};

#endif // GAMERULES_H
