#include "comport_dialog.h"

COMPortDialog::COMPortDialog(QString _port, PortSettings _settings, bool _auto_search, QObject *parent)
{
    setupUi(this);
	this->setWindowTitle(tr("Set COM-Port Settings"));

    COM_Settings = _settings;

    QStringList port_names;
    for (int i = 1; i <= 15; i++)
    {
        QString name = QString("COM%1").arg(i);
        QextSerialPort *port = new QextSerialPort(name);
        bool res = port->open(QextSerialPort::ReadWrite);

        if (res)
        {
            port_names.append(name);
            port->close();
        }
        delete port;
    }

    cboxPort->addItems(port_names);
    int index = cboxPort->findText(_port);
    if (index >= 0)
    {
        cboxPort->setCurrentIndex(index);
        port_name = _port;
    }
    else port_name = "";

	auto_search = _auto_search;
	chboxPortAutoSearch->setChecked(auto_search);

    QStringList st_bits;
#if defined(Q_OS_WIN)
    st_bits << "1" << "1.5" << "2";       // FOR WINDOWS
    cboxStopBits->addItems(st_bits);
    if (COM_Settings.StopBits == STOP_1) cboxStopBits->setCurrentIndex(0);
    else if (COM_Settings.StopBits == STOP_1_5) cboxStopBits->setCurrentIndex(1);
    else if (COM_Settings.StopBits == STOP_2) cboxStopBits->setCurrentIndex(2);
#else
    st_bits << "1" << "2";
    cboxStopBits->addItems(st_bits);
    if (COM_Settings.DataBits == STOP_1) cboxStopBits->setCurrentIndex(0);
    else if (COM_Settings.DataBits == STOP_2) cboxStopBits->setCurrentIndex(1);
#endif


    QStringList rates;
#if defined(Q_OS_WIN)                     // FOR WINDOWS
    rates << "600" << "1200" << "2400" << "4800" << "9600" << "14400" << "19200";
    rates << "38400" << "56000" << "57600" << "115200" << "128000" << "256000";
#else
    rates << "600" << "1200" << "2400" << "4800" << "9600" << "19200" << "38400" << "57600" << "115200";
#endif
    cboxRate->addItems(rates);
    int _rate = cboxRate->findText(QString::number(COM_Settings.BaudRate));
    if (_rate >= 0) cboxRate->setCurrentIndex(_rate);


    QStringList bits;
    bits << "5" << "6" << "7" << "8";
    cboxBits->addItems(bits);
    int _bits = cboxBits->findText(QString::number(COM_Settings.DataBits));
    if (_bits >= 0) cboxBits->setCurrentIndex(_bits);


    QStringList parity;
#if defined(Q_OS_WIN)                     // FOR WINDOWS
    parity << "No Parity" << "Odd Parity" << "Even Parity" << "Space Parity" << "Mark Parity";
#else
    parity << "No Parity" << "Odd Parity" << "Even Parity" << "Space Parity";
#endif
    cboxParity->addItems(parity);
    if (COM_Settings.Parity == PAR_NONE) cboxParity->setCurrentIndex(0);
    else if (COM_Settings.Parity == PAR_ODD) cboxParity->setCurrentIndex(1);
    else if (COM_Settings.Parity == PAR_EVEN) cboxParity->setCurrentIndex(2);
    else if (COM_Settings.Parity == PAR_SPACE) cboxParity->setCurrentIndex(3);
#if defined(Q_OS_WIN)
    else if (COM_Settings.Parity == PAR_MARK) cboxParity->setCurrentIndex(4);
#endif

    setConnections();
}

void COMPortDialog::setConnections()
{
    connect(cboxPort, SIGNAL(currentIndexChanged(QString)), this, SLOT(setCOMPort(QString)));
    connect(cboxRate, SIGNAL(currentIndexChanged(QString)), this, SLOT(setBaudrate(QString)));
    connect(cboxBits, SIGNAL(currentIndexChanged(QString)), this, SLOT(setDataBits(QString)));
    connect(cboxStopBits, SIGNAL(currentIndexChanged(QString)), this, SLOT(setStopBits(QString)));
    connect(cboxParity, SIGNAL(currentIndexChanged(QString)), this, SLOT(setParity(QString)));
	connect(chboxPortAutoSearch, SIGNAL(toggled(bool)), this, SLOT(setAutoSearch(bool)));
}

void COMPortDialog::setCOMPort(QString str)
{
    port_name = str;
}

void COMPortDialog::setBaudrate(QString str)
{
    bool ok;
    int val = str.toInt(&ok);
    if (!ok) return;

    COM_Settings.BaudRate = (BaudRateType)val;

    int tt = 0;
}

void COMPortDialog::setDataBits(QString str)
{
    bool ok;
    int val = str.toInt(&ok);
    if (!ok) return;

    COM_Settings.DataBits = (DataBitsType)val;

    int tt = 0;
}

void COMPortDialog::setStopBits(QString str)
{
    if (str == "1") COM_Settings.StopBits = STOP_1;
    else if (str == "1.5") COM_Settings.StopBits = STOP_1_5;
    else if (str == "2") COM_Settings.StopBits = STOP_2;
}

void COMPortDialog::setParity(QString str)
{
    if (str == "No Parity") COM_Settings.Parity = PAR_NONE;
    else if (str == "Odd Parity") COM_Settings.Parity = PAR_ODD;
    else if (str == "Even Parity") COM_Settings.Parity = PAR_EVEN;
    else if (str == "Space Parity") COM_Settings.Parity = PAR_SPACE;
    else if (str == "Mark Parity") COM_Settings.Parity = PAR_MARK;
}

void COMPortDialog::setAutoSearch(bool flag)
{
	auto_search = flag;
}

void COMPortDialog::enableAutoSearch(bool flag)
{
	chboxPortAutoSearch->setEnabled(flag);
}
