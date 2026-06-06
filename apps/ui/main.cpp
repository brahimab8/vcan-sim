#include <QApplication>
#include <QThread>
#include <QPalette>
#include <QStyle>

#include "main_window.h"
#include "can_worker_qt.h"
#include "monitor_core.h"
#include "can_bridge.h"
#include "ui_controller.h"
#include "driver.h"

static void applyDarkTheme(QApplication& app)
{
    app.setStyle("Fusion");

    QPalette p;
    p.setColor(QPalette::Window,          QColor(28,  28,  30));
    p.setColor(QPalette::WindowText,      QColor(235, 235, 245));
    p.setColor(QPalette::Base,            QColor(38,  38,  40));
    p.setColor(QPalette::AlternateBase,   QColor(44,  44,  46));
    p.setColor(QPalette::ToolTipBase,     QColor(28,  28,  30));
    p.setColor(QPalette::ToolTipText,     QColor(235, 235, 245));
    p.setColor(QPalette::Text,            QColor(235, 235, 245));
    p.setColor(QPalette::Button,          QColor(50,  50,  54));
    p.setColor(QPalette::ButtonText,      QColor(235, 235, 245));
    p.setColor(QPalette::BrightText,      Qt::red);
    p.setColor(QPalette::Highlight,       QColor(10,  132, 255));
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::Disabled, QPalette::Text,       QColor(100, 100, 100));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(100, 100, 100));
    app.setPalette(p);
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    applyDarkTheme(app);

    std::string interface = "vcan0";
    for (int i = 1; i < argc; ++i)
        interface = argv[i];

    SocketCanDriver driver(interface);
    MonitorCore     core(std::nullopt, std::optional<std::string>("data/csv"));
    CanBridge       bridge(driver, core);
    UiController    controller(driver);
    MainWindow      win(&controller);

    CanWorkerQt* worker = new CanWorkerQt(&bridge, &controller, 50);
    QThread*     thread = new QThread;
    worker->moveToThread(thread);

    QObject::connect(thread, &QThread::started,          worker, &CanWorkerQt::start);
    QObject::connect(&app,   &QApplication::aboutToQuit, worker, &CanWorkerQt::stop);
    QObject::connect(&app,   &QApplication::aboutToQuit, thread, &QThread::quit);

    QObject::connect(&win,   &MainWindow::sendRequested,  worker, &CanWorkerQt::onSendRequested, Qt::QueuedConnection);
    QObject::connect(worker, &CanWorkerQt::sendResult,    &win,   &MainWindow::handleSendResult,  Qt::QueuedConnection);

    qRegisterMetaType<DecodedMessage>("DecodedMessage");

    bridge.setOnMessage([worker](const DecodedMessage& msg){
        emit worker->decodedMessage(msg);
    });

    QObject::connect(worker, &CanWorkerQt::decodedMessage, &win, &MainWindow::handleDecodedMessage, Qt::QueuedConnection);

    thread->start();
    win.show();
    int rc = app.exec();

    thread->quit();
    thread->wait();
    delete worker;
    delete thread;
    return rc;
}