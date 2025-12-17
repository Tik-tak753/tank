#ifndef LEVELLOADER_H
#define LEVELLOADER_H

#include <QList>
#include <QPoint>
#include <memory>

#include "world/Map.h"
#include "core/GameRules.h"

struct LevelData
{
    std::unique_ptr<Map> map;
    QPoint playerSpawn;
    QList<QPoint> enemySpawns;
    QPoint baseCell;
};

/*
 * LevelLoader відповідає за побудову карти з ресурсів або
 * процедурних генераторів. Поки що використовується вбудований
 * шаблон рівня, який імітує стартову арену Battle City.
 */
class LevelLoader
{
public:
    LevelData loadDefaultLevel(const GameRules& rules) const;
};

#endif // LEVELLOADER_H
