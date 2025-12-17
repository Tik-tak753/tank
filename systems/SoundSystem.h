#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

#include <QString>

/*
 * SoundSystem — обгортка для відтворення звуків пострілів/вибухів.
 * Поки що це заглушка, яку можна підключити до QSoundEffect.
 */
class SoundSystem
{
public:
    void playShot(const QString& /*asset*/);
    void playExplosion(const QString& /*asset*/);
};

#endif // SOUNDSYSTEM_H
