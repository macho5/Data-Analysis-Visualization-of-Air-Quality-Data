#ifndef LOADWIDGET_H
#define LOADWIDGET_H

#include <QMainWindow>
#include "loadthread.h"
#include <QHash>
#include <QVector>
#include "kdtree.h"
#include <QDateTime>
#include <QString>

using namespace std;

struct Time
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    Time(int _y = 0, int _mo = 0, int _d = 0, int _h = 0, int _mi = 0, int _s = 0)
        :year(_y), month(_mo), day(_d), hour(_h), minute(_mi), second(_s){}
    Time(const Time &o)
        :year(o.year), month(o.month), day(o.day), hour(o.hour), minute(o.minute), second(o.second){}
    Time(const QString &timestamp){
        QString date = timestamp.split(' ').at(0);
        QString time = timestamp.split(' ').at(1);
        QStringList datelist = date.split('-');
        year = datelist.at(0).toInt();
        month = datelist.at(1).toInt();
        day = datelist.at(2).toInt();
        hour = time.split(':').at(0).toInt();
        minute = 0;
        second = 0;
    }
    bool operator<(Time const &oth)
    {
        if(year < oth.year)
            return true;
        else if (year > oth.year)
            return false;
        if(month < oth.month)
            return true;
        else if(month > oth.month)
            return false;
        if(day < oth.day)
            return true;
        else if(day > oth.day)
            return false;
        if(hour < oth.hour)
            return true;
        else if(hour > oth.hour)
            return false;
        if(minute < oth.minute)
            return true;
        else if(minute > oth.minute)
            return false;
        if(second < oth.second)
            return true;
        else
            return false;
    }
    bool operator<=(Time const &oth)
    {
        if(year < oth.year)
            return true;
        else if (year > oth.year)
            return false;
        if(month < oth.month)
            return true;
        else if(month > oth.month)
            return false;
        if(day < oth.day)
            return true;
        else if(day > oth.day)
            return false;
        if(hour < oth.hour)
            return true;
        else if(hour > oth.hour)
            return false;
        if(minute < oth.minute)
            return true;
        else if(minute > oth.minute)
            return false;
        if(second <= oth.second)
            return true;
        else
            return false;
    }

    bool operator>(Time const &oth)
    {
        if(year > oth.year)
            return true;
        else if (year < oth.year)
            return false;
        if(month > oth.month)
            return true;
        else if(month < oth.month)
            return false;
        if(day > oth.day)
            return true;
        else if(day < oth.day)
            return false;
        if(hour > oth.hour)
            return true;
        else if(hour < oth.hour)
            return false;
        if(minute > oth.minute)
            return true;
        else if(minute < oth.minute)
            return false;
        if(second > oth.second)
            return true;
        else
            return false;

    }
    bool operator>=(Time const &oth)
    {
        if(year > oth.year)
            return true;
        else if (year < oth.year)
            return false;
        if(month > oth.month)
            return true;
        else if(month < oth.month)
            return false;
        if(day > oth.day)
            return true;
        else if(day < oth.day)
            return false;
        if(hour > oth.hour)
            return true;
        else if(hour < oth.hour)
            return false;
        if(minute > oth.minute)
            return true;
        else if(minute < oth.minute)
            return false;
        if(second >= oth.second)
            return true;
        else
            return false;

    }

    bool operator==(Time const &oth)
    {
        return year == oth.year && month == oth.month && day == oth.day && hour == oth.hour && minute == oth.minute && second == oth.second;
    }
    operator QString() const
    {
        QString ret;
        ret.append(QString::number(year));
        ret.append('/');
        ret.append(QString::number(month));
        ret.append('/');
        ret.append(QString::number(day));
        ret.append(' ');
        ret.append(QString::number(hour));
        ret.append(':');
        ret.append(QString::number(minute));
        ret.append(':');
        ret.append(QString::number(second));
        return ret;
    }
    Time& operator=(const Time &o)
    {
        if (this == &o)
            return *this;
        year = o.year;
        month = o.month;
        minute = o.minute;
        second = o.second;
        day = o.day;
        hour = o.hour;
        return *this;
    }
    //两个日期差多少天 ，必须大的减小的
    friend int operator-(const Time &a, const Time &b)
    {
        QDate end(a.year, a.month, a.day);
        QDate sta(b.year, b.month, b.day);
        int cnt = 0;

        while(sta < end)
        {
            ++cnt;
            sta = sta.addDays(1);
        }
        return cnt;
    }
};
struct Info
{
    Time time;
    float pollutants[6];
    Info(Time t, float p[]) :
        time(t){
        for(int i = 0; i < 6; ++i){
            pollutants[i] = p[i];
        }
    }
};

struct Record{
    int count;
    float value;
    Record(){
        count = 0;
        value = 0;
    }
};



namespace Ui {
class loadWidget;
}

class loadWidget : public QMainWindow
{
    Q_OBJECT

public:
    explicit loadWidget(QWidget *parent = nullptr);

    ~loadWidget();

private slots:
    void on_selectStationsButton_clicked();

    void on_selectPollutantsButton_clicked();

    void on_loadButton_clicked();
    void handleProgressBar(int n);
    void handleFinished();
    void handleStation(QVector<stationPack>);
    void handlePollutants(QVector<pollutantPack>);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

private:
    Ui::loadWidget *ui;
    int stationLines = 0;
    int pollutantsLines = 0;
    bool stationReady = false;
    bool pollutantsReady = false;
    QVector<pair<int, QVector<Info>>> data;
    QVector<pair<int, QString>> IDtoName;
    Kdtree locationTree;
    bool isLoaded;
};

#endif // LOADWIDGET_H
