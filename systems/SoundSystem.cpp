#include "systems/SoundSystem.h"

#include <QDebug>

void SoundSystem::playShot(const QString& asset)
{
    qDebug() << "Play shot sound" << asset;
}

void SoundSystem::playExplosion(const QString& asset)
{
    qDebug() << "Play explosion sound" << asset;
}
