#include "joinwindow.h"
#include <iostream>
#include "./ui_joinwindow.h"
#include "errorHandler.h"
#include "queryTemplates.h"
#include "sonicSocket.h"
#include "windowcontroller.h"

JoinWindow::JoinWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::JoinWindow) {
    ui->setupUi(this);
}

JoinWindow::~JoinWindow() {
    delete ui;
}

void JoinWindow::on_JoinButton_clicked() {
    using client::queries::join_query;
    using client::web::socket;

    bool is_correct_id = true;
    int id = ui->lineEdit->displayText().toInt(&is_correct_id);
    QString response = socket().query(join_query(id));
    qDebug() << "join result: " << response;
    //error_alert("Error while joining", "Wrong ID");
    auto &controller = FastTyping::WindowController::getInstance();
    controller.setActiveWindow("GameWindow");
}

void JoinWindow::on_ReturnButton_clicked() {
    auto &controller = FastTyping::WindowController::getInstance();
    controller.setActiveWindow("MainWindow");
}
