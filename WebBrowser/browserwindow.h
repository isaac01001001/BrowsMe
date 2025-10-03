#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QLineEdit>
#include <QToolBar>
#include <QProgressBar>
#include <QTabWidget>
#include <QAction>
#include <QMenu>
#include <QStatusBar>
#include <QVBoxLayout>

class BrowserWindow : public QMainWindow
{
    Q_OBJECT

public:
    BrowserWindow(QWidget *parent = nullptr);
    void navigateTo(const QString &url);

private slots:
    void onUrlChanged(const QUrl &url);
    void onLoadStarted();
    void onLoadFinished(bool success);
    void onLoadProgress(int progress);
    void onTitleChanged(const QString &title);
    void onIconChanged(const QIcon &icon);
    
    void navigateBack();
    void navigateForward();
    void reloadPage();
    void stopLoading();
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void showAbout();
    void newTab();
    void closeTab(int index);
    void currentTabChanged(int index);
    
    void updateNavigationActions();

private:
    void setupUI();
    void setupMenu();
    void setupToolBar();
    void setupConnections();
    QWidget* createBrowserTab();
    
    QTabWidget *m_tabWidget;
    QToolBar *m_toolBar;
    QStatusBar *m_statusBar;
    QProgressBar *m_progressBar;
    
    // Navigation actions
    QAction *m_backAction;
    QAction *m_forwardAction;
    QAction *m_reloadAction;
    QAction *m_stopAction;
    QAction *m_homeAction;
    
    // Zoom actions
    QAction *m_zoomInAction;
    QAction *m_zoomOutAction;
    QAction *m_resetZoomAction;
    
    // Tab actions
    QAction *m_newTabAction;
    QAction *m_closeTabAction;
    
    QLineEdit *m_urlBar;
    double m_currentZoom;
};

#endif // BROWSERWINDOW_H



