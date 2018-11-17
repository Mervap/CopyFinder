#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QMap>
#include <memory>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private
    slots:
            void

    selectDirectory();

    void startScanning();
    void startHashing();
    void stopScanningOrHashing();
    void showAboutDialog();

    void updateProgressBar();
    void updateProgressBarColor();
    void updateProgress(QString str);
    void showResult();

private:
    std::unique_ptr <Ui::MainWindow> ui;
    QMap<QByteArray, QVector<QString>> hashes;
    QThread *thread = nullptr;

    double mRunner;
    QTimer *progressBarTimer;
    QString progressBarStileSheet;
    QDir currentDir;

    qint64 summarySize = 0;
    int amount = 0;
};

#endif // MAINWINDOW_H
