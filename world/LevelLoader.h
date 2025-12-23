#ifndef LEVELLOADER_H
#define LEVELLOADER_H

#include <QList>
#include <QPoint>
#include <QString>
#include <QStringList>
#include <memory>

#include "world/Map.h"
#include "core/GameRules.h"

struct LevelData
{
    std::unique_ptr<Map> map;
    QPoint playerSpawn;
    QList<QPoint> enemySpawns;
    QPoint baseCell;
    bool loadedFromFile = false;
};

/*
 * LevelLoader відповідає за побудову карти з ресурсів або
 * процедурних генераторів. Підтримує просте текстове
 * представлення, де кожен символ визначає тип тайлу/спавн.
 */
class LevelLoader
{
public:
    LevelData loadDefaultLevel(const GameRules& rules) const;
    LevelData loadLevelByName(const QString& fileName, const GameRules& rules) const;
    LevelData loadLevelByIndex(int index, const GameRules& rules) const;
    QStringList availableLevelFiles() const;
    LevelData loadFromText(const QStringList& lines, const GameRules& rules) const;
    LevelData loadSavedLevel(const GameRules& rules) const;
};

#endif // LEVELLOADER_H
