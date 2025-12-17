#ifndef SPRITEMANAGER_H
#define SPRITEMANAGER_H

#include <QString>
#include <QHash>

/*
 * SpriteManager зберігає шляхи до текстур і анімацій.
 */
class SpriteManager
{
public:
    void registerSprite(const QString& key, const QString& path);
    QString spritePath(const QString& key) const;

private:
    QHash<QString, QString> m_sprites;
};

#endif // SPRITEMANAGER_H
