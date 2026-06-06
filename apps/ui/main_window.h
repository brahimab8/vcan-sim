#pragma once

#include <QMainWindow>
#include <QGroupBox>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QPushButton>
#include <QMap>
#include <QString>
#include <QVector>

#include "decoder.h"
#include "ui_controller.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(UiController* controller, QWidget* parent = nullptr);

public slots:
    void handleDecodedMessage(const DecodedMessage& msg);
    void handleSendResult(bool ok, float value);

signals:
    void sendRequested(float rpm);

private slots:
    void onSendClicked();
    void onSliderChanged(int value);

private:
    UiController* controller_;

    QMap<QString, QLabel*>      value_labels_;
    QMap<QString, QGridLayout*> signal_to_layout_;

    struct ControlWidgets {
        QString      card_key;  // "MessageName/SignalName": card to update on send
        QSlider*     slider;
        QSpinBox*    readout;
        QPushButton* send_btn;
        QLabel*      status;
    };
    QVector<ControlWidgets> controls_;
};