#ifndef SMTPCONFIG_H
#define SMTPCONFIG_H

#include <qdialog.h>

#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlabel.h>

class SMTPConfig:public QDialog
{
    Q_OBJECT
public:
    struct SMTPConfigData{
        bool enabled;
        QString serverHost;
        QString serverPort;
        QString senderAddress;
    };

    SMTPConfig(QWidget *parent = NULL, const char *name = NULL, struct SMTPConfigData *_configData = NULL);
    ~SMTPConfig() {};

public slots:
    void commitData();
    
signals:

protected:
    QGroupBox       *mainBox;
    QCheckBox       *enableCB;
    QLabel          *serverHostLabel;
    QLineEdit       *serverHostEdit;
    QLabel          *serverPortLabel;
    QLineEdit       *serverPortEdit;
    QLabel          *senderAddressLabel;
    QLineEdit       *senderAddressEdit;

private:
    SMTPConfigData *configData;
};
#endif