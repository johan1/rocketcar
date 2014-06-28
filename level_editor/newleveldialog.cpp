#include "newleveldialog.h"
#include "ui_newleveldialog.h"

NewLevelDialog::NewLevelDialog(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::NewLevelDialog) {
    ui->setupUi(this);
    setWindowTitle(QString("Create new Level"));
}

NewLevelDialog::~NewLevelDialog() {
    delete ui;
}

void NewLevelDialog::handleAccepted() {
    editor = LevelEditor(
                ui->nameEdit->text().toStdString(),
                ui->widthBox->value(),
                ui->heightBox->value());
}
