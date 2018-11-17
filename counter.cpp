#include "counter.h"

#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QThread>
#include <QDebug>

Counter::Counter(qint64 *size, int *amount, QDir directory) : size(size), amount(amount), directory(directory),
                                                              directoryAmount(0) {}

void Counter::countFiles(QDir const &dir) {
    QDirIterator it(dir);
    while (it.hasNext()) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            return;
        }

        it.next();
        if (!it.fileInfo().isSymLink() && it.fileInfo().isDir()) {
            if (it.fileName() != "." && it.fileName() != "..") {
                countFiles(QDir(it.filePath()));
            }
        } else {
            ++(*amount);
            if (!it.fileInfo().isSymLink()) {
                *size += it.fileInfo().size();
            }
        }

        if (dir == directory) {
            emit updateProgressBar();
        }
    }
}

void Counter::doWork() {
    directoryAmount = directory.count();
    countFiles(directory);
    if (!QThread::currentThread()->isInterruptionRequested()) {
        emit workDone();
    }

    QThread::currentThread()->quit();
}
