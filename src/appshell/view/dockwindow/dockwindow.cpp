/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "dockwindow.h"

#include "thirdparty/KDDockWidgets/src/DockWidgetQuick.h"
#include "thirdparty/KDDockWidgets/src/LayoutSaver.h"
#include "thirdparty/KDDockWidgets/src/private/quick/MainWindowQuick_p.h"
#include "thirdparty/KDDockWidgets/src/private/DockRegistry_p.h"
#include "thirdparty/KDDockWidgets/src/Config.h"

#include "dockcentralview.h"
#include "dockpageview.h"
#include "dockpanelview.h"
#include "dockstatusbarview.h"
#include "docktoolbarview.h"
#include "dockingholderview.h"
#include "dockwindow.h"

#include "async/async.h"
#include "log.h"

using namespace mu::dock;
using namespace mu::async;

namespace mu::dock {
static const QList<Location> POSSIBLE_LOCATIONS {
    Location::Left,
    Location::Right,
    Location::Top,
    Location::Bottom
};

static KDDockWidgets::Location locationToKLocation(Location location)
{
    switch (location) {
    case Location::Left: return KDDockWidgets::Location_OnLeft;
    case Location::Right: return KDDockWidgets::Location_OnRight;
    case Location::Top: return KDDockWidgets::Location_OnTop;
    case Location::Bottom: return KDDockWidgets::Location_OnBottom;
    case Location::Center: break;
    case Location::Undefined: break;
    }

    return KDDockWidgets::Location_None;
}

static void clearRegistry()
{
    TRACEFUNC;

    auto registry = KDDockWidgets::DockRegistry::self();

    registry->clear();

    for (KDDockWidgets::DockWidgetBase* dock : registry->dockwidgets()) {
        registry->unregisterDockWidget(dock);
    }
}
}

DockWindow::DockWindow(QQuickItem* parent)
    : QQuickItem(parent),
    m_toolBars(this),
    m_pages(this)
{
}

DockWindow::~DockWindow()
{
    dockWindowProvider()->deinit();
}

void DockWindow::componentComplete()
{
    TRACEFUNC;

    QQuickItem::componentComplete();

    m_mainWindow = new KDDockWidgets::MainWindowQuick("mainWindow",
                                                      KDDockWidgets::MainWindowOption_None,
                                                      this);

    connect(qApp, &QCoreApplication::aboutToQuit, this, &DockWindow::onQuit);
}

void DockWindow::geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    if (!m_currentPage) {
        QQuickItem::geometryChanged(newGeometry, oldGeometry);
        return;
    }

    //! NOTE: it is important to reset the current minimum width for all top-level toolbars
    //! Otherwise, the window content can be displaced after LayoutWidget::onResize(QSize newSize)
    //! due to lack of free space
    QList<DockToolBarView*> topToolBars = topLevelToolBars(m_currentPage);
    for (DockToolBarView* toolBar : topToolBars) {
        toolBar->setMinimumWidth(toolBar->contentWidth());
    }

    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    alignToolBars(m_currentPage);
}

void DockWindow::onQuit()
{
    TRACEFUNC;

    m_quiting = true;

    IF_ASSERT_FAILED(m_currentPage) {
        return;
    }

    savePageState(m_currentPage->objectName());

    clearRegistry();

    saveGeometry();
}

QString DockWindow::currentPageUri() const
{
    return m_currentPage ? m_currentPage->uri() : QString();
}

QQmlListProperty<mu::dock::DockToolBarView> DockWindow::toolBarsProperty()
{
    return m_toolBars.property();
}

QQmlListProperty<mu::dock::DockPageView> DockWindow::pagesProperty()
{
    return m_pages.property();
}

void DockWindow::init()
{
    clearRegistry();

    restoreGeometry();

    dockWindowProvider()->init(this);

    uiConfiguration()->windowGeometryChanged().onNotify(this, [this]() {
        if (!m_quiting) {
            resetWindowState();
        }
    });

    Async::call(this, [this]() {
        startupScenario()->run();
    });
}

void DockWindow::loadPage(const QString& uri, const QVariantMap& params)
{
    TRACEFUNC;

    if (currentPageUri() == uri) {
        if (m_currentPage) {
            m_currentPage->setParams(params);
        }
        return;
    }

    bool isFirstOpening = (m_currentPage == nullptr);

    if (!isFirstOpening) {
        savePageState(m_currentPage->objectName());
        clearRegistry();
    }

    bool ok = doLoadPage(uri, params);
    if (!ok) {
        return;
    }

    //! NOTE: show window as maximized if the user closed app in FullScreen mode
    if (isFirstOpening && (m_mainWindow->windowHandle()->windowStates() & QWindow::FullScreen)) {
        m_mainWindow->showMaximized();
    }

    emit currentPageUriChanged(uri);

    if (isFirstOpening) {
        emit windowLoaded();
    }

    emit pageLoaded();

    notifyAboutDocksOpenStatus();
}

bool DockWindow::isDockOpen(const QString& dockName) const
{
    return m_currentPage && m_currentPage->isDockOpen(dockName);
}

void DockWindow::toggleDock(const QString& dockName)
{
    if (m_currentPage) {
        m_currentPage->toggleDock(dockName);
        m_docksOpenStatusChanged.send({ dockName });
    }
}

void DockWindow::setDockOpen(const QString& dockName, bool open)
{
    if (m_currentPage) {
        m_currentPage->setDockOpen(dockName, open);
        m_docksOpenStatusChanged.send({ dockName });
    }
}

Channel<QStringList> DockWindow::docksOpenStatusChanged() const
{
    return m_docksOpenStatusChanged;
}

bool DockWindow::isDockFloating(const QString& dockName) const
{
    return m_currentPage && m_currentPage->isDockFloating(dockName);
}

void DockWindow::toggleDockFloating(const QString& dockName)
{
    if (m_currentPage) {
        m_currentPage->toggleDockFloating(dockName);
    }
}

DockPageView* DockWindow::currentPage() const
{
    return m_currentPage;
}

QQuickItem& DockWindow::asItem() const
{
    return *m_mainWindow;
}

void DockWindow::restoreDefaultLayout()
{
    TRACEFUNC;

    clearRegistry();

    if (m_currentPage) {
        for (DockBase* dock : m_currentPage->allDocks()) {
            dock->resetToDefault();
        }
    }

    for (const DockPageView* page : m_pages.list()) {
        uiConfiguration()->setPageState(page->objectName(), QByteArray());
    }

    uiConfiguration()->setWindowGeometry(QByteArray());
}

void DockWindow::loadPageContent(const DockPageView* page)
{
    TRACEFUNC;

    addDock(page->centralDock());

    loadPanels(page);
    loadToolBars(page);

    if (page->statusBar()) {
        addDock(page->statusBar(), Location::Bottom);
    }

    loadTopLevelToolBars(page);
}

void DockWindow::loadTopLevelToolBars(const DockPageView* page)
{
    TRACEFUNC;

    QList<DockToolBarView*> allToolBars = m_toolBars.list();
    allToolBars << page->mainToolBars();

    DockToolBarView* prevToolBar = nullptr;

    for (DockToolBarView* toolBar : allToolBars) {
        auto location = prevToolBar ? Location::Right : Location::Top;
        addDock(toolBar, location, prevToolBar);
        prevToolBar = toolBar;
    }
}

void DockWindow::loadToolBars(const DockPageView* page)
{
    TRACEFUNC;

    for (DockToolBarView* toolBar : page->toolBars()) {
        addDock(toolBar, toolBar->location());
    }

    for (Location location : POSSIBLE_LOCATIONS) {
        if (auto holder = page->holder(DockType::ToolBar, location)) {
            addDock(holder, location);
        }
    }
}

void DockWindow::loadPanels(const DockPageView* page)
{
    TRACEFUNC;

    auto canAddAsTab = [](const DockPanelView* panel, const DockPanelView* destination) {
        return panel->isVisible() && destination->isOpen() && destination->tabifyPanel() == panel;
    };

    auto addPanel = [this, page, canAddAsTab](DockPanelView* panel, Location location) {
        for (DockPanelView* destinationPanel : page->panels()) {
            if (canAddAsTab(panel, destinationPanel)) {
                registerDock(panel);

                destinationPanel->addPanelAsTab(panel);
                destinationPanel->setCurrentTabIndex(0);

                return;
            }
        }

        addDock(panel, location);
    };

    for (DockPanelView* panel : page->panels()) {
        addPanel(panel, panel->location());
    }

    for (Location location : POSSIBLE_LOCATIONS) {
        if (auto holder = page->holder(DockType::Panel, location)) {
            addDock(holder, location);
        }
    }
}

void DockWindow::alignToolBars(const DockPageView* page)
{
    QList<DockToolBarView*> topToolBars = topLevelToolBars(page);

    DockToolBarView* lastLeftToolBar = nullptr;
    DockToolBarView* lastCentralToolBar = nullptr;

    int leftToolBarsWidth = 0;
    int centralToolBarsWidth = 0;
    int rightToolBarsWidth = 0;

    int separatorThicnkess = KDDockWidgets::Config::self().separatorThickness();

    for (DockToolBarView* toolBar : topToolBars) {
        if (toolBar->floating() || !toolBar->isVisible()) {
            continue;
        }

        switch (static_cast<DockToolBarAlignment::Type>(toolBar->alignment())) {
        case DockToolBarAlignment::Left:
            lastLeftToolBar = toolBar;
            leftToolBarsWidth += toolBar->contentWidth();
            break;
        case DockToolBarAlignment::Center:
            lastCentralToolBar = toolBar;
            centralToolBarsWidth += (toolBar->contentWidth() + separatorThicnkess);
            break;
        case DockToolBarAlignment::Right:
            rightToolBarsWidth += (toolBar->contentWidth() + separatorThicnkess);
            break;
        }
    }

    if (!lastLeftToolBar || !lastCentralToolBar) {
        return;
    }

    int deltaForLastLeftToolbar = (width() - centralToolBarsWidth) / 2 - leftToolBarsWidth;
    int deltaForLastCentralToolBar = (width() - centralToolBarsWidth) / 2 - rightToolBarsWidth;

    deltaForLastLeftToolbar = std::max(deltaForLastLeftToolbar, 0);
    deltaForLastCentralToolBar = std::max(deltaForLastCentralToolBar, 0);

    int freeSpace = width() - (leftToolBarsWidth + centralToolBarsWidth + rightToolBarsWidth);

    if (deltaForLastLeftToolbar + deltaForLastCentralToolBar > freeSpace) {
        deltaForLastLeftToolbar = freeSpace;
        deltaForLastCentralToolBar = 0;
    }

    lastLeftToolBar->setMinimumWidth(lastLeftToolBar->contentWidth() + deltaForLastLeftToolbar);
    lastCentralToolBar->setMinimumWidth(lastCentralToolBar->contentWidth() + deltaForLastCentralToolBar);
}

void DockWindow::addDock(DockBase* dock, Location location, const DockBase* relativeTo)
{
    TRACEFUNC;

    registerDock(dock);

    KDDockWidgets::DockWidgetBase* relativeDock = relativeTo ? relativeTo->dockWidget() : nullptr;

    auto visibilityOption = dock->isVisible() ? KDDockWidgets::InitialVisibilityOption::StartVisible
                            : KDDockWidgets::InitialVisibilityOption::StartHidden;

    KDDockWidgets::InitialOption options(visibilityOption, dock->preferredSize());

    m_mainWindow->addDockWidget(dock->dockWidget(), locationToKLocation(location), relativeDock, options);
}

void DockWindow::registerDock(DockBase* dock)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(dock) {
        return;
    }

    auto registry = KDDockWidgets::DockRegistry::self();
    auto dockWidget = dock->dockWidget();

    if (!registry->containsDockWidget(dockWidget->uniqueName())) {
        registry->registerDockWidget(dockWidget);
    }
}

DockPageView* DockWindow::pageByUri(const QString& uri) const
{
    for (DockPageView* page : m_pages.list()) {
        if (page->uri() == uri) {
            return page;
        }
    }

    return nullptr;
}

bool DockWindow::doLoadPage(const QString& uri, const QVariantMap& params)
{
    DockPageView* newPage = pageByUri(uri);
    IF_ASSERT_FAILED(newPage) {
        return false;
    }

    loadPageContent(newPage);
    restorePageState(newPage->objectName());
    initDocks(newPage);

    newPage->setParams(params);

    m_currentPage = newPage;

    return true;
}

void DockWindow::saveGeometry()
{
    TRACEFUNC;

    /// NOTE: The state of all dock widgets is also saved here,
    /// since the library does not provide the ability to save
    /// and restore only the application geometry.
    /// Therefore, for correct operation after saving or restoring geometry,
    /// it is necessary to apply the appropriate method for the state.
    uiConfiguration()->setWindowGeometry(windowState());
}

void DockWindow::restoreGeometry()
{
    TRACEFUNC;

    if (!restoreLayout(uiConfiguration()->windowGeometry())) {
        LOGE() << "Could not restore the window geometry!";
    }
}

void DockWindow::savePageState(const QString& pageName)
{
    TRACEFUNC;

    uiConfiguration()->setPageState(pageName, windowState());
}

void DockWindow::restorePageState(const QString& pageName)
{
    TRACEFUNC;

    /// NOTE: Do not restore geometry
    bool ok = restoreLayout(uiConfiguration()->pageState(pageName), true /*restoreRelativeToMainWindow*/);
    if (!ok) {
        LOGE() << "Could not restore the state of " << pageName << "!";
    }
}

bool DockWindow::restoreLayout(const QByteArray& layout, bool restoreRelativeToMainWindow)
{
    if (layout.isEmpty()) {
        return true;
    }

    TRACEFUNC;

    auto option = restoreRelativeToMainWindow ? KDDockWidgets::RestoreOption_RelativeToMainWindow
                  : KDDockWidgets::RestoreOption_None;

    KDDockWidgets::LayoutSaver layoutSaver(option);
    return layoutSaver.restoreLayout(layout);
}

QByteArray DockWindow::windowState() const
{
    TRACEFUNC;

    KDDockWidgets::LayoutSaver layoutSaver;
    return layoutSaver.serializeLayout();
}

void DockWindow::resetWindowState()
{
    QString currentPageUriBackup = currentPageUri();

    /// NOTE: for reset geometry
    m_currentPage = nullptr;

    bool ok = doLoadPage(currentPageUriBackup, {});
    if (!ok) {
        return;
    }

    notifyAboutDocksOpenStatus();
}

void DockWindow::initDocks(DockPageView* page)
{
    TRACEFUNC;

    for (DockToolBarView* toolbar : m_toolBars.list()) {
        toolbar->init();
    }

    if (page) {
        page->init();
    }

    alignToolBars(page);

    for (DockToolBarView* toolbar : page->mainToolBars()) {
        connect(toolbar, &DockToolBarView::floatingChanged, this, [this, page]() {
            alignToolBars(page);
        }, Qt::UniqueConnection);
    }
}

void DockWindow::notifyAboutDocksOpenStatus()
{
    const DockPageView* page = currentPage();

    IF_ASSERT_FAILED(page) {
        return;
    }

    QStringList dockNames;

    for (DockToolBarView* toolBar : page->toolBars()) {
        dockNames << toolBar->objectName();
    }

    for (DockPanelView* panel : page->panels()) {
        dockNames << panel->objectName();
    }

    m_docksOpenStatusChanged.send(dockNames);
}

QList<DockToolBarView*> DockWindow::topLevelToolBars(const DockPageView* page) const
{
    QList<DockToolBarView*> toolBars = m_toolBars.list();

    if (page) {
        toolBars << page->mainToolBars();
    }

    return toolBars;
}
