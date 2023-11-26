#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include<QtSerialPort/QSerialPort>
#include<QtSerialPort/QSerialPortInfo>
#include<QDebug>
#include<QTimer>
#include<QByteArray>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private slots:
    void sendData();
    void readData();

private:
    Ui::Dialog *ui;
    QSerialPort* com;
    QString DataFromComPort;
    bool bDataReceived= false;
    QTimer* timer;
    float fServoAngle[5];
};
#endif // DIALOG_H
