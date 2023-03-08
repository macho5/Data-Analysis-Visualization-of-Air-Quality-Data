#include "loadthread.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QMessageBox>
#include <QVector>
#include <QDebug>

loadThread::loadThread(QString filename1, QString filename2)
{
    station = filename1;
    pollutant = filename2;
}

void loadThread::run(){
    int cnt = 0;
    QVector<stationPack> vec;
    QFile inputFile(station);
    if(!inputFile.open(QIODevice::ReadOnly)) return;
    QTextStream inputTextStream(&inputFile);
    inputTextStream.readLine();
    while(!inputTextStream.atEnd()){
        ++cnt;
        QString line = inputTextStream.readLine();
        QStringList row = line.split(",");
        if(row.size() < 4) break;
        stationPack spack;
        spack.stationID = row[0].toInt();
        spack.stationName = row[1];
        spack.latitude = row[2].toFloat();
        spack.longitude = row[3].toFloat();
        vec.push_back(spack);
        if(cnt % 10 == 0){
            emit change(cnt);
        }
        if(vec.size() == 10){
            emit sendStation(vec);
            vec.clear();
        }
    }
    emit sendStation(vec);
    vec.clear();
    inputFile.close();


    QVector<pollutantPack> vec2;
    QFile inputFile2(pollutant);
    if(!inputFile2.open(QIODevice::ReadOnly)) return;
    QTextStream inputTextStream2(&inputFile2);
    inputTextStream2.readLine();
    while(!inputTextStream2.atEnd()){
        ++cnt;
        QString line = inputTextStream2.readLine();
        QStringList row = line.split(",");
        if(row.size() < 8) break;
        pollutantPack ppack;
        ppack.stationID = row[0].toInt();
        ppack.timestamp = row[1];
        for(int i = 0; i < 6; ++i){
            if(row[2 + i].toStdString() == "NULL")
                row[2 + i] = 0;
            ppack.pollutants[i] = row[2 + i].toFloat();
        }
        vec2.push_back(ppack);
        if(cnt % 10 == 0){
            emit change(cnt);
        }
        if(vec2.size() == 10){
            emit sendPollutants(vec2);
            vec2.clear();
        }
    }
    emit change(cnt);
    emit sendPollutants(vec2);
    vec2.clear();
    inputFile.close();


}
