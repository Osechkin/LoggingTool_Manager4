#include "signal_processing_wizard.h"


SignalProcessingWizard::SignalProcessingWizard(QWidget *parent /* = 0 */) : QWidget(parent), ui(new Ui::SignalProcessingWizard)
{
	ui->setupUi(this);
	ui->pbtApply->setIcon(QIcon(":/images/apply.png"));

	
	QStringList proc_list; proc_list << "ADC Data" << "FID";
	ui->comboBox->addItems(proc_list);

	QStringList headlist;
	ui->treeWidget->setColumnCount(3);
	headlist << tr("Parameter") << tr("Value") << tr("Units");
	ui->treeWidget->setHeaderLabels(headlist);
	ui->treeWidget->setColumnWidth(0,120);
	ui->treeWidget->setColumnWidth(1,60);
	ui->treeWidget->setColumnWidth(2,60);
	
	ui->treeWidget->header()->setFont(QFont("Arial", 10, 0, false));	

	showProcParameters(QString("ADC Data"));
	setConnections();
}

SignalProcessingWizard::~SignalProcessingWizard()
{
	delete ui;
}

void SignalProcessingWizard::setConnections()
{
	connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(treeWidgetActivated(QTreeWidgetItem*,int)));
	connect(ui->comboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(showProcParameters(QString)));
	connect(ui->pbtApply, SIGNAL(clicked()), this, SLOT(applyProc()));
}


void SignalProcessingWizard::treeWidgetActivated(QTreeWidgetItem *item, int col)
{
	for (int i = 0; i < c_items.count(); i++)
	{
		QTreeWidgetItem *sub_item = c_items[i]->getQSubTreeWidgetItem();
		if (sub_item == item)
		{
			c_item_selected = c_items[i];
			return;
		}
	}
}

void SignalProcessingWizard::showProcParameters(QString text)
{
	ui->treeWidget->clear();
	//qDeleteAll(c_items);
	c_items.clear();
	
	QList<CSettings> title_settings_list;
	CSettings title_settings("label", tr("Output Data"));
	title_settings.title_flag = true;
	title_settings.text_color = QColor(Qt::darkRed);
	title_settings.background_color = QColor(Qt::blue).lighter(160);
	title_settings_list.append(title_settings);

	CTreeWidgetItem *c_title = new CTreeWidgetItem(ui->treeWidget, 0, title_settings_list);
	c_title->show();
	
	if (text == "ADC Data")
	{
		QStringList titles; titles << tr("ADC Signal #"); 

		QList<CSettings> item_settings_list;

		CSettings item_settings1("label", titles[0]);
		item_settings1.text_color = QColor(Qt::darkBlue);
		item_settings1.background_color = QColor(Qt::white);
		
		CSettings item_settings2("spinbox", QString("%1").arg(1));
		item_settings2.min_max = QPair<double,double>(1,1);
		item_settings2.background_color = QColor(Qt::white);
		item_settings2.text_color = QColor(Qt::darkMagenta);

		CSettings item_settings3("combobox", QString(""));
		item_settings3.background_color = QColor(Qt::white);		
		item_settings3.text_color = QColor(Qt::darkMagenta);	

		item_settings_list << item_settings1 << item_settings2 << item_settings3;
		CTreeWidgetItem *c_item = new CTreeWidgetItem(ui->treeWidget, c_title->getQSubTreeWidgetItem(), item_settings_list);
		c_items.append(c_item);
		c_item->show();
	}
	else if (text == "FID")
	{
		// 2 parameters
		QStringList titles; titles << tr("Noise #") << tr("FID Signal #"); 
		for (int j = 0; j < 2; j++)
		{
			QList<CSettings> item_settings_list;

			CSettings item_settings1("label", titles[j]);
			item_settings1.text_color = QColor(Qt::darkBlue);
			item_settings1.background_color = QColor(Qt::white);
			//item_settings1.background_color = QColor(Qt::blue).lighter(190);

			CSettings item_settings2("spinbox", QString("%1").arg(j+1));
			item_settings2.min_max = QPair<double,double>(j+1,j+1);
			item_settings2.background_color = QColor(Qt::white);
			item_settings2.text_color = QColor(Qt::darkMagenta);

			CSettings item_settings3("combobox", QString(""));
			item_settings3.background_color = QColor(Qt::white);
			//item_settings3.background_color = QColor(Qt::blue).lighter(190);
			item_settings3.text_color = QColor(Qt::darkMagenta);

			item_settings_list << item_settings1 << item_settings2 << item_settings3;
			CTreeWidgetItem *c_item = new CTreeWidgetItem(ui->treeWidget, c_title->getQSubTreeWidgetItem(), item_settings_list);
			c_items.append(c_item);
			c_item->show();
		}

	}
}

void SignalProcessingWizard::applyProc()
{
	cmd_seq.clear();

	QString title = ui->comboBox->currentText();
	if (title == tr("ADC Data"))
	{
		for (int i = 0; i < c_items.count(); i++)
		{
			CTreeWidgetItem *c_item = c_items[i];
			QList<CSettings> c_settings_list = c_item->getCSettings();
			QString title = c_settings_list[0].value.toString();
			float signal_number = c_settings_list[1].value.toFloat();
			if (title == tr("ADC Signal #"))
			{				
				cmd_seq << 0x00		// номер программы "обработки" данных
						<< 0x01		// код команды "скопировать в выходной буффер двухбайтных целых чисел"
						<< 0x01		// количество параметров - 1
						<< 0x13;	// тип данных для копирования - в данном случае оригинальные данные АЦП

				uint32_t *a = (uint32_t*)&signal_number;	// параметр 1 - номер спинового эха 
				cmd_seq.push_back(*a & 0x000000FF);			
				cmd_seq.push_back((*a >> 8) & 0x000000FF);
				cmd_seq.push_back((*a >> 16) & 0x000000FF);
				cmd_seq.push_back((*a >> 24) & 0x000000FF);
				
				cmd_seq.push_back(0xFF);					// конец программы "обработки"
			}
		}
	}
}