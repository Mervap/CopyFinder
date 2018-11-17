#include "hashcounter.h"
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QCryptographicHash>
#include <QThread>

HashCounter::HashCounter(QDir directory, QMap<QByteArray, QVector<QString>> &hashes) : hashes(&hashes),
                                                                                        directory(directory) {}


void HashCounter::countHash() {
    for (auto group : preHashes) {

        if (QThread::currentThread()->isInterruptionRequested()) {
            return;
        }

        if (group.size() < 2) {
            continue;
        }

        for(auto item : group) {
            QByteArray result;
            QCryptographicHash hash(QCryptographicHash::Sha256);
            QFile f(item);
            if (f.open(QFile::ReadOnly)) {
                hash.addData(&f);

                QByteArray result = hash.result();
                if (hashes->find(result) == hashes->end()) {
                    hashes->insert(result, QVector<QString>(1, item));
                } else {
                    (*hashes)[result].push_back(item);
                    emit updateProgress(HashCounter::directory.relativeFilePath(item) +
                                        " may be a copy of " +
                                        HashCounter::directory.relativeFilePath((*hashes)[result][0]) + "\n");
                }
            }
        }
    }
}

void HashCounter::preCountHash(QDir const &directory) {
    QDirIterator it(directory);
    while (it.hasNext()) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            return;
        }

        it.next();
        if (!it.fileInfo().isSymLink() && it.fileInfo().isDir()) {
            if (it.fileName() != "." && it.fileName() != "..") {
                preCountHash(QDir(it.filePath()));
            }
        } else {
            QByteArray result;
            QCryptographicHash hash(QCryptographicHash::Sha256);

            if (it.fileInfo().isSymLink()) {
                hash.addData(it.fileInfo().symLinkTarget().toUtf8());
                result = hash.result();
                if (hashes->find(result) == hashes->end()) {
                    hashes->insert(result, QVector<QString>(1, it.filePath()));
                } else {
                    (*hashes)[result].push_back(it.filePath());
                    emit updateProgress(HashCounter::directory.relativeFilePath(it.filePath()) +
                                        " may be a copy of " +
                                        HashCounter::directory.relativeFilePath((*hashes)[result][0]) + "\n");
                }
            } else {
                QFile f(it.filePath());
                if (f.open(QFile::ReadOnly)) {

                    qint64 result = f.size();
                    if (preHashes.find(result) == preHashes.end()) {
                        preHashes.insert(result, QVector<QString>(1, f.fileName()));
                    } else {
                        preHashes[result].push_back(f.fileName());
                    }
                }
            }
            emit updateProgressBar();
        }
    }
}

void HashCounter::doWork() {
    preCountHash(directory);
    countHash();
    if (!QThread::currentThread()->isInterruptionRequested()) {
        emit workDone();
    }

    QThread::currentThread()->quit();
}
