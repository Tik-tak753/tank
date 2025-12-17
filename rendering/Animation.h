#ifndef ANIMATION_H
#define ANIMATION_H

#include <QVector>
#include <QString>

/*
 * Animation тримає перелік кадрів та просту індикацію прогресу.
 */
class Animation
{
public:
    void addFrame(const QString& frame) { m_frames.append(frame); }
    QString currentFrame() const;
    void step();

private:
    QVector<QString> m_frames;
    int m_index = 0;
};

#endif // ANIMATION_H
