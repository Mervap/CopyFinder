#ifndef DELTEDIALOG_H
#define DELTEDIALOG_H

#include <QByteArray>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include <memory>

namespace Ui {
    class DeleteDialog;
}

class DeleteDialog : public QDialog {
    Q_OBJECT

public:
    explicit DeleteDialog(QWidget *parent = nullptr,
                          QMap<QByteArray, QVector<QString>> *copies = nullptr,
                          std::vector<QByteArray> * ordegedKeys = nullptr,
                          QDir *origin = nullptr);

    ~DeleteDialog();

private
    slots:
        void clickCheck(QTreeWidgetItem *item);
        void deleteFiles();
        void cancel();

private:
    QTreeWidgetItem *createTreeWidgetItem();

    std::unique_ptr <Ui::DeleteDialog> ui;
    std::unique_ptr <QDir> const root;
};

#endif  // DELTEDIALOG_H
