#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

class QGraphicsScene;
class QGraphicsView;
class QTimer;
class QKeyEvent;

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
    // View / Scene
    QGraphicsScene* m_scene = nullptr;
    QGraphicsView* m_view = nullptr;

    // Game
    std::unique_ptr<Game> m_game;
    std::unique_ptr<InputSystem> m_input;
    std::unique_ptr<Renderer> m_renderer;

    // Timer
    QTimer* m_timer = nullptr;
};

#endif // MAINWINDOW_H
