#include <QMessageBox>

#include "communication_dialog.h"


CommunicationDialog::CommunicationDialog(Communication_Settings *_comm_settings, QObject *parent)
{
	setupUi(this);
	
	rbtAdaptive->setEnabled(false);
	rbtCorrectedErrs->setChecked(true);	
	chboxInterleaving->setEnabled(false);	
	
	comm_settings.packet_length = _comm_settings->packet_length;
	comm_settings.block_length = _comm_settings->block_length;
	comm_settings.errs_count = _comm_settings->errs_count;
	comm_settings.packet_delay = _comm_settings->packet_delay;

	comm_settings.antinoise_coding = _comm_settings->antinoise_coding;
	comm_settings.interleaving = _comm_settings->interleaving;
	comm_settings.noise_control = _comm_settings->noise_control;
	comm_settings.packlen_autoadjust = _comm_settings->packlen_autoadjust;

	gboxCoding->setChecked(comm_settings.antinoise_coding);

	sbxErrorsCnt->setValue(comm_settings.errs_count);
	rbtPacksAutosize->setChecked(comm_settings.packlen_autoadjust);
	rbtFixedPackLen->setChecked(!comm_settings.packlen_autoadjust);

	sbxPackLen->setVisible(!comm_settings.packlen_autoadjust);
	lblBytes->setVisible(!comm_settings.packlen_autoadjust);
		
	sbxPackLen->setEnabled(!rbtPacksAutosize->isChecked());
	sbxPackLen->setValue(comm_settings.packet_length);
	sbxBlockLen->setValue(comm_settings.block_length);

	double perc = (100.0*comm_settings.errs_count/(double)comm_settings.block_length);
	QString perc_str = QString::number(perc, 'g', 4);
	lblPerc->setText(QString("( <font color=darkred>%1%</font> )").arg(perc_str));

	sbxPackLen->setSingleStep(comm_settings.block_length);
	if (comm_settings.packet_length % comm_settings.block_length != 0) comm_settings.packet_length = 2*comm_settings.block_length;
	sbxPackLen->setMinimum(comm_settings.block_length);
	sbxPackLen->setMaximum((254/comm_settings.block_length)*comm_settings.block_length);

	sbxPackDelay->setValue(comm_settings.packet_delay);

	setConnections();
}

void CommunicationDialog::setConnections()
{
	connect(gboxCoding, SIGNAL(toggled(bool)), this, SLOT(setAntinoiseCoding(bool)));
	connect(chboxInterleaving, SIGNAL(clicked(bool)), this, SLOT(setInterleaving(bool)));
	connect(rbtAdaptive, SIGNAL(toggled(bool)), this, SLOT(setAdaptiveErrorControl(bool)));
	connect(rbtCorrectedErrs, SIGNAL(toggled(bool)), this, SLOT(setManualErrorControl(bool)));
	connect(rbtPacksAutosize, SIGNAL(clicked(bool)), this, SLOT(setAutoPackLen(bool)));
	connect(rbtFixedPackLen, SIGNAL(clicked(bool)), this, SLOT(setFixedPackLen(bool)));
	connect(sbxErrorsCnt, SIGNAL(valueChanged(int)), this, SLOT(changeCorrectedErrors(int)));
	connect(sbxBlockLen, SIGNAL(valueChanged(int)), this, SLOT(changeBlockLen(int)));
	connect(sbxPackLen, SIGNAL(valueChanged(int)), this, SLOT(changePackLen(int)));
	connect(sbxPackDelay, SIGNAL(valueChanged(int)), this, SLOT(setPacketDelay(int)));
}


void CommunicationDialog::setAntinoiseCoding(bool flag)
{
	comm_settings.antinoise_coding = flag;

	sbxErrorsCnt->setEnabled(flag);
	sbxErrorsCnt->setVisible(flag);
	lblPerc->setVisible(flag);
}

void CommunicationDialog::setInterleaving(bool flag)
{
	comm_settings.interleaving = flag;
}

void CommunicationDialog::setAdaptiveErrorControl(bool flag)
{
	comm_settings.noise_control = flag;
	sbxErrorsCnt->setEnabled(!flag);
	sbxErrorsCnt->setVisible(!flag);
	lblPerc->setVisible(!flag);
}

void CommunicationDialog::setManualErrorControl(bool flag)
{
	comm_settings.noise_control = !flag;
	sbxErrorsCnt->setEnabled(flag);
	sbxErrorsCnt->setVisible(flag);
	lblPerc->setVisible(flag);
}

void CommunicationDialog::setAutoPackLen(bool flag)
{
	comm_settings.packlen_autoadjust = flag;
	sbxPackLen->setEnabled(!flag);
	sbxPackLen->setVisible(!flag);
	lblBytes->setVisible(!flag);
}

void CommunicationDialog::setFixedPackLen(bool flag)
{
	comm_settings.packlen_autoadjust = !flag;
	sbxPackLen->setEnabled(flag);
	sbxPackLen->setVisible(flag);
	lblBytes->setVisible(flag);
}

void CommunicationDialog::changeCorrectedErrors(int val)
{
	comm_settings.errs_count = val;

	double perc = (100.0*comm_settings.errs_count/(double)comm_settings.block_length);
	QString perc_str = QString::number(perc, 'g', 4);
	lblPerc->setText(QString("( <font color=darkred>%1%</font> )").arg(perc_str));	
}

void CommunicationDialog::changePackLen(int val)
{
	if (val % comm_settings.block_length > 0) 
	{
		int ret = QMessageBox::warning(this, "Warning!", tr("Packet length should be a multiple of the block length!"), QMessageBox::Ok);	
		sbxPackLen->setValue(comm_settings.packet_length);
	}
	else comm_settings.packet_length = val;
}

void CommunicationDialog::changeBlockLen(int val)
{
	comm_settings.block_length = val;

	if (comm_settings.errs_count > comm_settings.block_length/4) comm_settings.errs_count = comm_settings.block_length/4;
	changeCorrectedErrors(comm_settings.errs_count);

	sbxErrorsCnt->setMaximum(comm_settings.block_length/4);
	sbxErrorsCnt->setMinimum(1);

	sbxPackLen->setSingleStep(comm_settings.block_length);	
	int blocks = comm_settings.packet_length / comm_settings.block_length;
	comm_settings.packet_length = blocks*comm_settings.block_length;

	sbxPackLen->setMinimum(comm_settings.block_length);
	sbxPackLen->setMaximum((254/comm_settings.block_length)*comm_settings.block_length);
	sbxPackLen->setValue(blocks*comm_settings.block_length);
}

void CommunicationDialog::setPacketDelay(int val)
{
	comm_settings.packet_delay = val;
}