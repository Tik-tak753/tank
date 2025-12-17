#include "rendering/SpriteManager.h"

void SpriteManager::registerSprite(const QString& key, const QString& path)
{
    m_sprites.insert(key, path);
}

QString SpriteManager::spritePath(const QString& key) const
{
    return m_sprites.value(key);
}
