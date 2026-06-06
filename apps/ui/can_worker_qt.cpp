#include "can_worker_qt.h"

CanWorkerQt::CanWorkerQt(CanBridge* bridge, UiController* controller, int poll_ms, QObject* parent)
    : QObject(parent), bridge_(bridge), controller_(controller), poll_ms_(poll_ms)
{
    timer_ = new QTimer(this);
    timer_->setInterval(poll_ms_);
    connect(timer_, &QTimer::timeout, this, &CanWorkerQt::onPoll);
}

void CanWorkerQt::start()  { timer_->start(); }
void CanWorkerQt::stop()   { timer_->stop();  }

void CanWorkerQt::onPoll()
{
    while (bridge_->runOnce()) {}
}

void CanWorkerQt::onSendRequested(float rpm)
{
    const auto result = controller_ ? controller_->sendTargetRpm(rpm) : std::nullopt;
    emit sendResult(result.has_value(), result.value_or(0.0f));
}