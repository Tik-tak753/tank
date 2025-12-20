#include "mainwindow.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QFont>
#include <QTimer>
#include <QPoint>
#include <QKeyEvent>
#include <QSize>
#include <QColor>

#include "core/Game.h"
#include "systems/InputSystem.h"
#include "rendering/Renderer.h"
#include "utils/Constants.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    /* =====================
     * Scene / View
     * ===================== */

    m_scene = new QGraphicsScene(this);

    m_view = new QGraphicsView(m_scene, this);
    setCentralWidget(m_view);
    resize(800, 600);
    setFocusPolicy(Qt::StrongFocus);
    m_view->setFocus();

    /* =====================
     * Game
     * ===================== */

    m_game = std::make_unique<Game>(this);
    m_input = std::make_unique<InputSystem>();

    GameRules& rules = m_game->rules();
    rules.setMapSize(QSize(GRID_WIDTH, GRID_HEIGHT));
    rules.setBaseCell(QPoint(GRID_WIDTH / 2, GRID_HEIGHT - 2));

    m_game->setInputSystem(m_input.get());
    m_game->initialize();

    m_renderer = std::make_unique<Renderer>(m_scene);

    /* =====================
     * Timer / GameLoop
     * ===================== */

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        const int deltaMs = 16; // ~60 FPS
        m_game->update(deltaMs);
        if (m_renderer)
            m_renderer->renderFrame(*m_game);

        if (m_game && m_game->state().isBaseDestroyed() && !m_gameOverItem) {
            m_gameOverItem = new QGraphicsTextItem(QStringLiteral("GAME OVER"));
            QFont font = m_gameOverItem->font();
            font.setPointSize(40);
            m_gameOverItem->setFont(font);
            m_gameOverItem->setDefaultTextColor(Qt::red);
            m_gameOverItem->setZValue(1000);
            m_scene->addItem(m_gameOverItem);

            QRectF sceneRect = m_scene->sceneRect();
            if (!sceneRect.isValid() || sceneRect.isNull())
                sceneRect = m_scene->itemsBoundingRect();

            const QRectF textRect = m_gameOverItem->boundingRect();
            const QPointF centeredPos = sceneRect.center() - QPointF(textRect.width() / 2.0, textRect.height() / 2.0);
            m_gameOverItem->setPos(centeredPos);
        }
    });
    m_timer->start(16);
}

MainWindow::~MainWindow()
{
    // Qt удалит QObject-детей автоматически
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    if (event->key() == Qt::Key_R && m_game && m_game->state().isBaseDestroyed()) {
        m_game->restart();

        if (m_gameOverItem) {
            m_scene->removeItem(m_gameOverItem);
            delete m_gameOverItem;
            m_gameOverItem = nullptr;
        }

        event->accept();
        return;
    }

    if (m_input && m_input->handleKeyPress(event->key())) {
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        QMainWindow::keyReleaseEvent(event);
        return;
    }

    if (m_input && m_input->handleKeyRelease(event->key())) {
        event->accept();
        return;
    }

    QMainWindow::keyReleaseEvent(event);
}
