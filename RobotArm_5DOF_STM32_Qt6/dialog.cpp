#include "dialog.h"
#include "./ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    com= new QSerialPort();
    com->setPortName("COM4");
    com->setBaudRate(QSerialPort::BaudRate::Baud9600);
    com->setParity(QSerialPort::Parity::NoParity);
    com->setStopBits(QSerialPort::StopBits::OneStop);
    com->setDataBits(QSerialPort::DataBits::Data8);
    com->setFlowControl(QSerialPort::FlowControl::NoFlowControl);
    com->open(QIODevice::ReadWrite);

    if(com->isOpen())
    {
        qDebug()<<"Serial Port is connected!";
        qDebug()<<com->error();
    }
    else
    {
        qDebug()<<"Can't open a COM port";
        qDebug()<<com->error();
    }

    connect(com, SIGNAL(readyRead()), this, SLOT(readData()));
    sendData();

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(sendData()));
    timer->start(500);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::sendData()
{
    unsigned char buff[10];
    fServoAngle[0]= (float)ui->horizontalSlider->value();
    fServoAngle[1]=  (float)ui->horizontalSlider_2->value();
    fServoAngle[2]=  (float)ui->horizontalSlider_3->value();
    fServoAngle[3]= (float) ui->horizontalSlider_4->value();
    fServoAngle[4]= (float) ui->horizontalSlider_5->value();
    for(int i=0; i<5; i++)
    {
        buff[i*2+0]= i;
        buff[i*2+1]= (int)(128+(fServoAngle[i]/100.0f)*180.0f-90.0f);
    }

    com->write((const char*)buff, 10);

    for(int i=0; i<10; i++)
    {
        qDebug()<<QString(QChar::fromLatin1(buff[i]));
    }
}

/*void Dialog::on_pushButton_clicked()
{
    if(com->isOpen())
    {
       // com->write(ui->lineEdit->text().toLatin1()+char(10));
        com->flush(); // Call this func for sending the buffered data immediately
    }
    else
    {
        qDebug()<<"Can't send data becaues the com port is closed!";
    }
}
*/
void Dialog::readData()
{
    if(com->isOpen())
    {
        while(com->bytesAvailable())
        {
            DataFromComPort+= com->readAll();
        }
        if(DataFromComPort.at(DataFromComPort.length()-1)==char(10))
        {
            bDataReceived= true;
        }
        if(bDataReceived)
        {
            qDebug()<<"Data from serial port: "<<DataFromComPort;
           // ui->textEdit->append(DataFromComPort);
            DataFromComPort="";
            bDataReceived= false;
        }
    }
}




