#include "systems/MenuSystem.h"

#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPen>
#include <QPointF>
#include <QSizeF>
#include <algorithm>

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
    case MenuState::PauseMenu:
        if (key == Qt::Key_Up || key == Qt::Key_Down) {
            if (!m_activeEntries.isEmpty()) {
                const int direction = (key == Qt::Key_Up) ? -1 : 1;
                m_selectedIndex = (m_selectedIndex + direction + m_activeEntries.size()) % m_activeEntries.size();
                updateSelectionVisuals();
            }
            event.accept();
            return true;
        }
        if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Space) {
            if (m_selectedIndex >= 0 && m_selectedIndex < m_activeEntries.size()) {
                const auto action = m_activeEntries[m_selectedIndex].action;
                if (action)
                    action();
            }
            event.accept();
            return true;
        }
        if (m_state == MenuState::MainMenu && key == Qt::Key_Escape) {
            if (m_exitCallback)
                m_exitCallback();
            event.accept();
            return true;
        }
        if (m_state == MenuState::PauseMenu && key == Qt::Key_Escape) {
            resumeGame();
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

    if (blocksGameplay() && m_state != MenuState::None) {
        event.accept();
        return true;
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

void MenuSystem::activateMenu(MenuState state, const QString& title, QVector<MenuEntry> entries)
{
    m_state = state;
    m_activeEntries = std::move(entries);
    m_activeTitle = title;
    m_selectedIndex = m_activeEntries.isEmpty() ? -1 : 0;
    updateMenuOverlays();
}

void MenuSystem::buildMainMenu()
{
    QVector<MenuEntry> entries;
    entries.append(MenuEntry{QStringLiteral("Start Game"), [this]() { startGame(); }});
    entries.append(MenuEntry{QStringLiteral("Exit"), [this]() {
        if (m_exitCallback)
            m_exitCallback();
    }});
    activateMenu(MenuState::MainMenu, QStringLiteral("BATTLE CITY"), std::move(entries));
}

void MenuSystem::buildPauseMenu()
{
    QVector<MenuEntry> entries;
    entries.append(MenuEntry{QStringLiteral("Resume"), [this]() { resumeGame(); }});
    entries.append(MenuEntry{QStringLiteral("Restart Level"), [this]() { restartLevel(); }});
    entries.append(MenuEntry{QStringLiteral("Exit to Main Menu"), [this]() { returnToMainMenu(); }});
    activateMenu(MenuState::PauseMenu, QStringLiteral("PAUSED"), std::move(entries));
}

void MenuSystem::startGame()
{
    clearInput();
    if (m_game)
        m_game->startNewGame();
    m_state = MenuState::None;
    clearGameOverOverlay();
    hideMenuItems();
    updateMenuBackground(sceneRect(), false);
}

void MenuSystem::pauseGame()
{
    clearInput();
    if (m_game)
        m_game->pause();
    buildPauseMenu();
}

void MenuSystem::resumeGame()
{
    clearInput();
    if (m_game)
        m_game->resume();
    m_state = MenuState::None;
    hideMenuItems();
    updateMenuBackground(sceneRect(), false);
}

void MenuSystem::restartLevel()
{
    clearInput();
    if (m_game)
        m_game->startNewGame();
    m_state = MenuState::None;
    clearGameOverOverlay();
    hideMenuItems();
    updateMenuBackground(sceneRect(), false);
}

void MenuSystem::returnToMainMenu()
{
    clearInput();
    if (m_game)
        m_game->enterMainMenu();
    clearGameOverOverlay();
    buildMainMenu();
}

void MenuSystem::enterGameOverMenu(bool victory)
{
    clearInput();
    m_victory = victory;
    m_state = MenuState::GameOverMenu;
    hideMenuItems();
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

void MenuSystem::updateMenuBackground(const QRectF& rect, bool visible)
{
    if (!m_scene)
        return;

    if (!m_overlayRect) {
        m_overlayRect = m_scene->addRect(rect, Qt::NoPen, QColor(0, 0, 0, 160));
        m_overlayRect->setZValue(1400);
    }

    if (visible) {
        m_overlayRect->setRect(rect);
        m_overlayRect->setVisible(true);
    } else {
        m_overlayRect->setVisible(false);
    }
}

void MenuSystem::updateMenuPanel(const QRectF& rect)
{
    if (!m_scene)
        return;

    if (!m_menuPanel) {
        m_menuPanel = m_scene->addRect(rect, QPen(QColor(80, 80, 80, 200)), QBrush(QColor(20, 20, 20, 210)));
        m_menuPanel->setZValue(1410);
    }

    const qreal panelWidth = std::min(rect.width() * 0.6, 420.0);
    const qreal padding = 24.0;
    const qreal spacing = 12.0;
    const QFont titleFont = [] {
        QFont font;
        font.setPointSize(34);
        font.setBold(true);
        return font;
    }();
    const QFont entryFont = [] {
        QFont font;
        font.setPointSize(22);
        return font;
    }();
    const QFontMetrics titleMetrics(titleFont);
    const QFontMetrics entryMetrics(entryFont);
    const qreal titleHeight = static_cast<qreal>(titleMetrics.height());
    const qreal entryHeight = static_cast<qreal>(entryMetrics.height());
    const qreal entriesHeight = m_activeEntries.isEmpty() ? 0.0
        : (m_activeEntries.size() * entryHeight + (m_activeEntries.size() - 1) * spacing);
    const qreal headerSpacing = m_activeEntries.isEmpty() ? 0.0 : spacing * 2.0;
    const qreal panelHeight = padding * 2.0 + titleHeight + headerSpacing + entriesHeight;

    const QPointF center = rect.center();
    const QRectF panelRect(center.x() - panelWidth / 2.0,
                           center.y() - panelHeight / 2.0,
                           panelWidth,
                           panelHeight);
    m_menuPanel->setRect(panelRect);
    m_menuPanel->setVisible(true);

    if (!m_menuTitleItem)
        m_menuTitleItem = m_scene->addText(QString());
    m_menuTitleItem->setDefaultTextColor(Qt::white);
    m_menuTitleItem->setFont(titleFont);
    m_menuTitleItem->setPlainText(m_activeTitle);
    m_menuTitleItem->setZValue(1420);
    const qreal titleWidth = static_cast<qreal>(titleMetrics.horizontalAdvance(m_activeTitle));
    const QPointF titlePos(panelRect.center().x() - titleWidth / 2.0,
                           panelRect.top() + padding);
    m_menuTitleItem->setPos(titlePos);
    m_menuTitleItem->setVisible(true);

    updateMenuEntries(panelRect);
}

void MenuSystem::updateMenuEntries(const QRectF& panelRect)
{
    if (!m_scene)
        return;

    while (m_menuEntryItems.size() < m_activeEntries.size()) {
        QGraphicsTextItem* item = m_scene->addText(QString());
        item->setZValue(1425);
        m_menuEntryItems.append(item);
    }
    while (m_menuEntryItems.size() > m_activeEntries.size()) {
        QGraphicsTextItem* item = m_menuEntryItems.takeLast();
        if (m_scene)
            m_scene->removeItem(item);
        delete item;
    }

    const qreal padding = 24.0;
    const qreal spacing = 12.0;
    const QFont entryFont = [] {
        QFont font;
        font.setPointSize(22);
        return font;
    }();
    const QFont selectedFont = [] {
        QFont font;
        font.setPointSize(22);
        font.setBold(true);
        return font;
    }();
    const QFont titleFont = [] {
        QFont font;
        font.setPointSize(34);
        font.setBold(true);
        return font;
    }();

    const qreal titleHeight = static_cast<qreal>(QFontMetrics(titleFont).height());
    qreal currentY = panelRect.top() + padding + titleHeight + (m_activeEntries.isEmpty() ? 0.0 : spacing * 2.0);

    for (int i = 0; i < m_activeEntries.size(); ++i) {
        const MenuEntry& entry = m_activeEntries[i];
        QGraphicsTextItem* item = m_menuEntryItems[i];
        const bool selected = (i == m_selectedIndex);
        const QFont& font = selected ? selectedFont : entryFont;
        item->setFont(font);
        item->setPlainText(entry.label);
        item->setDefaultTextColor(selected ? QColor(255, 236, 140) : QColor(225, 225, 225));
        const qreal textWidth = static_cast<qreal>(QFontMetrics(font).horizontalAdvance(entry.label));
        const QPointF pos(panelRect.center().x() - textWidth / 2.0, currentY);
        item->setPos(pos);
        item->setVisible(true);
        currentY += static_cast<qreal>(QFontMetrics(entryFont).height()) + spacing;
    }
}

void MenuSystem::hideMenuItems()
{
    if (m_menuPanel)
        m_menuPanel->setVisible(false);
    if (m_menuTitleItem)
        m_menuTitleItem->setVisible(false);
    for (QGraphicsTextItem* item : m_menuEntryItems) {
        if (item)
            item->setVisible(false);
    }
}

void MenuSystem::updateSelectionVisuals()
{
    updateMenuEntries(m_menuPanel ? m_menuPanel->rect() : QRectF());
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

    const QRectF rect = sceneRect();
    if (!rect.isValid() || rect.isNull())
        return;

    updateMenuBackground(rect, blocksGameplay());

    switch (m_state) {
    case MenuState::MainMenu:
        hideOverlay(m_pauseMenuItem);
        hideOverlay(m_gameOverItem);
        updateMenuPanel(rect);
        break;
    case MenuState::PauseMenu:
        hideOverlay(m_mainMenuItem);
        hideOverlay(m_gameOverItem);
        updateMenuPanel(rect);
        break;
    case MenuState::GameOverMenu: {
        const QString text = m_victory ? QStringLiteral("STAGE CLEAR") : QStringLiteral("GAME OVER");
        ensureOverlay(m_gameOverItem, text);
        if (m_gameOverItem) {
            QFont font = m_gameOverItem->font();
            font.setPointSize(40);
            font.setBold(true);
            m_gameOverItem->setFont(font);
            m_gameOverItem->setDefaultTextColor(m_victory ? QColor(240, 240, 240) : Qt::red);
            m_gameOverItem->setZValue(1600);
        }
        hideOverlay(m_mainMenuItem);
        hideOverlay(m_pauseMenuItem);
        hideMenuItems();
        centerOverlay(m_gameOverItem);
        break;
    }
    case MenuState::None:
        hideOverlay(m_mainMenuItem);
        hideOverlay(m_pauseMenuItem);
        hideOverlay(m_gameOverItem);
        updateMenuBackground(rect, false);
        hideMenuItems();
        break;
    }
}
