#ifndef NEWLEVELDIALOG_H
#define NEWLEVELDIALOG_H

#include <QDialog>
#include "leveleditor.h"

namespace Ui {
class NewLevelDialog;
}

class NewLevelDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit NewLevelDialog(QWidget *parent = 0);
    ~NewLevelDialog();

    LevelEditor &getEditor() {
        return editor;
    }

private:
    LevelEditor editor;
    Ui::NewLevelDialog *ui;

private slots:
    void handleAccepted();
};

#endif // NEWLEVELDIALOG_H
