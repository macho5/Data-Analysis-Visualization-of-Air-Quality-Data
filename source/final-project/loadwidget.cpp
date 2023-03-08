#include "loadwidget.h"
#include "ui_loadwidget.h"
#include "loadthread.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QMessageBox>
#include <QVector>
#include <QTabWidget>
#include <QMessageBox>
#include <QButtonGroup>
#include <QBarSet>
#include <QBarSeries>
#include <QChart>
#include <QGraphicsView>
#include <QLineSeries>
#include <QValueAxis>
#include <QCategoryAxis>

loadWidget::loadWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::loadWidget)
{
    ui->setupUi(this);
    ui->nextButton->setEnabled(false);
    ui->progressBar->setRange(0, 2891830);
    ui->progressBar->setValue(0);
    ui->loadButton->setEnabled(false);
}

loadWidget::~loadWidget()
{
    delete ui;
}

void loadWidget::on_selectStationsButton_clicked()
{
    stationLines = 0;
    QString filename = QFileDialog::getOpenFileName(this, "open", "./", "csv file(*.csv)");
    ui->lineEdit->setText(filename);
    ui->loadButton->setEnabled(true);
}


void loadWidget::on_selectPollutantsButton_clicked()
{
    pollutantsLines = 0;
    QString filename = QFileDialog::getOpenFileName(this, "open", "./", "csv file(*.csv)");
    ui->lineEdit_2->setText(filename);
    ui->loadButton->setEnabled(true);
}


void loadWidget::on_loadButton_clicked()
{
    QString station = ui->lineEdit->text();
    QString pollutants = ui->lineEdit_2->text();
    loadThread *thread = new loadThread(station, pollutants);
    qRegisterMetaType<pollutantPack>("pollutantPack");
    qRegisterMetaType<stationPack>("stationPack");
    connect(thread, &loadThread::started,
            [=](){
        ui->loadButton->setEnabled(false);
    });
    connect(thread, &loadThread::sendStation, this, &loadWidget::handleStation);
    connect(thread, &loadThread::sendPollutants, this, &loadWidget::handlePollutants);
    connect(thread, &loadThread::change, this, &loadWidget::handleProgressBar);
    connect(thread, &loadThread::finished, this, &loadWidget::handleFinished);
    thread->start();
}

void loadWidget::handleProgressBar(int n)
{
    ui->progressBar->setValue(n);
}

void loadWidget::handleFinished(){
    ui->loadButton->setEnabled(true);
    QMessageBox::information(this, "infomation", "Done!");
}

void loadWidget::handleStation(QVector<stationPack> pack){
    for(int i = 0; i < pack.size(); ++i){
        stationPack item = pack[i];
        Kdpack kdp(item.latitude, item.longitude, item.stationID);
        locationTree.insert(kdp);
        IDtoName.push_back({item.stationID, item.stationName});
    }
}

void loadWidget::handlePollutants(QVector<pollutantPack> pack){
    for(int i = 0; i < pack.size(); ++i){
        pollutantPack item = pack[i];
        int ID = item.stationID;
        Info info(Time(item.timestamp), item.pollutants);
        if(data.size() == 0){
            QVector<Info> vec;
            vec.push_back(info);
            data.push_back({ID, vec});
        }
        int lastID = data.at(data.size() - 1).first;
        if(ID == lastID){
            data[data.size()-1].second.push_back(info);
        } else {
            QVector<Info> vec;
            vec.push_back(info);
            data.push_back({ID, vec});
        }
    }

}

bool cmp(pair<int, Record> p1, pair<int, Record> p2){
    return p1.second.value / p1.second.count > p2.second.value / p2.second.count;
}

void loadWidget::on_pushButton_clicked()
{
    if(ui->lineEdit_3->text().isEmpty() || ui->lineEdit_4->text().isEmpty() ||
       ui->lineEdit_5->text().isEmpty() || ui->lineEdit_6->text().isEmpty() ||
       ui->lineEdit_3->text().toFloat() >= ui->lineEdit_4->text().toFloat() ||
       ui->lineEdit_5->text().toFloat() >= ui->lineEdit_6->text().toFloat() ||
       ui->dateTimeEdit->date() >= ui->dateTimeEdit_2->date()){
        QMessageBox::warning(this, "Warning", "Your input is illegal!");
        return;
    }
    QButtonGroup *group = new QButtonGroup;
    group->addButton(ui->radioButton, 0);
    group->addButton(ui->radioButton_3, 1);
    group->addButton(ui->radioButton_5, 2);
    group->addButton(ui->radioButton_2, 3);
    group->addButton(ui->radioButton_4, 4);
    group->addButton(ui->radioButton_6, 5);
    int id = group->checkedId();
    float fromLa = ui->lineEdit_3->text().toFloat();
    float toLa = ui->lineEdit_4->text().toFloat();
    float fromLo = ui->lineEdit_5->text().toFloat();
    float toLo = ui->lineEdit_6->text().toFloat();
    Time fromTime = Time(ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd hh-mm-ss"));
    Time toTime = Time(ui->dateTimeEdit_2->dateTime().toString("yyyy-MM-dd hh-mm-ss"));

    QVector<int> stationInRange = locationTree.rangeSearch(fromLa, toLa, fromLo, toLo);
    QVector<pair<int, Record>> result;

    for(int i = 0; i < stationInRange.size(); ++i){
        QVector<Info> vec;
        for(int j = 0; j < data.size(); ++j){
            if(data[j].first == stationInRange[i]){
                vec = data[j].second;
                break;
            }
        }
        Record record;
        for(int j = 0; j < vec.size(); ++j){
            Info info = vec[j];
            if(info.time < fromTime || info.time > toTime)
                continue;
            if(info.pollutants[id] != 0){
                record.count++;
                record.value += info.pollutants[id];
            }
        }
        result.push_back({stationInRange[i], record});
    }
    sort(result.begin(), result.end(), cmp);

    QBarSeries *series = new QBarSeries();
        for(int i = 0; i < std::min(10, int(result.size())); i++) {
            QString label;
            for(int j = 0; j < IDtoName.size(); ++j){
                if(IDtoName[j].first == result[i].first){
                    label = IDtoName[j].second;
                    break;
                }
            }
            QBarSet *set0 = new QBarSet(label);
            *set0 << (result[i].second.value / result[i].second.count);
            series->append(set0);
        }
        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Top 10 stations with most selected pollutant");
        chart->setAnimationOptions(QChart::SeriesAnimations);
        ui->graphicsView->setChart(chart);
}


void loadWidget::on_pushButton_2_clicked()
{
    if(ui->dateTimeEdit_3->date() >= ui->dateTimeEdit_4->date()){
        QMessageBox::warning(this, "Warning", "Your input is illegal!");
        return;
    }
    QButtonGroup *group = new QButtonGroup;
    group->addButton(ui->radioButton, 0);
    group->addButton(ui->radioButton_3, 1);
    group->addButton(ui->radioButton_5, 2);
    group->addButton(ui->radioButton_2, 3);
    group->addButton(ui->radioButton_4, 4);
    group->addButton(ui->radioButton_6, 5);
    int id = group->checkedId();
    int station = ui->lineEdit_7->text().toInt();
    QVector<Info> vec;
    for(int j = 0; j < data.size(); ++j){
        if(data[j].first == station){
            vec = data[j].second;
            break;
        }
    }
    Time fromTime = Time(ui->dateTimeEdit_3->dateTime().toString("yyyy-MM-dd hh-mm-ss"));
    Time toTime = Time(ui->dateTimeEdit_4->dateTime().toString("yyyy-MM-dd hh-mm-ss"));


    QChart* chart = new QChart();
    QString label;
    for(int j = 0; j < IDtoName.size(); ++j){
        if(IDtoName[j].first == station){
            label = IDtoName[j].second;
            break;
        }
    }
    chart->setTitle("Concentration of " + label + " station");
    QLineSeries *series = new QLineSeries;
    QValueAxis *axisY = new QValueAxis;
    series->setName("concentration");
    QCategoryAxis *axisX = new QCategoryAxis;
    axisX->setTitleText("date");
    axisY->setTitleText("concentration");

    Record record;
    for(int j = 0; j < vec.size(); ++j){
        Info info = vec[j];
        if(info.time < fromTime || info.time > toTime)
            continue;
        if(info.pollutants[id] != 0){
            record.value += info.pollutants[id];
            record.count++;
            axisX->append(info.time, j);
            series->append(j, info.pollutants[id]);
        }
    }
    ui->lineEdit_8->setText(QString("%1").arg(record.value / record.count));
    chart->addSeries(series);
    chart->addAxis(axisX,Qt::AlignBottom);
    series->attachAxis(axisX);
    chart->addAxis(axisY,Qt::AlignLeft);
    series->attachAxis(axisY);
    ui->graphicsView_2->setChart(chart);

}


void loadWidget::on_pushButton_3_clicked()
{
    if(ui->dateTimeEdit_5->date() >= ui->dateTimeEdit_6->date()
       || ui->lineEdit_9->text().isEmpty() || ui->lineEdit_10->text().isEmpty()){
        QMessageBox::warning(this, "Warning", "Your input is illegal!");
        return;
    }
    QButtonGroup *group = new QButtonGroup;
    group->addButton(ui->radioButton, 0);
    group->addButton(ui->radioButton_3, 1);
    group->addButton(ui->radioButton_5, 2);
    group->addButton(ui->radioButton_2, 3);
    group->addButton(ui->radioButton_4, 4);
    group->addButton(ui->radioButton_6, 5);
    int id = group->checkedId();
    int station1 = ui->lineEdit_9->text().toInt();
    int station2 = ui->lineEdit_10->text().toInt();
    Time fromTime = Time(ui->dateTimeEdit_5->dateTime().toString("yyyy-MM-dd hh-mm-ss"));
    Time toTime = Time(ui->dateTimeEdit_6->dateTime().toString("yyyy-MM-dd hh-mm-ss"));

    QVector<Info> vec1;
    for(int j = 0; j < data.size(); ++j){
        if(data[j].first == station1){
            vec1 = data[j].second;
            break;
        }
    }
    QChart* chart1 = new QChart();

    QString label1;
    for(int j = 0; j < IDtoName.size(); ++j){
        if(IDtoName[j].first == station1){
            label1 = IDtoName[j].second;
            break;
        }
    }

    chart1->setTitle("Concentration of " + label1 + " station");
    QLineSeries *series1 = new QLineSeries;
    QValueAxis *axisY1 = new QValueAxis;
    series1->setName("concentration");
    QCategoryAxis *axisX1 = new QCategoryAxis;
    axisX1->setTitleText("date");
    axisY1->setTitleText("concentration");

    Record record1;
    for(int j = 0; j < vec1.size(); ++j){
        Info info = vec1[j];
        if(info.time < fromTime || info.time > toTime)
            continue;
        if(info.pollutants[id] != 0){
            record1.value += info.pollutants[id];
            record1.count++;
            axisX1->append(info.time, j);
            series1->append(j, info.pollutants[id]);
        }
    }
    ui->lineEdit_11->setText(QString("%1").arg(record1.value / record1.count));
    chart1->addSeries(series1);
    chart1->addAxis(axisX1,Qt::AlignBottom);
    series1->attachAxis(axisX1);
    chart1->addAxis(axisY1,Qt::AlignLeft);
    series1->attachAxis(axisY1);
    ui->graphicsView_3->setChart(chart1);

    QVector<Info> vec2;
    for(int j = 0; j < data.size(); ++j){
        if(data[j].first == station2){
            vec2 = data[j].second;
            break;
        }
    }
    QChart* chart2 = new QChart();

    QString label2;
    for(int j = 0; j < IDtoName.size(); ++j){
        if(IDtoName[j].first == station2){
            label2 = IDtoName[j].second;
            break;
        }
    }

    chart2->setTitle("Concentration of " + label2 + " station");
    QLineSeries *series2 = new QLineSeries;
    QValueAxis *axisY2 = new QValueAxis;
    series2->setName("concentration");
    QCategoryAxis *axisX2 = new QCategoryAxis;
    axisX2->setTitleText("date");
    axisY2->setTitleText("concentration");

    Record record2;
    for(int j = 0; j < vec2.size(); ++j){
        Info info = vec2[j];
        if(info.time < fromTime || info.time > toTime)
            continue;
        if(info.pollutants[id] != 0){
            record2.value += info.pollutants[id];
            record2.count++;
            axisX2->append(info.time, j);
            series2->append(j, info.pollutants[id]);
        }
    }
    ui->lineEdit_12->setText(QString("%1").arg(record2.value / record2.count));
    chart2->addSeries(series2);
    chart2->addAxis(axisX2,Qt::AlignBottom);
    series2->attachAxis(axisX2);
    chart2->addAxis(axisY2,Qt::AlignLeft);
    series2->attachAxis(axisY2);
    ui->graphicsView_4->setChart(chart2);
}


void loadWidget::on_pushButton_4_clicked()
{
    if(ui->lineEdit_13->text().isEmpty() || ui->lineEdit_14->text().isEmpty() ||
       ui->lineEdit_15->text().isEmpty() || ui->lineEdit_16->text().isEmpty() ||
       ui->lineEdit_13->text().toFloat() >= ui->lineEdit_14->text().toFloat() ||
       ui->lineEdit_15->text().toFloat() >= ui->lineEdit_16->text().toFloat() ||
       ui->dateTimeEdit_7->date() >= ui->dateTimeEdit_8->date()){
        QMessageBox::warning(this, "Warning", "Your input is illegal!");
        return;
    }
    bool b1 = ui->checkBox->isChecked();
    bool b2 = ui->checkBox_2->isChecked();
    bool b3 = ui->checkBox_3->isChecked();
    bool b4 = ui->checkBox_4->isChecked();
    bool b5 = ui->checkBox_5->isChecked();
    bool b6 = ui->checkBox_6->isChecked();
    if(b1 + b2 + b3 + b4 + b5 + b6 != 2){
        QMessageBox::warning(this, "Warning", "Please choose correct number of pollutants!");
        return;
    }

    QVector<int> id;
    if(b1) id.push_back(0);
    if(b2) id.push_back(1);
    if(b3) id.push_back(2);
    if(b4) id.push_back(3);
    if(b5) id.push_back(4);
    if(b6) id.push_back(5);

    float fromLa = ui->lineEdit_13->text().toFloat();
    float toLa = ui->lineEdit_14->text().toFloat();
    float fromLo = ui->lineEdit_15->text().toFloat();
    float toLo = ui->lineEdit_16->text().toFloat();
    QVector<int> stationInRange = locationTree.rangeSearch(fromLa, toLa, fromLo, toLo);
    Time fromTime = Time(ui->dateTimeEdit_7->dateTime().toString("yyyy-MM-dd hh-mm-ss"));
    Time toTime = Time(ui->dateTimeEdit_8->dateTime().toString("yyyy-MM-dd hh-mm-ss"));

    int count1 = 0, count2 = 0;
    float value1 = 0, value2 = 0;

    for(int i = 0; i < stationInRange.size(); ++i){
        QVector<Info> vec;
        for(int j = 0; j < data.size(); ++j){
            if(data[j].first == stationInRange[i]){
                vec = data[j].second;
                break;
            }
        }
        for(int j = 0; j < vec.size(); ++j){
            Info info = vec[j];
            if(info.time < fromTime || info.time > toTime)
                continue;
            if(info.pollutants[id[0]] != 0){
                count1++;
                value1 += info.pollutants[id[0]];
            }
            if(info.pollutants[id[1]] != 0){
                count2++;
                value2 += info.pollutants[id[1]];
            }
        }
    }

    if(count1 == 0 || count2 == 0){
        QMessageBox::warning(this, "Warning", "There is no data in the range. Please check your input!");
    }

    QBarSeries *series = new QBarSeries();
    QString label[6] = {"PM25", "PM10", "NO2", "CO", "O3", "SO2"};
    QBarSet *set0 = new QBarSet(label[id[0]]);
    *set0 << (value1 / count1);
    series->append(set0);
    QBarSet *set1 = new QBarSet(label[id[1]]);
    *set1 << (value2 / count2);
    series->append(set1);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Difference between " + label[id[0]] + " and " + label[id[1]]);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    ui->graphicsView_5->setChart(chart);

    ui->label_35->setText("Average concentration of " + label[id[0]]);
    ui->label_36->setText("Average concentration of " + label[id[1]]);
    ui->lineEdit_17->setText(QString("%1").arg(value1 / count1));
    ui->lineEdit_18->setText(QString("%1").arg(value2 / count2));
}


void loadWidget::on_pushButton_5_clicked()
{
    bool b1 = ui->checkBox_7->isChecked();
    bool b2 = ui->checkBox_8->isChecked();
    if(b1 && b2){
        QMessageBox::warning(this, "Warning", "Please choose only one check box!");
        return;
    }
    if(!b1 && !b2){
        QMessageBox::warning(this, "Warning", "Please choose one check box!");
        return;
    }

    if(b1){
        float value[6] = {0};
        int count[6] = {0};
        int station = ui->lineEdit_23->text().toInt();

        QString label;
        for(int j = 0; j < IDtoName.size(); ++j){
            if(IDtoName[j].first == station){
                label = IDtoName[j].second;
                break;
            }
        }

        QVector<Info> vec;
        for(int j = 0; j < data.size(); ++j){
            if(data[j].first == station){
                vec = data[j].second;
                break;
            }
        }
        for(int i = 0; i < vec.size(); ++i){
            if(vec[i].time >= Time(2015, 3, 1)){
                for(int j = 0; j < 6; ++j){
                    if(vec[i].pollutants[j] != 0){
                        ++count[j];
                        value[j] += vec[i].pollutants[j];
                    }
                }
            }
        }
        QString text = "In 2015/5, " + label + ", the predicted:\nPM25 concentration is ";
        text.append(QString::number(value[0]/count[0]));
        text.append("\nPM10 concentration is ");
        text.append(QString::number(value[1]/count[1]));
        text.append("\nNO2 concentration is ");
        text.append(QString::number(value[2]/count[2]));
        text.append("\nCO concentration is ");
        text.append(QString::number(value[3]/count[3]));
        text.append("\nO3 concentration is ");
        text.append(QString::number(value[4]/count[4]));
        text.append("\nSO2 concentration is ");
        text.append(QString::number(value[5]/count[5]));
        ui->textEdit->setText(text);
    }

    if(b2){
        float fromLa = ui->lineEdit_19->text().toFloat();
        float toLa = ui->lineEdit_20->text().toFloat();
        float fromLo = ui->lineEdit_21->text().toFloat();
        float toLo = ui->lineEdit_22->text().toFloat();
        QVector<int> stationInRange = locationTree.rangeSearch(fromLa, toLa, fromLo, toLo);
        float value[6] = {0};
        int count[6] = {0};

        for(int k = 0; k < stationInRange.size(); ++k){
            QVector<Info> vec;
            for(int j = 0; j < data.size(); ++j){
                if(data[j].first == stationInRange[k]){
                    vec = data[j].second;
                    break;
                }
            }
            for(int i = 0; i < vec.size(); ++i){
                if(vec[i].time >= Time(2015, 3, 1)){
                    for(int j = 0; j < 6; ++j){
                        if(vec[i].pollutants[j] != 0){
                            ++count[j];
                            value[j] += vec[i].pollutants[j];
                        }
                    }
                }
            }
        }
        QString text = "In 2015/5, in the geographical range, the predicted:\nPM25 concentration is ";
        text.append(QString::number(value[0]/count[0]));
        text.append("\nPM10 concentration is ");
        text.append(QString::number(value[1]/count[1]));
        text.append("\nNO2 concentration is ");
        text.append(QString::number(value[2]/count[2]));
        text.append("\nCO concentration is ");
        text.append(QString::number(value[3]/count[3]));
        text.append("\nO3 concentration is ");
        text.append(QString::number(value[4]/count[4]));
        text.append("\nSO2 concentration is ");
        text.append(QString::number(value[5]/count[5]));
        ui->textEdit->setText(text);
    }
}


void loadWidget::on_pushButton_6_clicked()
{
    bool b1 = ui->checkBox_9->isChecked();
    bool b2 = ui->checkBox_10->isChecked();
    if(b1 && b2){
        QMessageBox::warning(this, "Warning", "Please choose only one check box!");
        return;
    }
    if(!b1 && !b2){
        QMessageBox::warning(this, "Warning", "Please choose one check box!");
        return;
    }

    if(b1){
        int station1 = ui->lineEdit_24->text().toInt();
        int station2 = ui->lineEdit_25->text().toInt();
        QString label1;
        for(int j = 0; j < IDtoName.size(); ++j){
            if(IDtoName[j].first == station1){
                label1 = IDtoName[j].second;
                break;
            }
        }
        QString label2;
        for(int j = 0; j < IDtoName.size(); ++j){
            if(IDtoName[j].first == station2){
                label2 = IDtoName[j].second;
                break;
            }
        }
        QVector<Info> vec1;
        for(int j = 0; j < data.size(); ++j){
            if(data[j].first == station1){
                vec1 = data[j].second;
                break;
            }
        }
        QVector<Info> vec2;
        for(int j = 0; j < data.size(); ++j){
            if(data[j].first == station2){
                vec2 = data[j].second;
                break;
            }
        }
        int size = std::min(vec1.size(), vec2.size());

        float simi[6] = {0};
        int count[6] = {0};

        for(int i = 0; i < size; ++i){
            for(int j = 0; j < 6; ++j){
                if(vec1[i].pollutants[j] == 0 || vec2[i].pollutants[j] == 0) continue;
                ++count[j];
                simi[j] += std::abs(vec1[i].pollutants[j] - vec2[i].pollutants[j]) * 100 / std::max(vec1[i].pollutants[j], vec2[i].pollutants[j]);
            }
        }
        QString text = "The similarity between " + label1 + " and " + label2 + " in:\nPM25 concentration is ";
        text.append(QString::number(100 - simi[0]/count[0]));
        text.append("%\nPM10 concentration is ");
        text.append(QString::number(100 - simi[1]/count[1]));
        text.append("%\nNO2 concentration is ");
        text.append(QString::number(100 - simi[2]/count[2]));
        text.append("%\nCO concentration is ");
        text.append(QString::number(100 - simi[3]/count[3]));
        text.append("%\nO3 concentration is ");
        text.append(QString::number(100 - simi[4]/count[4]));
        text.append("%\nSO2 concentration is ");
        text.append(QString::number(100 - simi[5]/count[5]));
        text.append("%");
        ui->textEdit_2->setText(text);
    }

    if(b2){
        bool c1 = ui->checkBox_11->isChecked();
        bool c2 = ui->checkBox_12->isChecked();
        bool c3 = ui->checkBox_13->isChecked();
        bool c4 = ui->checkBox_14->isChecked();
        bool c5 = ui->checkBox_15->isChecked();
        bool c6 = ui->checkBox_16->isChecked();
        if(c1 + c2 + c3 + c4 + c5 + c6 != 2){
            QMessageBox::warning(this, "Warning", "Please choose correct number of pollutants!");
            return;
        }

        QVector<int> id;
        if(c1) id.push_back(0);
        if(c2) id.push_back(1);
        if(c3) id.push_back(2);
        if(c4) id.push_back(3);
        if(c5) id.push_back(4);
        if(c6) id.push_back(5);

        int count[4] = {0};
        float value[4] = {0};
        QString label[6] = {"PM25", "PM10", "NO2", "CO", "O3", "SO2"};

        for(int i = 0; i < data.size(); ++i){
            QVector<Info> vec = data[i].second;
            for(int i = 0; i < vec.size(); ++i){
                if(vec[i].time <= Time(2015, 1, 1)){
                    if(vec[i].pollutants[id[0]] != 0){
                        ++count[0];
                        value[0] += vec[i].pollutants[id[0]];
                    }
                    if(vec[i].pollutants[id[1]] != 0){
                        ++count[1];
                        value[1] += vec[i].pollutants[id[1]];
                    }
                } else {
                    if(vec[i].pollutants[id[0]] != 0){
                        ++count[2];
                        value[2] += vec[i].pollutants[id[0]];
                    }
                    if(vec[i].pollutants[id[1]] != 0){
                        ++count[3];
                        value[3] += vec[i].pollutants[id[1]];
                    }
                }
            }
        }

        float simi1 = std::abs(value[1]/count[1] - value[0]/count[0]) * 100 / std::max(value[1]/count[1], value[0]/count[0]);
        float simi2 = std::abs(value[3]/count[3] - value[2]/count[2]) * 100 / std::max(value[2]/count[2], value[3]/count[3]);

        QString text = "The average concentration of " + label[id[0]] + " from 2014-5-1 to 2014-12-31 is ";
        text.append(QString::number(value[0]/count[0]));
        text.append("\nThe average concentration of " + label[id[1]] + " from 2014-5-1 to 2014-12-31 is ");
        text.append(QString::number(value[1]/count[1]));
        text.append("\nThe similarity between the concentration of " + label[id[0]] + " and " + label[id[1]] + " in the period is ");
        text.append(QString::number(100 - simi1));
        text.append("%\n\nThe average concentration of " + label[id[0]] + " from 2015-1-1 to 2015-4-30 is ");
        text.append(QString::number(value[2]/count[2]));
        text.append("\nThe average concentration of " + label[id[1]] + " from 2015-1-1 to 2015-4-30 is ");
        text.append(QString::number(value[3]/count[3]));
        text.append("\nThe similarity between the concentration of " + label[id[0]] + " and " + label[id[1]] + " in the period is ");
        text.append(QString::number(100 - simi2));
        text.append("%");
        ui->textEdit_2->setText(text);
    }
}

