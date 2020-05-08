#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>
#include <QTextCodec>
#include <QSettings>
#include <QClipboard>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->label->setStyleSheet("color:#DC143C");
    ui->label_2->setStyleSheet("color:#0000FF");
    ui->label_3->setStyleSheet("color:#8B4513");
    ui->pushButton_2->setEnabled(false);
    m_pSystemWatcher = new QFileSystemWatcher();
    connect(m_pSystemWatcher, &QFileSystemWatcher::fileChanged, this, &Widget::onFileChanged);
    timer = new QTimer(this);
    timer->setInterval(1000);
    timer->setSingleShot(false);
    connect(timer, &QTimer::timeout, this, [ = ] {
        QTextStream stream(file);
        QString str = stream.readLine();
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
                company_adinfo[company] = ad;
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
                username_adinfo[name] = ad;
                if (str.split(" - ").at(1).size() > 42) {
                    ui->textBrowser->append(toColorString(str, "#0000FF"));
                    return;
                } else {
                    ui->textBrowser->append(str);
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
    QSettings setting("./Setting.ini",
                      QSettings::IniFormat);  //QSettings能记录一些程序中的信息，下次再打开时可以读取出来
    QString lastPath = setting.value("LastFilePath").toString();  //获取上次的打开路径
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
        setting.setValue("LastFilePath", fileName); //记录路径到QSetting中保存
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
