#include "mainwindow.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QPoint>
#include <QKeyEvent>
#include <QSize>
#include <QResizeEvent>
#include <QtGlobal>
#include <QEvent>
#include <QMouseEvent>

#include "core/Game.h"
#include "systems/InputSystem.h"
#include "systems/MenuSystem.h"
#include "rendering/Renderer.h"
#include "utils/Constants.h"
#include "world/Tile.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    /* =====================
     * Scene / View
     * ===================== */

    m_scene = new QGraphicsScene(this);

    m_view = new QGraphicsView(m_scene, this);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setMouseTracking(true);
    if (m_view->viewport())
        m_view->viewport()->setMouseTracking(true);
    if (m_view->viewport())
        m_view->viewport()->installEventFilter(this);
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

    m_menuSystem = std::make_unique<MenuSystem>();
    m_menuSystem->setGame(m_game.get());
    m_menuSystem->setInputSystem(m_input.get());
    m_menuSystem->setScene(m_scene);
    m_menuSystem->setExitCallback([this]() { close(); });
    m_menuSystem->showMainMenu();

    m_renderer = std::make_unique<Renderer>(m_scene);

    /* =====================
     * Timer / GameLoop
     * ===================== */

    m_frameTimer.start();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        const qint64 frameDeltaMs = m_frameTimer.restart();
        m_frameAccumulatorMs += frameDeltaMs;

        while (m_frameAccumulatorMs >= kFixedTickMs) {
            if (m_game && (!m_menuSystem || !m_menuSystem->blocksGameplay()))
                m_game->update(kFixedTickMs);
            m_frameAccumulatorMs -= kFixedTickMs;
        }

        if (m_menuSystem && m_game)
            m_menuSystem->syncWithGameState(m_game->state());

        const qreal alpha = static_cast<qreal>(m_frameAccumulatorMs) / static_cast<qreal>(kFixedTickMs);

        if (m_renderer)
            m_renderer->renderFrame(*m_game, alpha);

        if (m_menuSystem)
            m_menuSystem->renderMenus();
    });
    m_timer->start(kFixedTickMs);
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

    if (m_menuSystem && m_menuSystem->handleInput(*event))
        return;

    if (m_menuSystem && m_menuSystem->blocksGameplay()) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    if (m_game) {
        if (event->key() == Qt::Key_F1) {
            m_game->enterEditMode();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_F2) {
            m_game->exitEditMode();
            event->accept();
            return;
        }

        if (m_game->isEditMode()) {
            bool handled = true;
            switch (event->key()) {
            case Qt::Key_1: m_game->selectEditorTile(TileType::Empty); break;
            case Qt::Key_2: m_game->selectEditorTile(TileType::Brick); break;
            case Qt::Key_3: m_game->selectEditorTile(TileType::Steel); break;
            case Qt::Key_4: m_game->selectEditorTile(TileType::Forest); break;
            case Qt::Key_5: m_game->selectEditorTile(TileType::Water); break;
            case Qt::Key_6: m_game->selectEditorTile(TileType::Ice); break;
            case Qt::Key_7: m_game->selectEditorTile(TileType::Base); break;
            default:
                handled = false;
                break;
            }

            if (handled) {
                event->accept();
                return;
            }

            event->accept();
            return;
        }
    }

    if (m_input && m_input->handleKeyPress(event->key(), event->nativeScanCode())) {
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

    if (m_menuSystem && m_menuSystem->blocksGameplay()) {
        QMainWindow::keyReleaseEvent(event);
        return;
    }

    if (m_game && m_game->isEditMode()) {
        event->accept();
        return;
    }

    if (m_input && m_input->handleKeyRelease(event->key(), event->nativeScanCode())) {
        event->accept();
        return;
    }

    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    if (m_menuSystem)
        m_menuSystem->renderMenus();
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (!m_game || !m_renderer || !m_view)
        return QMainWindow::eventFilter(watched, event);

    if (watched != m_view && watched != m_view->viewport())
        return QMainWindow::eventFilter(watched, event);

    const QEvent::Type type = event->type();
    const bool isMouseMove = type == QEvent::MouseMove;
    const bool isMousePress = type == QEvent::MouseButtonPress;
    const bool isLeave = type == QEvent::Leave;

    if (!isMouseMove && !isMousePress && !isLeave)
        return QMainWindow::eventFilter(watched, event);

    if (!m_game->isEditMode()) {
        m_game->clearHoveredCell();
        return QMainWindow::eventFilter(watched, event);
    }

    if (isLeave) {
        m_game->clearHoveredCell();
        return false;
    }

    auto* mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if (!mouseEvent)
        return QMainWindow::eventFilter(watched, event);

    const QPointF scenePos = m_view->mapToScene(mouseEvent->pos());
    QPoint cell;
    if (!m_renderer->scenePosToCell(scenePos, *m_game, cell)) {
        m_game->clearHoveredCell();
        return false;
    }

    m_game->setHoveredCell(cell);

    if (isMousePress)
        m_game->applyEditorClick(cell, mouseEvent->button());

    return false;
}
