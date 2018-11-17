#include "deletedialog.h"
#include "ui_deletedialog.h"

#include <QDebug>
#include <QString>
#include <QStyle>
#include <QCommonStyle>
#include <QDesktopWidget>
#include <QThread>
#include <QDebug>
#include <QPalette>
#include <QColor>
#include <QTimer>
#include <algorithm>

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
                           std::vector<QByteArray> *orderedKeys,
                           QDir *root)
        : QDialog(parent), ui(new Ui::DeleteDialog), root(root) {

    ui->setupUi(this);

    ui->treeWidget->clear();
    ui->treeWidget->setUniformRowHeights(true);

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    for (auto group : *orderedKeys) {
        auto values = copies->value(group);
        if (values.size() == 1) continue;
        QTreeWidgetItem * item = new QTreeWidgetItem(ui->treeWidget);
        item->setCheckState(0, Qt::CheckState::Unchecked);

        item->setText(0, QString(root->relativeFilePath(values[0])));
        item->setText(1, QString::number(values.size()));
        qint64 size = 0;
        for (auto file : values) {
            QTreeWidgetItem *subItem = new QTreeWidgetItem(item);
            subItem->setCheckState(0, Qt::CheckState::Unchecked);
            subItem->setText(0, root->relativeFilePath(file));
            item->addChild(subItem);
            size += QFile(file).size();
        }

        if (size / 1024 < 1) {
            item->setText(2, QString::number(size) + " B");
        } else if (size / 1024 / 1024 < 1) {
            item->setText(2, QString::number(size / 1024) + " KB");
        } else if (size / 1024 / 1024 / 1024 < 1) {
            item->setText(2, QString::number(size / 1024 / 1024) + " MB");
        } else {
            item->setText(2, QString::number(size / 1024 / 1024 / 1024) + " GB");
        }
        ui->treeWidget->addTopLevelItem(item);
    }

    ui->progressBar->setStyleSheet(progressBarStileSheetGreen);
    connect(ui->treeWidget, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(clickCheck(QTreeWidgetItem *)));
    connect(ui->deleteButton, SIGNAL(released()), this, SLOT(deleteFiles()));
    connect(ui->cancelButton, SIGNAL(released()), this, SLOT(cancel()));
}

DeleteDialog::~DeleteDialog() {}

void DeleteDialog::clickCheck(QTreeWidgetItem *item) {

    if (item->parent() == nullptr) {
        if (item->checkState(0) == Qt::Unchecked) {
            for (int i = 0; i < item->childCount(); ++i) {
                item->child(i)->setCheckState(0, Qt::Unchecked);
            }
        } else if (item->checkState(0) == Qt::Checked){
            for (int i = 0; i < item->childCount(); ++i) {
               item->child(i)->setCheckState(0, Qt::Checked);
            }
        }
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
