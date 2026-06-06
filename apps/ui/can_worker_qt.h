#pragma once

#include <QObject>
#include <QTimer>

#include "can_bridge.h"
#include "ui_controller.h"

class CanWorkerQt : public QObject {
    Q_OBJECT
public:
    CanWorkerQt(CanBridge* bridge, UiController* controller, int poll_ms = 50, QObject* parent = nullptr);

public slots:
    void start();
    void stop();
    void onSendRequested(float rpm);

signals:
    void decodedMessage(const DecodedMessage& msg);
    void sendResult(bool ok, float value);      // value is the sent RPM on success, 0 on failure

private slots:
    void onPoll();

private:
    CanBridge*    bridge_;
    UiController* controller_;
    QTimer*       timer_;
    int           poll_ms_;
};