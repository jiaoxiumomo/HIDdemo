#include "mainwindow.h"
#include "hidapi.h"
#include <QMessageBox>

const int OUT_REPORT_SIZE = 2;
const int IN_REPORT_SIZE = 2;
const int TIME_OUT = 5;
const bool NO_REPORT_ID = true;
unsigned char UsbTxData[64] = {0},Rxbuffer[64] = {0};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
  nHID_VID=0x0483;
  nHID_PID=0x5750;
  hDev = NULL;
  setWindowTitle(tr("Custom HID Demonstration"));
  setWindowIcon(QIcon(":/img/HID"));
  createMenu();
  refreshpanelInfo();
  timer = new QTimer(this);
  timer->start(100); // 每隔0.1s
  connect(timer, SIGNAL(timeout()), this, SLOT(refreshLed()));
}
MainWindow::~MainWindow()
{
    delete timer;
    hid_close(hDev);
}
void MainWindow::createMenu()
{

    //创建主分割窗口
    mainSplitter=new QSplitter(Qt::Vertical,0);
    //USB Target
    demoShow=new QPushButton;
    demoShow->setText(tr("Find"));
    demoShow->setFixedSize(60,25);
    usbTargetLabel=new QLabel;
    usbTargetLabel->setText(tr("USB HID Target"));
    usbTargetLabel->setFont(QFont("Timers",10,QFont::Bold));
    usbTargetLabel->setFixedWidth(115);
    usbTargetCombox=new QComboBox;

    VIDTest=new QLabel;
    VIDTest->setText(tr("VID: 0x"));
    PIDTest=new QLabel;
    PIDTest->setText(tr("PID: 0x"));
    VIDNumber=new QLineEdit;
    QString str = QString("%1").arg(nHID_VID,4,16,QLatin1Char('0'));
    VIDNumber->setText(str);//QString::number()
    QRegExp regExp("[a-fA-F0-9]{4}");
    VIDNumber->setValidator(new QRegExpValidator(regExp, this));
    PIDNumber=new QLineEdit;
    str = QString("%1").arg(nHID_PID,4,16,QLatin1Char('0'));
    PIDNumber->setText(str);
    PIDNumber->setValidator(new QRegExpValidator(regExp, this));

    layout1=new QHBoxLayout;
    layout1->addWidget(usbTargetLabel);
    layout1->addWidget(usbTargetCombox);
    layout1->addWidget(demoShow);
    layout2=new QHBoxLayout;
    layout2->addWidget(VIDTest);
    layout2->addWidget(VIDNumber);
    layout2->addWidget(PIDTest);
    layout2->addWidget(PIDNumber);
    upSplitter=new QVBoxLayout;
    upSplitter->addLayout(layout1);
    upSplitter->addLayout(layout2);
    widget1=new QWidget(mainSplitter);
    widget1->setLayout(upSplitter);
    widget1->setFixedHeight(80);
    //创建下面的分割窗口
    underSplitter=new QSplitter(Qt::Horizontal,mainSplitter);
    //左窗口
    switch0Icon.addFile(":/img/SWOFF");
    switch1Icon.addFile(":/img/SWON");
    switchFaceGroup=new QButtonGroup;
    for(int i=0;i<4;i++)
    {
        switchFace[i]=new QPushButton;
        switchFace[i]->setIcon(switch0Icon);
        switchFace[i]->setIconSize(QSize(70,30));
        switchFace[i]->setFixedSize(70,30);
        switchFace[i]->setCheckable(true);
        switchFace[i]->setFlat(true);
        switchFace[i]->setStyleSheet("background: transparent;");
        switchFaceGroup->addButton(switchFace[i],i);     
        switchTitle[i]=new QLabel;
        switchTitle[i]->setAlignment(Qt::AlignCenter);
        switchTitle[i]->setText(tr("LED")+QString::number(i+1));
        switchTitle[i]->setFont(QFont("Timers",12,QFont::Bold));
    }    
    switchFaceGroup->setExclusive(false);//设置是否具有排他性
    switchLayout=new QGridLayout;
    switchLayout->addWidget(switchTitle[0],0,0,1,1);
    switchLayout->addWidget(switchFace[0],0,1,1,1);
    switchLayout->addWidget(switchTitle[1],1,0,1,1);
    switchLayout->addWidget(switchFace[1],1,1,1,1);
    switchLayout->addWidget(switchTitle[2],2,0,1,1);
    switchLayout->addWidget(switchFace[2],2,1,1,1);
    switchLayout->addWidget(switchTitle[3],3,0,1,1);
    switchLayout->addWidget(switchFace[3],3,1,1,1);
    widget2=new QWidget(underSplitter);
    widget2->setLayout(switchLayout);
    //右窗口
    ledONImage.load(":/img/LED1");
    ledOffImage.load(":/img/LED0");
    ledTitle = new QLabel("Five-direction rocker");
    for(int i=0;i<5;i++)
    {
        ledFace[i]=new QLabel;
        ledFace[i]->setFixedSize(32,32);
        ledFace[i]->setScaledContents(true);
        ledFace[i]->setPixmap(ledOffImage);
    }
    ledLayout=new QGridLayout;
    ledLayout->addWidget(ledFace[0],0,1,1,1);
    ledLayout->addWidget(ledFace[1],1,0,1,1);
    ledLayout->addWidget(ledFace[2],1,1,1,1);
    ledLayout->addWidget(ledFace[3],1,2,1,1);
    ledLayout->addWidget(ledFace[4],2,1,1,1);
    widget3=new QWidget(underSplitter);
    widget3->setLayout(ledLayout);
    underSplitter->setStyleSheet("QSplitter::handle { background-color: gray }");
    underSplitter->setHandleWidth(1);
    mainSplitter->setStyleSheet("QSplitter::handle { background-color: gray }");
    mainSplitter->setHandleWidth(1);
    setCentralWidget(mainSplitter);
    this->resize(QSize(400,300));//设置初始大小
    mainSplitter->show();
    connect(switchFaceGroup,SIGNAL(buttonToggled(int,bool)),this,SLOT(onSwitchToggle(int,bool)));
    connect(switchFaceGroup,SIGNAL(buttonToggled(int,bool)),this,SLOT(SwitchChange(int,bool)));
    connect(demoShow,SIGNAL(clicked(bool)),this,SLOT(refreshpanelInfo()));
}
//开关
void MainWindow::SwitchChange(int index, bool value)
{
    if(hDev)
    {
        UsbTxData[0] = index+1;
        UsbTxData[1] = value;
        nBytesWrite = hid_write(hDev, UsbTxData, OUT_REPORT_SIZE, NO_REPORT_ID);//USB发送数据
    }
}
void MainWindow::onSwitchToggle(int index, bool state)
{
    if(state)
        switchFace[index]->setIcon(switch1Icon);// switchonImage
    else
        switchFace[index]->setIcon(switch0Icon);//switchoffImage
}

//led灯

void MainWindow::refreshLed()
{   
    hInfo=hid_enumerate(nHID_VID,nHID_PID);
    if(!hInfo)
    {
      usbTargetCombox->clear();
    }
    hid_free_enumeration(hInfo);

    if(hDev)
    {       
        nBytesRead = hid_read_timeout(hDev, Rxbuffer, IN_REPORT_SIZE, TIME_OUT);
        if (nBytesRead > 0)
        {
            switch (Rxbuffer[0])
            {
            case 1:
                showLed(3,Rxbuffer[1]);
                break;
            case 2:
                showLed(0,Rxbuffer[1]);
                break;
            case 3:
                showLed(1,Rxbuffer[1]);
                break;
            case 4:
                showLed(4,Rxbuffer[1]);
                break;
            case 5:
                showLed(5,Rxbuffer[1]);
                break;
            }
        }
   }
}
void MainWindow::showLed(int index, bool state)
{
    if (state)
           ledFace[index]->setPixmap(ledONImage);
       else
           ledFace[index]->setPixmap(ledOffImage);
}

void MainWindow::refreshpanelInfo()
{
    bool ok;
    usbTargetCombox->clear();
    nHID_VID=VIDNumber->text().toInt(&ok, 16);
    nHID_PID=PIDNumber->text().toInt(&ok, 16);
    hInfo=hid_enumerate(nHID_VID,nHID_PID);
    if(hInfo)
    {
        panelInfo = QString::fromWCharArray(hInfo->product_string);
        usbTargetCombox->addItem(panelInfo);
        hid_free_enumeration(hInfo);
        if(hDev)
        {
            hid_close(hDev);
        }
        hDev= hid_open(nHID_VID, nHID_PID, NULL);
    }
    else
    {
        hDev=NULL;
    }
}

/*
int MainWindow::HID_Demo()
{

        if (hDev == NULL) //open device failure
        {
            retVal = ERROR_NOTYET_CONNECT; //[XTJ,20180126] 原ERROR_OPEN_JUPOD;
        }else//success
        {
            retVal = ERROR_CONNECT_JUPOD;

            val = 0x01; UsbTxData[0] = 0x00; count = 0;
            UsbTxData[0] = 0x01;
            UsbTxData[1] = 0x01;
            nBytesWrite = hid_write(hDev, UsbTxData, OUT_REPORT_SIZE, NO_REPORT_ID);//USB发送数据
            if (nBytesWrite<0)
                return ERROR_CONNECT_JUPOD;
            for (int i=1; i<50; i++)
            {
                Sleep(300);
                nBytesRead = hid_read_timeout(hDev, Rxbuffer, IN_REPORT_SIZE, TIME_OUT);//
                if (nBytesRead > 0)
                {
                    UsbTxData[1] = 0x00;
                    nBytesWrite = hid_write(hDev, UsbTxData, OUT_REPORT_SIZE, NO_REPORT_ID);//USB发送数据
                    count = 0;
                    switch (Rxbuffer[0])
                    {
                    case 1:
                        UsbTxData[0] = 0x01;
                        UsbTxData[1] = Rxbuffer[1] & 0x01;
                        nBytesWrite = hid_write(hDev, UsbTxData, OUT_REPORT_SIZE, NO_REPORT_ID);//USB发送数据
                        break;
                    case 2:
                        UsbTxData[0] = 0x02;
                        UsbTxData[1] = Rxbuffer[1] & 0x01;
                        nBytesWrite = hid_write(hDev, UsbTxData, OUT_REPORT_SIZE, NO_REPORT_ID);//USB发送数据
                        break;
                    case 3:
                        UsbTxData[0] = 0x03;
                        UsbTxData[1] = Rxbuffer[1] & 0x01;
                        nBytesWrite = hid_write(hDev, UsbTxData, OUT_REPORT_SIZE, NO_REPORT_ID);//USB发送数据
                        break;
                    case 4:
                        UsbTxData[0] = 0x04;
                        UsbTxData[1] = Rxbuffer[1] & 0x01;
                        nBytesWrite = hid_write(hDev, UsbTxData, OUT_REPORT_SIZE, NO_REPORT_ID);//USB发送数据
                        break;
                    default:
                        UsbTxData[0] = 0x01;
                        UsbTxData[1] = 0x01;
                        nBytesWrite = hid_write(hDev, UsbTxData, OUT_REPORT_SIZE, NO_REPORT_ID);//USB发送数据
                        UsbTxData[0] = 0x02;
                        UsbTxData[1] = 0x01;
                        nBytesWrite = hid_write(hDev, UsbTxData, OUT_REPORT_SIZE, NO_REPORT_ID);//USB发送数据
                        UsbTxData[0] = 0x03;
                        UsbTxData[1] = 0x01;
                        nBytesWrite = hid_write(hDev, UsbTxData, OUT_REPORT_SIZE, NO_REPORT_ID);//USB发送数据
                        UsbTxData[0] = 0x04;
                        UsbTxData[1] = 0x01;
                        nBytesWrite = hid_write(hDev, UsbTxData, OUT_REPORT_SIZE, NO_REPORT_ID);//USB发送数据
                        break;
                    }
                }
                else
                {
                    if (count++ > 5)
                    {
                        UsbTxData[1] = 0x00;
                        nBytesWrite = hid_write(hDev, UsbTxData, OUT_REPORT_SIZE, NO_REPORT_ID);//USB发送数据

                        UsbTxData[0] = 0x01 + (UsbTxData[0] & 0x03); //Report ID
                        UsbTxData[1] = 0x01;
                        nBytesWrite = hid_write(hDev, UsbTxData, OUT_REPORT_SIZE, NO_REPORT_ID);//USB发送数据
                    }
                }
            }
        }

        return retVal;

}
*/

