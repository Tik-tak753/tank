#ifndef GAMELOOP_H
#define GAMELOOP_H

#include <QObject>
#include <QTimer>
#include <functional>

/*
 * GameLoop відповідає за стабільний виклик логіки оновлення.
 * Він інкапсулює QTimer, щоб надалі легко переносити
 * ігровий цикл у окремий потік або інший бекенд.
 */
class GameLoop : public QObject
{
    Q_OBJECT
public:
    explicit GameLoop(QObject* parent = nullptr);

    void setTickCallback(const std::function<void()>& callback);
    void setInterval(int ms);

    void start();
    void stop();

private:
    QTimer m_timer;
    std::function<void()> m_tick;
};

#endif // GAMELOOP_H
