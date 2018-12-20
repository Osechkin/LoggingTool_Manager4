#include <QRegExpValidator>
#include <QLineEdit>
#include <QSpinBox>

#include "tool_settings_dialog.h"


ToolSettingsSettingsDialog::ToolSettingsSettingsDialog(TCP_Settings &_nmrtool_settings, TCP_Settings &_dmeter_settings, TCP_Settings &_stmotor_settings, QWidget *parent /* = 0 */)
{
	this->setupUi(this);

	nmrtool_settings = _nmrtool_settings;
	dmeter_settings = _dmeter_settings;
	stmotor_settings = _stmotor_settings;

	//ledIP1->setInputMask("000.000.000.000");
	QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";	
	QRegExp ipRegex ("^" + ipRange
		+ "\\." + ipRange
		+ "\\." + ipRange
		+ "\\." + ipRange + "$");

	QRegExpValidator *ipValidator1 = new QRegExpValidator(ipRegex, this);
	QRegExpValidator *ipValidator2 = new QRegExpValidator(ipRegex, this);
	QRegExpValidator *ipValidator3 = new QRegExpValidator(ipRegex, this);
	
	ledIP1->setValidator(ipValidator1);
	ledIP2->setValidator(ipValidator2);
	ledIP3->setValidator(ipValidator3);

	ledIP1->setText(nmrtool_settings.ip_address);
	sbxPort1->setValue(nmrtool_settings.port);
	ledIP2->setText(dmeter_settings.ip_address);
	sbxPort2->setValue(dmeter_settings.port);
	ledIP3->setText(stmotor_settings.ip_address);
	sbxPort3->setValue(stmotor_settings.port);

	setConnections();
}

void ToolSettingsSettingsDialog::setConnections()
{
	connect(ledIP1, SIGNAL(editingFinished()), this, SLOT(inputIP()));
	connect(ledIP2, SIGNAL(editingFinished()), this, SLOT(inputIP()));
	connect(ledIP3, SIGNAL(editingFinished()), this, SLOT(inputIP()));

	connect(sbxPort1, SIGNAL(valueChanged(int)), this, SLOT(inputPort(int)));
	connect(sbxPort2, SIGNAL(valueChanged(int)), this, SLOT(inputPort(int)));
	connect(sbxPort3, SIGNAL(valueChanged(int)), this, SLOT(inputPort(int)));
}

void ToolSettingsSettingsDialog::inputIP()
{
	QLineEdit *led = qobject_cast<QLineEdit*>(sender());
	if (!led) return;

	if (led == ledIP1) nmrtool_settings.ip_address = led->text();
	else if (led == ledIP2) dmeter_settings.ip_address = led->text();
	else if (led == ledIP3) stmotor_settings.ip_address = led->text();
}

void ToolSettingsSettingsDialog::inputPort(int port)
{
	QSpinBox *sbx = qobject_cast<QSpinBox*>(sender());
	if (!sbx) return;

	if (sbx == sbxPort1) nmrtool_settings.port = port;
	else if (sbx == sbxPort2) dmeter_settings.port = port;
	else if (sbx == sbxPort3) stmotor_settings.port = port;
}