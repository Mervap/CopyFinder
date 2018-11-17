#ifndef HASHCOUNTER_H
#define HASHCOUNTER_H

#include <QDir>
#include <QMap>
#include <memory>
#include <QVector>
#include <QString>

class HashCounter : public QObject {
    Q_OBJECT
public:
    HashCounter(QDir directory, QMap<QByteArray, QVector<QString>> &hashes);

public
    slots:
        void doWork();

    signals:
        void workDone();
        void updateProgressBar();
        void updateProgress(QString str);

private:
    void preCountHash(QDir const &dir);
    void countHash();

    std::unique_ptr<QMap<QByteArray, QVector<QString>>> hashes;
    QMap<qint64, QVector<QString>> preHashes;
    QDir directory;
};

#endif // HASHCOUNTER_H
