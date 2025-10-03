#include "browserwindow.h"
#include <QWebEngineSettings>
#include <QMessageBox>
#include <QShortcut>
#include <QToolButton>
#include <QStyle>

BrowserWindow::BrowserWindow(QWidget *parent)
    : QMainWindow(parent), m_currentZoom(1.0)
{
    setupUI();
    setupMenu();
    setupToolBar();
    setupConnections();
    
    // Set window properties
    setWindowTitle("Web Browser");
    setMinimumSize(800, 600);
    resize(1200, 800);
    
    // Create initial tab
    newTab();
}

void BrowserWindow::setupUI()
{
    // Create central widget and layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // Create tab widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    m_tabWidget->setDocumentMode(true);
    
    layout->addWidget(m_tabWidget);
    setCentralWidget(centralWidget);
    
    // Create status bar
    m_statusBar = new QStatusBar(this);
    m_progressBar = new QProgressBar();
    m_progressBar->setMaximumSize(120, 16);
    m_progressBar->setTextVisible(false);
    m_progressBar->setVisible(false);
    
    m_statusBar->addPermanentWidget(m_progressBar);
    setStatusBar(m_statusBar);
}

void BrowserWindow::setupMenu()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");
    m_newTabAction = fileMenu->addAction("&New Tab", this, &BrowserWindow::newTab, QKeySequence::AddTab);
    fileMenu->addSeparator();
    fileMenu->addAction("&Quit", this, &QWidget::close, QKeySequence::Quit);
    
    // Edit menu
    QMenu *editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction("&Copy", this, []() { 
        if (QApplication::focusWidget()) 
            QApplication::focusWidget()->copy(); 
    }, QKeySequence::Copy);
    editMenu->addAction("&Paste", this, []() { 
        if (QApplication::focusWidget()) 
            QApplication::focusWidget()->paste(); 
    }, QKeySequence::Paste);
    
    // View menu
    QMenu *viewMenu = menuBar()->addMenu("&View");
    m_zoomInAction = viewMenu->addAction("Zoom &In", this, &BrowserWindow::zoomIn, QKeySequence::ZoomIn);
    m_zoomOutAction = viewMenu->addAction("Zoom &Out", this, &BrowserWindow::zoomOut, QKeySequence::ZoomOut);
    m_resetZoomAction = viewMenu->addAction("&Reset Zoom", this, &BrowserWindow::resetZoom, QKeySequence("Ctrl+0"));
    
    // Help menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About", this, &BrowserWindow::showAbout);
}

void BrowserWindow::setupToolBar()
{
    m_toolBar = addToolBar("Navigation");
    m_toolBar->setMovable(false);
    m_toolBar->setIconSize(QSize(16, 16));
    
    // Navigation buttons
    m_backAction = m_toolBar->addAction(style()->standardIcon(QStyle::SP_ArrowBack), "Back", this, &BrowserWindow::navigateBack);
    m_forwardAction = m_toolBar->addAction(style()->standardIcon(QStyle::SP_ArrowForward), "Forward", this, &BrowserWindow::navigateForward);
    m_reloadAction = m_toolBar->addAction(style()->standardIcon(QStyle::SP_BrowserReload), "Reload", this, &BrowserWindow::reloadPage);
    m_stopAction = m_toolBar->addAction(style()->standardIcon(QStyle::SP_BrowserStop), "Stop", this, &BrowserWindow::stopLoading);
    
    m_toolBar->addSeparator();
    
    // URL bar
    m_urlBar = new QLineEdit();
    m_urlBar->setClearButtonEnabled(true);
    m_urlBar->setPlaceholderText("Enter URL or search term");
    connect(m_urlBar, &QLineEdit::returnPressed, this, [this]() {
        QString text = m_urlBar->text();
        QUrl url;
        
        if (text.contains(".") && !text.contains(" ")) {
            if (!text.startsWith("http://") && !text.startsWith("https://")) {
                url = QUrl("https://" + text);
            } else {
                url = QUrl(text);
            }
        } else {
            url = QUrl("https://www.google.com/search?q=" + text);
        }
        
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
            view->setUrl(url);
        }
    });
    
    m_toolBar->addWidget(m_urlBar);
    
    // Home button
    m_homeAction = m_toolBar->addAction(style()->standardIcon(QStyle::SP_DirHomeIcon), "Home", this, [this]() {
        navigateTo("https://www.google.com");
    });
    
    m_toolBar->addSeparator();
    
    // New tab button
    QToolButton *newTabButton = new QToolButton();
    newTabButton->setDefaultAction(m_newTabAction);
    newTabButton->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    m_toolBar->addWidget(newTabButton);
}

void BrowserWindow::setupConnections()
{
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &BrowserWindow::closeTab);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &BrowserWindow::currentTabChanged);
}

QWidget* BrowserWindow::createBrowserTab()
{
    QWebEngineView *webView = new QWebEngineView();
    QWebEnginePage *page = new QWebEnginePage(QWebEngineProfile::defaultProfile(), webView);
    webView->setPage(page);
    
    // Enable modern web features
    webView->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    webView->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    webView->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    webView->settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);
    webView->settings()->setAttribute(QWebEngineSettings::AutoLoadImages, true);
    webView->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, true);
    
    // Connect signals
    connect(webView, &QWebEngineView::urlChanged, this, &BrowserWindow::onUrlChanged);
    connect(webView, &QWebEngineView::loadStarted, this, &BrowserWindow::onLoadStarted);
    connect(webView, &QWebEngineView::loadFinished, this, &BrowserWindow::onLoadFinished);
    connect(webView, &QWebEngineView::titleChanged, this, &BrowserWindow::onTitleChanged);
    connect(webView, &QWebEngineView::iconChanged, this, &BrowserWindow::onIconChanged);
    connect(webView->page(), &QWebEnginePage::loadProgress, this, &BrowserWindow::onLoadProgress);
    
    return webView;
}

void BrowserWindow::newTab()
{
    QWidget *tab = createBrowserTab();
    int index = m_tabWidget->addTab(tab, "New Tab");
    m_tabWidget->setCurrentIndex(index);
    
    // Navigate to home page
    QWebEngineView *webView = qobject_cast<QWebEngineView*>(tab);
    if (webView) {
        webView->setUrl(QUrl("https://www.google.com"));
    }
}

void BrowserWindow::closeTab(int index)
{
    if (m_tabWidget->count() > 1) {
        QWidget *widget = m_tabWidget->widget(index);
        m_tabWidget->removeTab(index);
        widget->deleteLater();
    } else {
        close();
    }
}

void BrowserWindow::currentTabChanged(int index)
{
    if (index >= 0) {
        updateNavigationActions();
        if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->widget(index))) {
            onUrlChanged(view->url());
            onTitleChanged(view->title());
            onLoadFinished(true);
        }
    }
}

void BrowserWindow::navigateTo(const QString &url)
{
    if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
        view->setUrl(QUrl(url));
    }
}

void BrowserWindow::navigateBack()
{
    if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
        view->back();
    }
}

void BrowserWindow::navigateForward()
{
    if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
        view->forward();
    }
}

void BrowserWindow::reloadPage()
{
    if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
        view->reload();
    }
}

void BrowserWindow::stopLoading()
{
    if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
        view->stop();
    }
}

void BrowserWindow::zoomIn()
{
    m_currentZoom += 0.1;
    if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
        view->setZoomFactor(m_currentZoom);
    }
}

void BrowserWindow::zoomOut()
{
    m_currentZoom -= 0.1;
    if (m_currentZoom < 0.25) m_currentZoom = 0.25;
    if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
        view->setZoomFactor(m_currentZoom);
    }
}

void BrowserWindow::resetZoom()
{
    m_currentZoom = 1.0;
    if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
        view->setZoomFactor(m_currentZoom);
    }
}

void BrowserWindow::updateNavigationActions()
{
    if (QWebEngineView *view = qobject_cast<QWebEngineView*>(m_tabWidget->currentWidget())) {
        m_backAction->setEnabled(view->history()->canGoBack());
        m_forwardAction->setEnabled(view->history()->canGoForward());
    }
}

void BrowserWindow::onUrlChanged(const QUrl &url)
{
    if (sender() == m_tabWidget->currentWidget()) {
        m_urlBar->setText(url.toString());
        m_urlBar->setCursorPosition(0);
    }
}

void BrowserWindow::onLoadStarted()
{
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    m_stopAction->setEnabled(true);
    m_reloadAction->setEnabled(false);
}

void BrowserWindow::onLoadFinished(bool success)
{
    m_progressBar->setVisible(false);
    m_progressBar->setValue(0);
    m_stopAction->setEnabled(false);
    m_reloadAction->setEnabled(true);
    
    if (!success) {
        m_statusBar->showMessage("Failed to load page", 3000);
    } else {
        m_statusBar->showMessage("Page loaded successfully", 3000);
    }
    
    updateNavigationActions();
}

void BrowserWindow::onLoadProgress(int progress)
{
    m_progressBar->setValue(progress);
}

void BrowserWindow::onTitleChanged(const QString &title)
{
    if (sender() == m_tabWidget->currentWidget()) {
        setWindowTitle(title + " - WebBrowser");
        
        // Update tab title
        QWebEngineView *view = qobject_cast<QWebEngineView*>(sender());
        if (view) {
            int index = m_tabWidget->indexOf(view);
            if (index >= 0) {
                QString shortTitle = title;
                if (shortTitle.length() > 20) {
                    shortTitle = shortTitle.left(20) + "...";
                }
                m_tabWidget->setTabText(index, shortTitle);
                m_tabWidget->setTabToolTip(index, title);
            }
        }
    }
}

void BrowserWindow::onIconChanged(const QIcon &icon)
{
    QWebEngineView *view = qobject_cast<QWebEngineView*>(sender());
    if (view) {
        int index = m_tabWidget->indexOf(view);
        if (index >= 0) {
            m_tabWidget->setTabIcon(index, icon);
        }
    }
}

void BrowserWindow::showAbout()
{
    QMessageBox::about(this, "About WebBrowser",
        "<h3>Web Browser</h3>"
        "<p>A modern web browser built with Qt and QWebEngine.</p>"
        "<p>Version 1.0</p>"
        "<p>Supports modern web standards including HTML5, CSS3, and JavaScript.</p>");
}


