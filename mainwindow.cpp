#include "hashcounter.h"
#include "counter.h"
#include "mainwindow.h"
#include "testdialog.h"
#include "deletedialog.h"
#include "ui_mainwindow.h"
#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QThread>
#include <QDebug>
#include <QPalette>
#include <QColor>
#include <QTimer>

const QString progressBarStileSheetGreen = QString("QProgressBar {"
                                                   "   border: 1px solid black;"
                                                   "   border-radius: 3px;"
                                                   "   text-align: center;"
                                                   "}"
                                                   ""
                                                   "QProgressBar::chunk {"
                                                   "   background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #32CD32, stop:%1 #98FB98, stop:1 #32CD32);}"
);;

const QString progressBarStileSheetRed = QString("QProgressBar {"
                                                 "   border: 1px solid black;"
                                                 "   border-radius: 3px;"
                                                 "   text-align: center;"
                                                 "}"
                                                 ""
                                                 "QProgressBar::chunk {"
                                                 "   background-color: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #DC143C, stop:%1 #F08080, stop:1 #DC143C);}"
);;

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    QCommonStyle style;
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));

    progressBarTimer = new QTimer(this);
    connect(progressBarTimer, SIGNAL(timeout()), this, SLOT(updateProgressBarColor()));

    progressBarStileSheet = progressBarStileSheetGreen;
    ui->progressBar->setStyleSheet(progressBarStileSheet.arg(mRunner));

    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
    connect(ui->selectDirectory, SIGNAL(released()), this, SLOT(selectDirectory()));
    connect(ui->findCopies, SIGNAL(released()), this, SLOT(startScanning()));
    connect(ui->cancelButton, SIGNAL(released()), this, SLOT(stopScanningOrHashing()));
}

MainWindow::~MainWindow() {
    delete progressBarTimer;
}

void MainWindow::selectDirectory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->directoryPath->clear();
    ui->directoryPath->insert(dir);
    ui->progressBar->setValue(0);
}

void countFiles(qint64 &size, int &amount, QDir const &dir) {
    QDirIterator it(dir);
    while (it.hasNext()) {
        it.next();
        if (it.fileInfo().isDir()) {
            if (it.fileName() != "." && it.fileName() != "..") {
                countFiles(size, amount, QDir(it.filePath()));
            }
        } else {
            ++amount;
            size += it.fileInfo().size();
        }
    }
}

void MainWindow::startScanning() {

    if (thread != nullptr) {
        return;
    }

    ui->progress->clear();
    mRunner = 0.1;
    progressBarTimer->start(40);
    progressBarStileSheet = progressBarStileSheetGreen;

    QString dir = ui->directoryPath->text();
    if (dir == "") {
        ui->progress->append("<div style=\"color: red;\"><b>WARNING:</b> Please, input a directory</div>");
        return;
    }

    currentDir = QDir(dir);
    if (!currentDir.exists()) {
        ui->progress->append("<div style=\"color: red;\"><b>WARNING:</b> No such directory</div>");
    } else {
        ui->findCopies->setEnabled(false);
        ui->progressBar->setValue(0);
        ui->progress->append(QString("Starting process for %1").arg(currentDir.path()));
        ui->progress->append("Counting files...");
        ui->progressBar->setMaximum(static_cast<int>(currentDir.count()));

        summarySize = 0;
        amount = 0;
        thread = new QThread;
        Counter *counter = new Counter(&summarySize, &amount, currentDir);

        counter->moveToThread(thread);
        connect(counter, SIGNAL(updateProgressBar()), this, SLOT(updateProgressBar()));
        connect(thread, SIGNAL(started()), counter, SLOT(doWork()));
        connect(counter, SIGNAL(workDone()), this, SLOT(startHashing()));

        thread->start();
    }
}

void MainWindow::startHashing() {
    progressBarTimer->stop();
    thread->wait();

    delete thread;
    thread = nullptr;

    progressBarTimer->start(40);
    progressBarStileSheet = progressBarStileSheetGreen;

    summarySize /= 1024 * 1024 * 1024;

    if (summarySize > 0) {
        QMessageBox question;
        question.setWindowTitle("Big size of files");
        question.setText(QString("There is more then %1GB, operation might take a long time.").arg(summarySize));
        question.setInformativeText("Are you sure you want to find all copies in this directory?");
        question.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        question.setDefaultButton(QMessageBox::Cancel);

        if (question.exec() == QMessageBox::Cancel) {
            ui->progress->append("Process canceled by user");
            return;
        }
    }

    ui->progress->append("Hashing files...");

    ui->progressBar->setMaximum(amount);
    ui->progressBar->setValue(0);

    hashes.clear();

    thread = new QThread;
    HashCounter *hashCounter = new HashCounter(currentDir, hashes);

    hashCounter->moveToThread(thread);

    connect(hashCounter, SIGNAL(updateProgressBar()), this, SLOT(updateProgressBar()));
    connect(hashCounter, SIGNAL(updateProgress(QString)), this, SLOT(updateProgress(QString)));
    connect(thread, SIGNAL(started()), hashCounter, SLOT(doWork()));
    connect(hashCounter, SIGNAL(workDone()), this, SLOT(showResult()));

    thread->start();
}

void MainWindow::showResult() {
    progressBarTimer->stop();
    thread->wait();

    delete thread;
    thread = nullptr;

    ui->progress->append("Process finished");

    bool flag = false;
    for (auto it : hashes) {
        if (it.size() > 1) {
            flag = true;
            break;
        }
    }

    if (!flag) {
        ui->progress->append("No duplicates found");
    } else {
        ui->progressBar->setMaximum(ui->progressBar->maximum() + 1);
        std::vector<std::pair<int64_t, QByteArray>> keys;
        for (auto group : hashes.keys()) {
            auto values = hashes.value(group);
            int64_t size = QFile(values[0]).size();
            keys.push_back({size * values.size(), group});
        }
        std::sort(keys.begin(), keys.end(), std::greater<std::pair<int64_t, QByteArray>>());

        std::vector<QByteArray> orderedKeys(keys.size());
        for (size_t i = 0; i < keys.size(); ++i) {
            orderedKeys[i] = keys[i].second;
        }

        ui->progressBar->setValue(ui->progressBar->value() + 1);

        DeleteDialog result(nullptr, &hashes, &orderedKeys, &currentDir);
        result.exec();
    }

    ui->findCopies->setEnabled(true);
}

void MainWindow::updateProgressBar() {
    if (thread != nullptr) {
        ui->progressBar->setValue(ui->progressBar->value() + 1);
    }
}

void MainWindow::updateProgress(QString str) {
    if (thread != nullptr) {
        ui->progress->append(str);
    }
}

void MainWindow::showAboutDialog() {
    //QMessageBox::aboutQt(this);
    TestDialog result(nullptr);
    result.exec();
}

void MainWindow::stopScanningOrHashing() {
    if (thread != nullptr && thread->isRunning()) {
        ui->progressBar->setStyleSheet(progressBarStileSheetRed.arg(mRunner));
        progressBarTimer->stop();
        thread->requestInterruption();
        thread->wait();

        delete thread;
        thread = nullptr;
        ui->progress->append("<br/><div style=\"color: red;\"><b>WARNING:</b> Work was canceled by user</div>");
        ui->findCopies->setEnabled(true);
    }
}

void MainWindow::updateProgressBarColor() {

    if (!progressBarTimer->isActive()) {
        return;
    }

    ui->progressBar->setStyleSheet(progressBarStileSheet.arg(mRunner));

    mRunner += 0.01;
    if (mRunner > 1) {
        mRunner = 0.1;
    }
}
