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
    // Завантажує дефолтну карту, побудовану за поточними правилами
    LevelData loadDefaultLevel(const GameRules& rules) const;
    // Читає рівень із файлу за ім'ям, якщо він існує
    LevelData loadLevelByName(const QString& fileName, const GameRules& rules) const;
    // Вибирає карту за індексом у списку доступних файлів
    LevelData loadLevelByIndex(int index, const GameRules& rules) const;
    // Повертає перелік текстових карт із каталогу
    QStringList availableLevelFiles() const;
    // Створює рівень із рядків текстової матриці
    LevelData loadFromText(const QStringList& lines, const GameRules& rules) const;
    // Пробує прочитати останню збережену карту редактора
    LevelData loadSavedLevel(const GameRules& rules) const;
};

#endif // LEVELLOADER_H
