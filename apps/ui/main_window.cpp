#include "main_window.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QFont>

// Applied once to each signal card frame
static const char* kCardQss =
    "QFrame {"
    "  background-color: #1c1c1e;"
    "  border: 1px solid #3a3a3c;"
    "  border-radius: 8px;"
    "}"
    "QFrame:hover {"
    "  border: 1px solid #636366;"
    "}";

static const char* kGroupQss =
    "QGroupBox {"
    "  border: 1px solid #3a3a3c;"
    "  border-radius: 8px;"
    "  margin-top: 10px;"
    "  font-weight: bold;"
    "  color: #ebebf5;"
    "}"
    "QGroupBox::title {"
    "  subcontrol-origin: margin;"
    "  left: 10px;"
    "  padding: 0 4px;"
    "}";

static const char* kSendBtnQss =
    "QPushButton {"
    "  background-color: #0a84ff;"
    "  color: white;"
    "  border: none;"
    "  border-radius: 6px;"
    "  padding: 4px 16px;"
    "  font-weight: bold;"
    "}"
    "QPushButton:hover   { background-color: #339cff; }"
    "QPushButton:pressed { background-color: #0060cc; }";

static const char* kSliderQss =
    "QSlider::groove:horizontal {"
    "  height: 4px;"
    "  background: #3a3a3c;"
    "  border-radius: 2px;"
    "}"
    "QSlider::sub-page:horizontal {"
    "  background: #0a84ff;"
    "  border-radius: 2px;"
    "}"
    "QSlider::handle:horizontal {"
    "  width: 14px; height: 14px;"
    "  margin: -5px 0;"
    "  background: white;"
    "  border-radius: 7px;"
    "}";

MainWindow::MainWindow(UiController* controller, QWidget* parent)
    : QMainWindow(parent), controller_(controller)
{
    QWidget*     central = new QWidget(this);
    QVBoxLayout* root    = new QVBoxLayout(central);
    root->setSpacing(12);
    root->setContentsMargins(12, 12, 12, 12);
    setCentralWidget(central);

    for (const auto& ecu : controller_->ecuDescriptors()) {
        auto* group   = new QGroupBox(QString::fromStdString(ecu.name), central);
        group->setStyleSheet(kGroupQss);
        auto* vbox    = new QVBoxLayout(group);
        vbox->setSpacing(10);
        auto* cards_w = new QWidget(group);
        auto* grid    = new QGridLayout(cards_w);
        grid->setSpacing(8);
        grid->setContentsMargins(0, 0, 0, 0);
        grid->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        vbox->addWidget(cards_w);

        int col = 0, row = 0;
        const int max_cols = 4;

        for (const auto& sig : ecu.signal_list) {
            const QString msg_name = QString::fromStdString(sig.message_name);
            const QString sig_name = QString::fromStdString(sig.signal_name);
            const QString key      = msg_name + "/" + sig_name;

            auto* card = new QFrame(cards_w);
            card->setStyleSheet(kCardQss);
            card->setMinimumWidth(160);
            card->setMinimumHeight(90);
            auto* cl = new QVBoxLayout(card);
            cl->setContentsMargins(12, 10, 12, 10);
            cl->setSpacing(4);

            auto* sig_lbl = new QLabel(sig_name, card);
            QFont sf = sig_lbl->font();
            sf.setPointSize(sf.pointSize() - 1);
            sig_lbl->setFont(sf);
            sig_lbl->setStyleSheet("color: #8e8e93; border: none; background: transparent;");

            auto* val_lbl = new QLabel("—", card);
            QFont vf = val_lbl->font();
            vf.setPointSize(vf.pointSize() + 5);
            vf.setBold(true);
            val_lbl->setFont(vf);
            val_lbl->setAlignment(Qt::AlignCenter);
            val_lbl->setStyleSheet("color: #ebebf5; border: none; background: transparent;");

            auto* msg_lbl = new QLabel(msg_name, card);
            QFont mf = msg_lbl->font();
            mf.setPointSize(mf.pointSize() - 2);
            msg_lbl->setFont(mf);
            msg_lbl->setStyleSheet("color: #48484a; border: none; background: transparent;");
            msg_lbl->setAlignment(Qt::AlignRight);

            cl->addWidget(sig_lbl);
            cl->addWidget(val_lbl);
            cl->addWidget(msg_lbl);

            grid->addWidget(card, row, col);
            if (++col >= max_cols) { col = 0; ++row; }

            value_labels_.insert(key, val_lbl);
            signal_to_layout_.insert(key, grid);
        }

        for (const auto& ctrl : controller_->controlDescriptors()) {
            bool owned = false;
            for (const auto& sig : ecu.signal_list) {
                if (sig.signal_name == ctrl.signal_name) { owned = true; break; }
            }
            if (!owned) continue;

            const QString card_key = QString::fromStdString(ctrl.message_name)
                                   + "/" + QString::fromStdString(ctrl.signal_name);

            auto* divider = new QFrame(group);
            divider->setFrameShape(QFrame::HLine);
            divider->setStyleSheet("background-color: #3a3a3c; border: none; max-height: 1px;");
            vbox->addWidget(divider);

            auto* slider = new QSlider(Qt::Horizontal, group);
            slider->setStyleSheet(kSliderQss);
            slider->setRange(static_cast<int>(ctrl.min), static_cast<int>(ctrl.max));
            slider->setValue(static_cast<int>(ctrl.default_value));
            slider->setTickInterval(static_cast<int>((ctrl.max - ctrl.min) / 16));
            slider->setTickPosition(QSlider::TicksBelow);

            auto* readout = new QSpinBox(group);
            readout->setRange(static_cast<int>(ctrl.min), static_cast<int>(ctrl.max));
            readout->setValue(static_cast<int>(ctrl.default_value));
            readout->setFixedWidth(85);
            readout->setSuffix(" rpm");

            auto* send_btn = new QPushButton("Send", group);
            send_btn->setStyleSheet(kSendBtnQss);
            send_btn->setFixedHeight(28);

            auto* status = new QLabel("", group);
            status->setStyleSheet("color: #8e8e93;");
            status->setMinimumWidth(60);

            connect(slider,   &QSlider::valueChanged,                        readout, &QSpinBox::setValue);
            connect(readout,  QOverload<int>::of(&QSpinBox::valueChanged),   slider,  &QSlider::setValue);
            connect(slider,   &QSlider::valueChanged,                        this,    &MainWindow::onSliderChanged);
            connect(send_btn, &QPushButton::clicked,                         this,    &MainWindow::onSendClicked);

            QHBoxLayout* ctrl_row = new QHBoxLayout;
            ctrl_row->setSpacing(8);
            ctrl_row->addWidget(new QLabel(QString::fromStdString(ctrl.label) + ":"));
            ctrl_row->addWidget(slider, 1);
            ctrl_row->addWidget(readout);
            ctrl_row->addWidget(send_btn);
            ctrl_row->addWidget(status);
            vbox->addLayout(ctrl_row);

            controls_.push_back({ card_key, slider, readout, send_btn, status });
        }

        root->addWidget(group);
    }

    root->addStretch();
    setWindowTitle("VcanSim UI");
    resize(920, 520);
}

void MainWindow::handleDecodedMessage(const DecodedMessage& msg)
{
    const QString msg_name = QString::fromStdString(msg.name);
    for (size_t i = 0; i < msg.columns.size(); ++i) {
        const QString key = msg_name + "/" + QString::fromStdString(msg.columns[i]);
        if (auto* lbl = value_labels_.value(key, nullptr)) {
            lbl->setText(QString::fromStdString(msg.values[i]));
        }
    }
}

void MainWindow::handleSendResult(bool ok, float value)
{
    for (auto& c : controls_) {
        c.status->setStyleSheet(ok ? "color: #30d158;" : "color: #ff453a;");
        c.status->setText(ok ? "Sent ✓" : "Failed ✗");
        if (ok) {
            if (auto* lbl = value_labels_.value(c.card_key, nullptr)) {
                lbl->setText(QString::number(static_cast<double>(value), 'f', 1));
            }
        }
    }
}

void MainWindow::onSliderChanged(int)
{
    for (auto& c : controls_)
        c.status->setText("");
}

void MainWindow::onSendClicked()
{
    for (auto& c : controls_) {
        emit sendRequested(static_cast<float>(c.slider->value()));
        c.status->setStyleSheet("color: #8e8e93;");
        c.status->setText("Sending…");
    }
}