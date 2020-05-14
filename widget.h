#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QFileSystemWatcher>
#include <QFile>
#include <QTimer>
#include <QDateTime>
#include <QMap>
#include <qDebug>
#include <QCloseEvent>
#include <QSettings>

namespace Ui
{
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT
    struct adInfo {
        QString username;
        QString company;
        QString message;
        QDateTime date;
    };

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    QString toColorString(QString str, QString color);
    friend QDebug operator<<(QDebug os, const adInfo &obj)
    {

        os << QString("玩家名：%1\n军团名：%2\n时间戳：%3").arg(obj.username).arg(obj.company.arg(
                                                                                                  obj.date.toString()));

        return os;

    }

private slots:
    void onFileChanged(const QString &path);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();
    void updateDutyTime();

    void on_pushButton_4_clicked();

    void setSetting(QString key, QString value);
    QString getSetting(QString key);

private:
    Ui::Widget *ui;
    QFileSystemWatcher *m_pSystemWatcher;
    QFile *file;
    QTimer *timer;
    QMap<QString, adInfo> company_adinfo;
    QMap<QString, adInfo> username_adinfo;
    QDateTime *listenTime;
    QSettings *setting;
    bool isFirstMessage;
protected :
    void closeEvent(QCloseEvent *event);
};

#endif // WIDGET_H
