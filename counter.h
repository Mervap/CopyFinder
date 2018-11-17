#ifndef COUNTER_H
#define COUNTER_H

#include <QDir>
#include <QMap>
#include <memory>
#include <QVector>
#include <QString>

class Counter : public QObject {
    Q_OBJECT
public:
    Counter(qint64 *size, int *amount, QDir directory);

public
    slots:
        void doWork();

    signals:
        void workDone();
        void updateProgressBar();

private:
    void countFiles(QDir const &dir);

    std::unique_ptr<qint64> size;
    std::unique_ptr<int> amount;
    QDir directory;
    uint directoryAmount;
};


#endif // COUNTER_H
