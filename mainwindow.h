#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLayout>
#include <QIcon>
#include <QImage>
#include <QWidget>
#include <QPixmap>
#include <QSize>
#include <QFont>
#include <QButtonGroup>
#include <QTimer>
#include <QLineEdit>
#include "hidapi.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int nHID_VID,nHID_PID;

protected:
    int nBytesWrite,nBytesRead;
    void createMenu();
    void showLed(int index, bool state);
private:
    QSplitter *mainSplitter,*underSplitter;
    QPushButton *switchFace[4],*demoShow;
    QLabel *usbTargetLabel,*switchTitle[4],*VIDTest,*PIDTest,*ledFace[5],*ledTitle;
    QComboBox *usbTargetCombox;
    QHBoxLayout *layout1,*layout2;
    QVBoxLayout *upSplitter;
    QGridLayout *switchLayout,*ledLayout;
    QIcon switch0Icon, switch1Icon;
    QPixmap  ledONImage,ledOffImage;
    QWidget *widget1,*widget2,*widget3;
    QButtonGroup *switchFaceGroup;
    QTimer *timer;
    hid_device *hDev;
    hid_device_info *hInfo;
    QString panelInfo;
    QLineEdit *VIDNumber,*PIDNumber;
private slots:
    void onSwitchToggle(int index,bool state);
    void refreshLed();
    void SwitchChange(int index, bool value);
    void refreshpanelInfo();
};

#endif // MAINWINDOW_H
