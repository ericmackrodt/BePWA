#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Slot for creating the PWA
    void onCreatePwaClicked();

private:
    Ui::MainWindow *ui;
    QWebEngineView *webView;

    // Network manager to download the favicon
    QNetworkAccessManager *networkManager;

    // Label to show the favicon
    QLabel *iconLabel;

    // Function to create the PWA shortcut
    void createPwaShortcut(const QString &url, const QString &name, const QString &iconPath);
};

#endif // MAINWINDOW_H
