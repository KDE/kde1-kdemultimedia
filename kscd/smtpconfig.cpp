#include <klocale.h>
#include <kapp.h>

#include <smtpconfig.h>

SMTPConfig::SMTPConfig(QWidget *parent=0, const char *name=0, struct SMTPConfigData *_configData = 0)
    : QDialog(parent, name)
{
    configData = _configData;
    
    mainBox = new QGroupBox(this, "mainBox");
    mainBox->setGeometry(10,10,520,420);

    enableCB = new QCheckBox(klocale->translate("Enable submission via SMTP"), this, "enableCB");
    enableCB->setGeometry(20, 20, 200, 15);
    enableCB->setChecked(configData->enabled);
    connect(enableCB, SIGNAL(clicked()), this, SLOT(enableClicked()));

    serverHostLabel = new QLabel(this, "serverHostLabel");
    serverHostLabel->setGeometry(20, 40, 130, 25);
    serverHostLabel->setText(klocale->translate("SMTP Address:Port"));
    
    serverHostEdit = new QLineEdit(this, "serverHostEdit");
    serverHostEdit->setGeometry(155, 40, 300, 25);
    serverHostEdit->setText(configData->serverHost.data());
    serverHostEdit->setEnabled(configData->enabled);

    serverPortLabel = new QLabel(this, "serverPortLabel");
    serverPortLabel->setGeometry(460, 40, 10, 25);
    serverPortLabel->setText(" :");
    
    serverPortEdit = new QLineEdit(this, "serverPortEdit");
    serverPortEdit->setGeometry(475, 40, 45, 25);
    serverPortEdit->setText(configData->serverPort.data());
    serverPortEdit->setEnabled(configData->enabled);

    senderAddressLabel = new QLabel(this, "senderAddressLabel");
    senderAddressLabel->setGeometry(20, 70, 130, 25);
    senderAddressLabel->setText(klocale->translate("Your Email Address"));

    senderAddressEdit = new QLineEdit(this, "senderAddressEdit");
    senderAddressEdit->setGeometry(155, 70, 365, 25);
    senderAddressEdit->setText(configData->senderAddress.data());
    senderAddressEdit->setEnabled(configData->enabled);
}

void SMTPConfig::commitData(void)
{
    configData->enabled = enableCB->isChecked();
    configData->serverHost = serverHostEdit->text();
    configData->serverPort = serverPortEdit->text();
    configData->senderAddress = senderAddressEdit->text();
}

void SMTPConfig::enableClicked(void)
{
    bool c;

    c = enableCB->isChecked();
    serverHostEdit->setEnabled(c);
    serverPortEdit->setEnabled(c);
    senderAddressEdit->setEnabled(c);
}
#include <smtpconfig.moc>