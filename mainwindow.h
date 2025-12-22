#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <memory>

class QGraphicsScene;
class QGraphicsView;
class QTimer;
class QKeyEvent;
class QGraphicsTextItem;

class Game;
class InputSystem;
class Renderer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    static constexpr int kFixedTickMs = 16;
    void startGame();
    void pauseGame();
    void resumeGame();
    void returnToMainMenu();
    void updateMenuOverlays();
    void clearGameOverOverlay();

    // View / Scene
    QGraphicsScene* m_scene = nullptr;
    QGraphicsView* m_view = nullptr;

    // Game
    std::unique_ptr<Game> m_game;
    std::unique_ptr<InputSystem> m_input;
    std::unique_ptr<Renderer> m_renderer;
    QGraphicsTextItem* m_gameOverItem = nullptr;
    QGraphicsTextItem* m_mainMenuItem = nullptr;
    QGraphicsTextItem* m_pauseMenuItem = nullptr;

    // Timer
    QTimer* m_timer = nullptr;
    qint64 m_frameAccumulatorMs = 0;
    QElapsedTimer m_frameTimer;
};

#endif // MAINWINDOW_H
