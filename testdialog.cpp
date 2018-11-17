#include "testdialog.h"
#include "ui_testdialog.h"

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


TestDialog::TestDialog(QWidget *parent) : QDialog(parent), ui(new Ui::TestDialog) {
    ui->setupUi(this);
}

TestDialog::~TestDialog() {}
