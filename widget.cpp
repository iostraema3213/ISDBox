#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>
#include <QTextCodec>
#include <QSettings>
#include <QClipboard>
#include <QMessageBox>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->timeEdit->setEnabled(false);
    isFirstMessage = false;
    file = nullptr;
    setting = new QSettings("./Setting.ini", QSettings::IniFormat);
    updateDutyTime();

    adInfo ad;
    ad.company = "";
    ad.date = QDateTime::currentDateTime();
    ad.message = "123";
    ad.username = "";

    ui->label->setStyleSheet("color:#DC143C");
    ui->label_2->setStyleSheet("color:#0000FF");
    ui->label_3->setStyleSheet("color:#8B4513");
    ui->pushButton_2->setEnabled(false);
    m_pSystemWatcher = new QFileSystemWatcher();
    connect(m_pSystemWatcher, &QFileSystemWatcher::fileChanged, this, &Widget::onFileChanged);
    timer = new QTimer(this);
    timer->setInterval(300);
    timer->setSingleShot(false);
    connect(timer, &QTimer::timeout, this, [ & ] {
        QTextStream stream(file);
        QString str = stream.readLine();
        if (isFirstMessage == false && !str.isEmpty())
        {
            listenTime = new QDateTime();
            *listenTime = QDateTime::currentDateTime();
            qDebug() << "+++++++++++++++++++++++";
            qDebug() << *listenTime;
            isFirstMessage = true;
        }
        if (!str.isEmpty())
        {
            qDebug()
                    << str;
        }
        if (!str.isEmpty() && str.contains("[ ") && str.contains(" ]") && str.contains(" > "))
        {
            if (str.contains(" - ")) { //疑似广告
                QString dateStr = str.split(" ]")[0].split("[ ").at(1);
                QDateTime date = QDateTime::fromString(dateStr, "yyyy.MM.dd hh:mm:ss");
                //                qDebug() << date;
                QString str1 = str;
                str1.remove(0, 24);
                QString name = str1.split(" > ").at(0);
                //                qDebug() << "名字:" << name;
                QString company = str1.split(" > ").at(1).split(" - ").at(0);
                adInfo ad;
                ad.company = company;
                ad.date = date;
                ad.message = str;
                ad.username = name;
                if (company_adinfo.contains(company)) {
                    if (qAbs(date.toSecsSinceEpoch() -  company_adinfo[company].date.toSecsSinceEpoch()) < 30 * 60) {
                        ui->textBrowser->append(toColorString(str, "#DC143C"));
                        qDebug() << "++++++++++";
                        qDebug() << company_adinfo[company];
                        QApplication::beep();
                        company_adinfo[company] = ad;
                        return;
                    }
                }
                if (!company.isEmpty()) {
                    company_adinfo[company] = ad;
                }
                if (username_adinfo.contains(name)) {
                    if (qAbs(date.toSecsSinceEpoch() -  username_adinfo[name].date.toSecsSinceEpoch()) < 30 * 60) {
                        ui->textBrowser->append(toColorString(str, "#DC143C"));
                        qDebug() << "---------";
                        qDebug() << username_adinfo[name];
                        QApplication::beep();
                        username_adinfo[name] = ad;
                        return;
                    }
                }
                if (!name.isEmpty()) {
                    username_adinfo[name] = ad;
                }
                if (str.split(" - ").at(1).size() > 42) {
                    ui->textBrowser->append(toColorString(str, "#0000FF"));
                    return;
                } else {
                    ui->textBrowser->append(toColorString(str, "#000000"));
                }
            } else if (str.contains("qq") || str.contains("QQ") || str.contains("yy") || str.contains("YY")) {
                ui->textBrowser->append(toColorString(str, "#8B4513"));
            }
        }
    });
}

Widget::~Widget()
{
    if (file != NULL) {
        file->close();
    }
    delete ui;
}

QString Widget::toColorString(QString str, QString color)
{
    return "<font color='" + color + "'>" + str + "</font>";
}

void Widget::onFileChanged(const QString &path)
{
    Q_UNUSED(path);
    //    qDebug() << file->readLine(10);
    //    QTextStream stream(file);
    //    qDebug() << stream.readLine();
}

void Widget::on_pushButton_clicked()
{
    QString lastPath = setting->value("LastFilePath").toString();  //获取上次的打开路径
    QString fileName = QFileDialog::getOpenFileName(this, tr("监听文件"), lastPath);
    if (fileName.isEmpty()) {
        return;
    }
    file = new QFile(fileName);
    file->open(QIODevice::ReadOnly);
    m_pSystemWatcher->addPath(fileName);
    file->readAll();
    timer->start();
    if (!fileName.isEmpty()) {
        setting->setValue("LastFilePath", fileName); //记录路径到QSetting中保存
        setting->sync();
    }
    ui->pushButton->setEnabled(false);
    ui->pushButton_2->setEnabled(true);
    ui->label_4->setText(fileName.split("/").last());
}

void Widget::on_pushButton_2_clicked()
{
    m_pSystemWatcher->removePath(file->fileName());
    file->close();
    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setEnabled(false);
    timer->stop();
    isFirstMessage = false;

    qDebug() << listenTime;
    if (listenTime == NULL || listenTime->isNull()) {
        return;
    }
    qDebug() << QDateTime::currentDateTime();
    int second = qAbs(QDateTime::currentDateTime().toSecsSinceEpoch() - listenTime->toSecsSinceEpoch());
    listenTime = NULL;

    int s = 0;
    int hour = 0;
    int minute = 0;
    if (second > 60) {
        minute = second / 60;
        s = second % 60;
    }
    if (minute > 60) {
        hour = minute / 60;
        minute = minute % 60;
    }
    QTime time(hour, minute, s);
    int r = QMessageBox::question(this, "", QString("是否保存本次值班时间%1").arg(time.toString("hh:mm:ss")),
                                  QMessageBox::Yes, QMessageBox::No);
    if (QMessageBox::Yes == r) {
        QString recordTimeString = setting->value("recordTime").toString();
        if (recordTimeString.isEmpty()) {
            setting->setValue("recordTime", time.toString("hh:mm:ss"));
            setting->sync();
        } else {
            QTime recordTime = QTime::fromString(recordTimeString, "hh:mm:ss");
            recordTime = recordTime.addSecs(second);
            setting->setValue("recordTime", recordTime.toString("hh:mm:ss"));
            setting->sync();
        }
        updateDutyTime();
    }
}

void Widget::on_pushButton_3_clicked()
{
    QDateTime date = QDateTime::currentDateTime();
    QString dateStr = date.toString("yyyy年MM月dd日hh点mm分");
    QString dateStr1 = date.addDays(1).toString("yyyy年MM月dd日hh点mm分");
    QString str = "您好，由于您违反了新手帮助频道发布广告的规则，现于" + dateStr +
                  "禁止您进入新手帮助频道，您可以在" + dateStr1 +
                  "后发邮件到24823610@qq.com申请解禁（回复本邮件不会被解禁），邮件中请提供你现在在新手帮助频道内发言时的提示（非弹出窗品）以及ISD给你发的邮件截图和你人物的名字，特此通知。如果你对此次处理有任何疑问，请加QQ：24823610咨询，谢谢。";
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(str);
}

void Widget::updateDutyTime()
{
    QString recordTimeString = setting->value("recordTime").toString();
    qDebug() << QTime::fromString(recordTimeString, "hh:mm:ss");
    ui->label_6->setText(recordTimeString);
    ui->timeEdit->setTime(QTime::fromString(recordTimeString, "hh:mm:ss"));
    //    ui->label_5->setText("本月执勤时间：" + recordTimeString);
}

void Widget::on_pushButton_4_clicked()
{
    QTime time(0, 0, 0);
    ui->label_6->setText(time.toString("hh:mm:ss"));
    ui->timeEdit->setTime(time);
    setSetting("recordTime", "");
}

void Widget::setSetting(QString key, QString value)
{
    setting->setValue(key, value);
    setting->sync();
}

QString Widget::getSetting(QString key)
{
    return setting->value(key).toString();
}

void Widget::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    qDebug() << "closeEvent ";
    if (ui->pushButton_2->isEnabled()) {
        on_pushButton_2_clicked();
    }
    qDebug() << "closeEvent ";
}
