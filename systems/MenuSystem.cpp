#include "systems/MenuSystem.h"

#include <QColor>
#include <QFont>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPointF>
#include <QSizeF>

#include "core/Game.h"
#include "systems/InputSystem.h"

MenuSystem::MenuSystem() = default;

void MenuSystem::setGame(Game* game)
{
    m_game = game;
}

void MenuSystem::setInputSystem(InputSystem* input)
{
    m_input = input;
}

void MenuSystem::setScene(QGraphicsScene* scene)
{
    m_scene = scene;
}

void MenuSystem::setExitCallback(std::function<void()> callback)
{
    m_exitCallback = std::move(callback);
}

bool MenuSystem::blocksGameplay() const
{
    return m_state == MenuState::MainMenu
        || m_state == MenuState::PauseMenu
        || m_state == MenuState::GameOverMenu;
}

bool MenuSystem::handleInput(QKeyEvent& event)
{
    const int key = event.key();
    switch (m_state) {
    case MenuState::MainMenu:
        if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Space) {
            startGame();
            event.accept();
            return true;
        }
        if (key == Qt::Key_Escape) {
            if (m_exitCallback)
                m_exitCallback();
            event.accept();
            return true;
        }
        break;
    case MenuState::PauseMenu:
        if (key == Qt::Key_Escape) {
            resumeGame();
            event.accept();
            return true;
        }
        if (key == Qt::Key_R) {
            restartLevel();
            event.accept();
            return true;
        }
        if (key == Qt::Key_M) {
            returnToMainMenu();
            event.accept();
            return true;
        }
        break;
    case MenuState::GameOverMenu:
        if (key == Qt::Key_R) {
            restartLevel();
            event.accept();
            return true;
        }
        if (key == Qt::Key_M) {
            returnToMainMenu();
            event.accept();
            return true;
        }
        break;
    case MenuState::None:
        if (key == Qt::Key_Escape) {
            pauseGame();
            event.accept();
            return true;
        }
        break;
    }

    return false;
}

void MenuSystem::syncWithGameState(const GameState& state)
{
    if (state.isVictory() || state.isGameOver()) {
        enterGameOverMenu(state.isVictory());
        return;
    }

    if (m_state == MenuState::GameOverMenu && !state.isGameOver() && !state.isVictory())
        m_state = MenuState::None;
}

void MenuSystem::renderMenus()
{
    updateMenuOverlays();
}

void MenuSystem::showMainMenu()
{
    returnToMainMenu();
}

void MenuSystem::startGame()
{
    clearInput();
    if (m_game)
        m_game->startNewGame();
    m_state = MenuState::None;
    clearGameOverOverlay();
}

void MenuSystem::pauseGame()
{
    clearInput();
    if (m_game)
        m_game->pause();
    m_state = MenuState::PauseMenu;
}

void MenuSystem::resumeGame()
{
    clearInput();
    if (m_game)
        m_game->resume();
    m_state = MenuState::None;
}

void MenuSystem::restartLevel()
{
    clearInput();
    if (m_game)
        m_game->startNewGame();
    m_state = MenuState::None;
    clearGameOverOverlay();
}

void MenuSystem::returnToMainMenu()
{
    clearInput();
    if (m_game)
        m_game->enterMainMenu();
    m_state = MenuState::MainMenu;
    clearGameOverOverlay();
}

void MenuSystem::enterGameOverMenu(bool victory)
{
    clearInput();
    m_victory = victory;
    m_state = MenuState::GameOverMenu;
}

void MenuSystem::clearInput()
{
    if (m_input)
        m_input->clear();
}

void MenuSystem::ensureOverlay(QGraphicsTextItem*& item, const QString& text)
{
    if (!m_scene)
        return;

    if (!item) {
        item = m_scene->addText(text);
        item->setDefaultTextColor(Qt::white);
        QFont font = item->font();
        font.setPointSize(28);
        item->setFont(font);
        item->setZValue(2000);
    }

    if (item->toPlainText() != text)
        item->setPlainText(text);

    item->setVisible(true);
}

void MenuSystem::hideOverlay(QGraphicsTextItem* item)
{
    if (item)
        item->setVisible(false);
}

void MenuSystem::centerOverlay(QGraphicsTextItem* item) const
{
    if (!item || !item->isVisible())
        return;

    const QRectF rect = sceneRect();
    if (!rect.isValid() || rect.isNull())
        return;

    const QRectF textRect = item->boundingRect();
    const QPointF centeredPos = rect.center() - QPointF(textRect.width() / 2.0, textRect.height() / 2.0);
    item->setPos(centeredPos);
}

QRectF MenuSystem::sceneRect() const
{
    if (!m_scene)
        return QRectF();

    QRectF rect = m_scene->sceneRect();
    if ((!rect.isValid() || rect.isNull()) && !m_scene->views().isEmpty() && m_scene->views().first()->viewport())
        rect = QRectF(QPointF(0.0, 0.0), QSizeF(m_scene->views().first()->viewport()->size()));

    if (!rect.isValid() || rect.isNull())
        rect = m_scene->itemsBoundingRect();

    return rect;
}

void MenuSystem::clearGameOverOverlay()
{
    if (m_scene && m_gameOverItem)
        m_scene->removeItem(m_gameOverItem);

    delete m_gameOverItem;
    m_gameOverItem = nullptr;
}

void MenuSystem::updateMenuOverlays()
{
    if (!m_scene)
        return;

    switch (m_state) {
    case MenuState::MainMenu:
        ensureOverlay(m_mainMenuItem, QStringLiteral("BATTLE CITY\n\nStart Game - Enter\nExit - Esc"));
        hideOverlay(m_pauseMenuItem);
        hideOverlay(m_gameOverItem);
        centerOverlay(m_mainMenuItem);
        break;
    case MenuState::PauseMenu:
        ensureOverlay(m_pauseMenuItem, QStringLiteral("PAUSED\n\nResume - Esc\nRestart Level - R\nMain Menu - M"));
        hideOverlay(m_mainMenuItem);
        hideOverlay(m_gameOverItem);
        centerOverlay(m_pauseMenuItem);
        break;
    case MenuState::GameOverMenu: {
        const QString text = m_victory ? QStringLiteral("STAGE CLEAR") : QStringLiteral("GAME OVER");
        ensureOverlay(m_gameOverItem, text);
        if (m_gameOverItem) {
            QFont font = m_gameOverItem->font();
            font.setPointSize(40);
            m_gameOverItem->setFont(font);
            m_gameOverItem->setDefaultTextColor(m_victory ? QColor(240, 240, 240) : Qt::red);
            m_gameOverItem->setZValue(1000);
        }
        hideOverlay(m_mainMenuItem);
        hideOverlay(m_pauseMenuItem);
        centerOverlay(m_gameOverItem);
        break;
    }
    case MenuState::None:
        hideOverlay(m_mainMenuItem);
        hideOverlay(m_pauseMenuItem);
        hideOverlay(m_gameOverItem);
        break;
    }
}
