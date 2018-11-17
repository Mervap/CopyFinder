#ifndef TestDialog_H
#define TestDialog_H

#include <QByteArray>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <memory>

namespace Ui {
    class TestDialog;
}

class TestDialog : public QDialog {
    Q_OBJECT

public:
    explicit TestDialog(QWidget *parent = nullptr);

    ~TestDialog();

private:
    std::unique_ptr <Ui::TestDialog> ui;
};

#endif  // TestDialog_H
