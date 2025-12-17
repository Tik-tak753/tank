#include "rendering/Animation.h"

QString Animation::currentFrame() const
{
    if (m_frames.isEmpty())
        return QString();
    return m_frames.at(m_index % m_frames.size());
}

void Animation::step()
{
    if (m_frames.isEmpty())
        return;
    ++m_index;
}
