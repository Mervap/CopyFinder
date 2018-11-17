#include "deletedialog.h"
#include "ui_deletedialog.h"

#include <QDebug>
#include <QString>

const QString progressBarStileSheetGreen = QString("QProgressBar {"
                                                   "   border: 1px solid black;"
                                                   "   border-radius: 3px;"
                                                   "   text-align: center;"
                                                   "}"
                                                   ""
                                                   "QProgressBar::chunk {"
                                                   "   background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #32CD32, stop:%1 #98FB98, stop:1 #32CD32);}"
);;

DeleteDialog::DeleteDialog(QWidget *parent,
                           QMap<QByteArray, QVector<QString>> *copies,
                           QDir *root)
        : QDialog(parent), ui(new Ui::DeleteDialog), root(root) {
    ui->setupUi(this);
    ui->treeWidget->clear();
    for (auto group : copies->keys()) {
        if (copies->value(group).size() == 1) continue;
        QTreeWidgetItem * item = new QTreeWidgetItem(ui->treeWidget);
        item->setCheckState(0, Qt::CheckState::Unchecked);

        item->setText(0, QString(root->relativeFilePath(copies->value(group)[0])));
        item->setText(1, QString::number(copies->value(group).size()));
        for (auto file : copies->value(group)) {
            QTreeWidgetItem * subItem = new QTreeWidgetItem(item);
            subItem->setCheckState(0, Qt::CheckState::Unchecked);
            subItem->setText(0, root->relativeFilePath(file));
            item->addChild(subItem);
        }
        ui->treeWidget->addTopLevelItem(item);
    }
    ui->treeWidget->sortItems(1, Qt::DescendingOrder);

    ui->progressBar->setStyleSheet(progressBarStileSheetGreen);
    connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem * , int)), this, SLOT(clickCheck(QTreeWidgetItem * )));
    connect(ui->deleteButton, SIGNAL(released()), this, SLOT(deleteFiles()));
    connect(ui->cancelButton, SIGNAL(released()), this, SLOT(cancel()));
}

DeleteDialog::~DeleteDialog() {}

void DeleteDialog::clickCheck(QTreeWidgetItem *item) {

    if (item->checkState(0) == Qt::Unchecked) {
        item->setCheckState(0, Qt::Checked);
        for (int i = 0; i < item->childCount(); ++i) {
            clickCheck(item->child(i), Qt::Checked, false);
        }

        if (item->parent() != nullptr) {
            clickCheck(item->parent(), Qt::Checked, true);
        }
    } else {
        item->setCheckState(0, Qt::Unchecked);
        for (int i = 0; i < item->childCount(); ++i) {
            clickCheck(item->child(i), Qt::Unchecked, false);
        }

        if (item->parent() != nullptr) {
            clickCheck(item->parent(), Qt::Unchecked, true);
        }
    }
}

void DeleteDialog::clickCheck(QTreeWidgetItem *item, Qt::CheckState state, bool p) {

    if (p) {
        int cntChecked = 0;
        int cntPartiallyChecked = 0;
        for (int i = 0; i < item->childCount(); ++i) {
            if (item->child(i)->checkState(0) == Qt::Checked) {
                ++cntChecked;
            } else if (item->child(i)->checkState(0) == Qt::PartiallyChecked) {
                ++cntPartiallyChecked;
            }
        }

        if (cntChecked == item->childCount()) {
            item->setCheckState(0, Qt::Checked);
        } else if (cntChecked + cntPartiallyChecked > 0) {
            item->setCheckState(0, Qt::PartiallyChecked);
        } else {
            item->setCheckState(0, Qt::Unchecked);
        }

    } else {
        item->setCheckState(0, state);
    }
}


void DeleteDialog::deleteFiles() {

    QVector <QString> files;
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i) {
        QTreeWidgetItem * item = ui->treeWidget->topLevelItem(i);
        for (int j = 0; j < item->childCount(); ++j) {
            if (item->child(j)->checkState(0) == Qt::Checked) {
                files.append(item->child(j)->text(0));
            }
        }
    }

    QMessageBox question;
    question.setWindowTitle("Deleting files");
    question.setText(QString("You're going to delete %1 files, are you sure?").arg(files.size()));
    question.setInformativeText("<div style=\"color: red;\">The process cannot be stopped!</div>");
    question.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    question.setDefaultButton(QMessageBox::Cancel);

    if (question.exec() == QMessageBox::Cancel) {
        return;
    }

    ui->progressBar->setMaximum(files.size());
    ui->progressBar->setValue(0);

    for (auto i : files) {
        QFile(root->absoluteFilePath(i)).remove();
        ui->progressBar->setValue(ui->progressBar->value() + 1);
    }

    close();
}

void DeleteDialog::cancel() {
    close();
}
