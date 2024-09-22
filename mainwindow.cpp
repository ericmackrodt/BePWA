#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>  // For file selection dialog
#include <QNetworkReply>
#include <QVBoxLayout>
#include <QPixmap>
#include <QNetworkRequest>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , webView(new QWebEngineView(this))  // Initialize the WebEngineView
{
    ui->setupUi(this);

    // Create a vertical layout
    QVBoxLayout *layout = new QVBoxLayout;

    // Add the WebEngineView to the layout
    layout->addWidget(webView);

    // Once the page is loaded, get the icon and display it
    connect(webView, &QWebEngineView::loadFinished, this, [this]() {
        QIcon icon = webView->icon();
        if (!icon.isNull()) {
            iconLabel->setPixmap(icon.pixmap(64, 64));
        }
    });

    // Load a webpage
    webView->load(QUrl("https://ericexperiment.com"));

    // Add the "Create PWA" button to the layout
    layout->addWidget(ui->createPwaButton);

    // Set the layout on the central widget
    ui->centralwidget->setLayout(layout);

    // Connect the button click to the handler
    connect(ui->createPwaButton, &QPushButton::clicked, this, &MainWindow::onCreatePwaClicked);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onCreatePwaClicked() {
    // Create the PWA dialog
    QDialog dialog(this);
    dialog.setWindowTitle("Create PWA Shortcut");

    QVBoxLayout layout(&dialog);

    // URL input
    QLineEdit *urlLineEdit = new QLineEdit(webView->url().toString(), &dialog);
    layout.addWidget(new QLabel("URL:"));
    layout.addWidget(urlLineEdit);

    // Name input
    QLineEdit *nameLineEdit = new QLineEdit(webView->title(), &dialog);
    layout.addWidget(new QLabel("App Name:"));
    layout.addWidget(nameLineEdit);

    // Icon Label to display the favicon or selected icon
    QLabel *iconLabel = new QLabel();
    layout.addWidget(new QLabel("App Icon:"));
    layout.addWidget(iconLabel);

    // Retrieve the favicon from the webView
    QIcon icon = webView->icon();
    if (!icon.isNull()) {
        QPixmap pixmap = icon.pixmap(64, 64);
        iconLabel->setPixmap(pixmap);
    } else {
        qDebug() << "No favicon available!";
    }

    // Button to choose a custom icon
    QPushButton *chooseIconButton = new QPushButton("Choose Icon");
    layout.addWidget(chooseIconButton);

    QString iconPath;
    connect(chooseIconButton, &QPushButton::clicked, [&]() {
        // Open a file dialog to select an icon
        QString selectedIconPath = QFileDialog::getOpenFileName(this, "Select Icon", "", "Images (*.png *.ico)");
        if (!selectedIconPath.isEmpty()) {
            iconPath = selectedIconPath;
            QPixmap newIcon(selectedIconPath);
            iconLabel->setPixmap(newIcon.scaled(64, 64, Qt::KeepAspectRatio));
        }
    });

    // Create PWA button
    QPushButton *createButton = new QPushButton("Create PWA");
    layout.addWidget(createButton);

    connect(createButton, &QPushButton::clicked, [&]() {
        // Use the selected icon or the default favicon
        createPwaShortcut(urlLineEdit->text(), nameLineEdit->text(), iconPath.isEmpty() ? "" : iconPath);
        dialog.accept();
    });

    dialog.exec();
}

void MainWindow::createPwaShortcut(const QString &url, const QString &name, const QString &iconPath) {
    // Define the path for the shortcuts folder inside the application directory
    QString appDir = QDir::currentPath();  // Assuming the app's directory is the current directory
    QString shortcutsDir = appDir + "/shortcuts";

    // Ensure the 'shortcuts' directory exists
    QDir dir;
    if (!dir.mkpath(shortcutsDir)) {
        qDebug() << "Failed to create directory: " << shortcutsDir;
        return;  // Handle failure to create the directory
    }

    // Create the PWA script in the 'shortcuts' directory
    QString scriptPath = shortcutsDir + "/" + name + ".sh";
    QFile scriptFile(scriptPath);
    if (scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&scriptFile);
        out << "#!/bin/sh\n";
        out << QString("%1/BePWA --url=%2\n").arg(appDir).arg(url);  // This will launch BePWA with the URL
        scriptFile.close();

        // Make the script executable
        QProcess::execute("chmod", QStringList() << "+x" << scriptPath);
    } else {
        qDebug() << "Failed to create PWA script: " << scriptPath;
        return;
    }

    // Define the path for the symbolic link in the Haiku menu
    QString linkDir = QDir::homePath() + "/config/settings/deskbar/menu/Applications/PWAs";
    if (!dir.mkpath(linkDir)) {
        qDebug() << "Failed to create directory: " << linkDir;
        return;  // Handle failure to create the directory
    }

    QString linkPath = linkDir + "/" + name;  // No .sh extension for the menu entry

    // Create the symbolic link in the Haiku menu directory
    QFile::remove(linkPath);  // Remove any existing link with the same name
    if (!QFile::link(scriptPath, linkPath)) {
        qDebug() << "Failed to create symbolic link: " << linkPath;
        return;
    }

    // Set the icon for the symbolic link if specified
    if (!iconPath.isEmpty()) {
        QProcess::execute("seticon", QStringList() << iconPath << linkPath);
    }

    qDebug() << "PWA shortcut created at: " << scriptPath;
    qDebug() << "Symbolic link created at: " << linkPath;
}
