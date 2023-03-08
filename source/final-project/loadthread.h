#ifndef LOADTHREAD_H
#define LOADTHREAD_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QVector>

struct pollutantPack{
    int stationID;
    QString timestamp;
    float pollutants[6];
    pollutantPack(){}
};

struct stationPack
{
    int stationID;
    QString stationName;
    float latitude;
    float longitude;

    stationPack(){}
};

class loadThread : public QThread
{
    Q_OBJECT
public:
    loadThread(QString, QString);

signals:
    void change(int);
    void sendPollutants(QVector<pollutantPack>);
    void sendStation(QVector<stationPack>);

private:
    void run() override;
    QString station;
    QString pollutant;
};

#endif // LOADTHREAD_H
