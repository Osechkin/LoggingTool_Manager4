#include "io_general.h"

#include "export_toFile_dialog.h"


ToOilExportDialog::ToOilExportDialog(DataTypeList &_dt_list, QWidget *parent)
{
	this->setupUi(this);
	this->setParent(parent);

	lblTitle->setText("<font color=darkblue>Select Data Types to export to Oil software</font>");
	lblTitle->setFont(QFont("Arial", 10, 0, false));

	pbtSaveSettings->setIcon(QIcon(":/images/save.png"));

	treeWidget->setColumnCount(1);
	treeWidget->header()->close();

	setConnections();

	dt_list = _dt_list;
	saving = false;

	showParameters();
}

ToOilExportDialog::~ToOilExportDialog()
{
	clearCTreeWidget();
}

void ToOilExportDialog::setLblTitle(QString str)
{
	lblTitle->setText(QString("<font color=darkblue>%1</font>").arg(str));
}

void ToOilExportDialog::setWinTitle(QString str)
{
	this->setWindowTitle(str);
}


void ToOilExportDialog::setConnections()
{
	connect(pbtSaveSettings, SIGNAL(clicked()), this, SLOT(saveSettings()));
}

void ToOilExportDialog::showParameters()
{
	clearCTreeWidget();
	treeWidget->clear();

	// Создание раздела "Original ADC data"
	QList<CSettings> org_ADC_data_list;
	CSettings title_org_ADC_data("label", tr("Original ADC data"));
	title_org_ADC_data.title_flag = true;
	title_org_ADC_data.text_color = QColor(Qt::darkRed);
	title_org_ADC_data.background_color = QColor(Qt::blue).lighter(170);
	org_ADC_data_list.append(title_org_ADC_data);

	CTreeWidgetItem *c_title_org_adc_data = new CTreeWidgetItem(treeWidget, 0, org_ADC_data_list);
	c_title_items.append(c_title_org_adc_data);
	c_title_org_adc_data->show();


	// Создание раздела "Preprocessed ADC data"
	QList<CSettings> ADC_data_list;
	CSettings title_ADC_data("label", tr("Preprocessed ADC data"));
	title_ADC_data.title_flag = true;
	title_ADC_data.text_color = QColor(Qt::darkRed);
	title_ADC_data.background_color = QColor(Qt::blue).lighter(170);
	ADC_data_list.append(title_ADC_data);

	CTreeWidgetItem *c_title_adc_data = new CTreeWidgetItem(treeWidget, 0, ADC_data_list);
	c_title_items.append(c_title_adc_data);
	c_title_adc_data->show();


	// Создание раздела "Quad.-detected ADC data"
	QList<CSettings> quad_ADC_data_list;
	CSettings title_quad_ADC_data("label", tr("Quad.-detected ADC data"));
	title_quad_ADC_data.title_flag = true;
	title_quad_ADC_data.text_color = QColor(Qt::darkRed);
	title_quad_ADC_data.background_color = QColor(Qt::blue).lighter(170);
	quad_ADC_data_list.append(title_quad_ADC_data);

	CTreeWidgetItem *c_title_quad_ADC_data = new CTreeWidgetItem(treeWidget, 0, quad_ADC_data_list);
	c_title_items.append(c_title_quad_ADC_data);
	c_title_quad_ADC_data->show();


	// Создание раздела "Frequency spectral data"
	QList<CSettings> fft_data_list;
	CSettings title_fft_data("label", tr("Frequency spectral data"));
	title_fft_data.title_flag = true;
	title_fft_data.text_color = QColor(Qt::darkRed);
	title_fft_data.background_color = QColor(Qt::blue).lighter(170);
	fft_data_list.append(title_fft_data);

	CTreeWidgetItem *c_title_fft_data = new CTreeWidgetItem(treeWidget, 0, fft_data_list);
	c_title_items.append(c_title_fft_data);
	c_title_fft_data->show();


	// Создание раздела "Power frequency spectra"
	QList<CSettings> power_data_list;
	CSettings title_power_data("label", tr("Frequency spectral data"));
	title_power_data.title_flag = true;
	title_power_data.text_color = QColor(Qt::darkRed);
	title_power_data.background_color = QColor(Qt::blue).lighter(170);
	power_data_list.append(title_power_data);

	CTreeWidgetItem *c_title_power_data = new CTreeWidgetItem(treeWidget, 0, power_data_list);
	c_title_items.append(c_title_power_data);
	c_title_power_data->show();


	// Создание раздела "Transverse relaxation data"
	QList<CSettings> relax_data_list;
	CSettings title_relax_data("label", tr("Transverse relaxation data"));
	title_relax_data.title_flag = true;
	title_relax_data.text_color = QColor(Qt::darkRed);
	title_relax_data.background_color = QColor(Qt::blue).lighter(170);
	relax_data_list.append(title_relax_data);

	CTreeWidgetItem *c_title_relax_data = new CTreeWidgetItem(treeWidget, 0, relax_data_list);
	c_title_items.append(c_title_relax_data);
	c_title_relax_data->show();


	// Создание раздела "Gamma logging data"
	QList<CSettings> gamma_data_list;
	CSettings title_gamma_data("label", tr("Gamma logging data"));
	title_gamma_data.title_flag = true;
	title_gamma_data.text_color = QColor(Qt::darkRed);
	title_gamma_data.background_color = QColor(Qt::blue).lighter(170);
	gamma_data_list.append(title_gamma_data);

	CTreeWidgetItem *c_title_gamma_data = new CTreeWidgetItem(treeWidget, 0, gamma_data_list);
	c_title_items.append(c_title_gamma_data);
	c_title_gamma_data->show();


	// Создание раздела "SDSP logging"
	QList<CSettings> sdsp_data_list;
	CSettings title_sdsp_data("label", tr("SDSP logging data"));
	title_sdsp_data.title_flag = true;
	title_sdsp_data.text_color = QColor(Qt::darkRed);
	title_sdsp_data.background_color = QColor(Qt::blue).lighter(170);
	sdsp_data_list.append(title_sdsp_data);

	CTreeWidgetItem *c_title_sdsp_data = new CTreeWidgetItem(treeWidget, 0, sdsp_data_list);
	c_title_items.append(c_title_sdsp_data);
	c_title_sdsp_data->show();


	// Создание раздела "NMR Tool monitoring"
	QList<CSettings> nmr_monitoring_list;
	CSettings title_nmr_monitoring("label", tr("NMR Tool monitoring"));
	title_nmr_monitoring.title_flag = true;
	title_nmr_monitoring.text_color = QColor(Qt::darkRed);
	title_nmr_monitoring.background_color = QColor(Qt::blue).lighter(170);
	nmr_monitoring_list.append(title_nmr_monitoring);

	CTreeWidgetItem *c_title_nmr_monitoring = new CTreeWidgetItem(treeWidget, 0, nmr_monitoring_list);
	c_title_items.append(c_title_nmr_monitoring);
	c_title_nmr_monitoring->show();


	// Создание раздела "Logging Tool monitoring"
	QList<CSettings> logging_monitoring_list;
	CSettings title_logging_monitoring("label", tr("Logging Tool monitoring"));
	title_logging_monitoring.title_flag = true;
	title_logging_monitoring.text_color = QColor(Qt::darkRed);
	title_logging_monitoring.background_color = QColor(Qt::blue).lighter(170);
	logging_monitoring_list.append(title_logging_monitoring);

	CTreeWidgetItem *c_title_logging_monitoring = new CTreeWidgetItem(treeWidget, 0, logging_monitoring_list);
	c_title_items.append(c_title_logging_monitoring);
	c_title_logging_monitoring->show();

	bool sgn_relax_used = false;
	bool aver_relax_used = false;
	bool afr_rx_used = false;
	bool afr_tx_used = false;
	
	// создание отдельных пунктов внутри разделов
	int param_count = dt_list.count();
	for (int i = 0; i < param_count; i++)
	{
		QList<CSettings> item_org_adc_list;
		QList<CSettings> item_preproc_adc_list;
		QList<CSettings> item_quad_data_list;
		QList<CSettings> item_fft_spectra_list;
		QList<CSettings> item_power_spectra_list;
		QList<CSettings> item_relax_list;
		QList<CSettings> item_gamma_list;
		QList<CSettings> item_sdsp_list;
		QList<CSettings> item_nmr_monitoring_list;
		QList<CSettings> item_logging_monitoring_list;			
				
		uint8_t data_type = dt_list[i].type;
		switch (data_type)
		{
		case DT_NS_FID_ORG:
		case DT_NS_SE_ORG:
		case DT_SGN_FID_ORG:
		case DT_SGN_SE_ORG:
			{				
				CSettings item_settings1("checkbox", dt_list[i].comment);
				item_settings1.text_color = QColor(Qt::darkBlue);	
				item_settings1.checkable = true;
				item_settings1.check_state = dt_list[i].flag;
				item_settings1.background_color = QColor(Qt::blue).lighter(190);	

				item_org_adc_list << item_settings1;
				CTreeWidgetItem *c_item = new CTreeWidgetItem(treeWidget, c_title_org_adc_data->getQSubTreeWidgetItem(), item_org_adc_list);
				connect(c_item, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
				c_item->linkObject(&dt_list[i]);
				c_items.append(c_item);
				c_item->show();
				break;
			}
		case DT_NS_FID:	
		case DT_NS_SE:
		case DT_SGN_FID:
		case DT_SGN_SE:
			{				
				CSettings item_settings1("checkbox", dt_list[i].comment);
				item_settings1.text_color = QColor(Qt::darkBlue);	
				item_settings1.checkable = true;
				item_settings1.check_state = dt_list[i].flag;
				item_settings1.background_color = QColor(Qt::blue).lighter(190);	

				item_preproc_adc_list << item_settings1;
				CTreeWidgetItem *c_item = new CTreeWidgetItem(treeWidget, c_title_adc_data->getQSubTreeWidgetItem(), item_preproc_adc_list);
				connect(c_item, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
				c_item->linkObject(&dt_list[i]);
				c_items.append(c_item);
				c_item->show();
				break;
			}
		case DT_NS_QUAD_FID_RE:	
		case DT_NS_QUAD_FID_IM:
		case DT_NS_QUAD_SE_RE:
		case DT_NS_QUAD_SE_IM:
		case DT_SGN_QUAD_FID_RE:
		case DT_SGN_QUAD_FID_IM:
		case DT_SGN_QUAD_SE_RE:
		case DT_SGN_QUAD_SE_IM:
		case DT_NS_QUAD_FID:
		case DT_NS_QUAD_SE:
		case DT_SGN_QUAD_FID:
		case DT_SGN_QUAD_SE:
			{
				CSettings item_settings1("checkbox", dt_list[i].comment);
				item_settings1.text_color = QColor(Qt::darkBlue);	
				item_settings1.checkable = true;
				item_settings1.check_state = dt_list[i].flag;
				item_settings1.background_color = QColor(Qt::blue).lighter(190);	

				item_quad_data_list << item_settings1;
				CTreeWidgetItem *c_item = new CTreeWidgetItem(treeWidget, c_title_quad_ADC_data->getQSubTreeWidgetItem(), item_quad_data_list);
				connect(c_item, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
				c_item->linkObject(&dt_list[i]);
				c_items.append(c_item);
				c_item->show();
				break;
			}
		case DT_NS_FFT_FID_RE:	
		case DT_NS_FFT_SE_RE:
		case DT_SGN_FFT_FID_RE:
		case DT_SGN_FFT_SE_RE:
		case DT_NS_FFT_FID_IM:
		case DT_NS_FFT_SE_IM:
		case DT_SGN_FFT_FID_IM:
		case DT_SGN_FFT_SE_IM:
		case DT_NS_FFT_FID_AM:
		case DT_NS_FFT_SE_AM:
		case DT_SGN_FFT_FID_AM:
		case DT_SGN_FFT_SE_AM:		
			{
				CSettings item_settings1("checkbox", dt_list[i].comment);
				item_settings1.text_color = QColor(Qt::darkBlue);	
				item_settings1.checkable = true;
				item_settings1.check_state = dt_list[i].flag;
				item_settings1.background_color = QColor(Qt::blue).lighter(190);	

				item_fft_spectra_list << item_settings1;
				CTreeWidgetItem *c_item = new CTreeWidgetItem(treeWidget, c_title_fft_data->getQSubTreeWidgetItem(), item_fft_spectra_list);
				connect(c_item, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
				c_item->linkObject(&dt_list[i]);
				c_items.append(c_item);
				c_item->show();
				break;
			}
		case DT_SGN_POWER_SE:
		case DT_SGN_POWER_FID:
		case DT_NS_POWER_SE:
		case DT_NS_POWER_FID:	
			{
				CSettings item_settings1("checkbox", dt_list[i].comment);
				item_settings1.text_color = QColor(Qt::darkBlue);	
				item_settings1.checkable = true;
				item_settings1.check_state = dt_list[i].flag;
				item_settings1.background_color = QColor(Qt::blue).lighter(190);	

				item_power_spectra_list << item_settings1;
				CTreeWidgetItem *c_item = new CTreeWidgetItem(treeWidget, c_title_power_data->getQSubTreeWidgetItem(), item_power_spectra_list);
				connect(c_item, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
				c_item->linkObject(&dt_list[i]);
				c_items.append(c_item);
				c_item->show();
				break;
			}
		case DT_SGN_RELAX:	
		case DT_SGN_RELAX2:
		case DT_SGN_RELAX3:
			{
				if (!sgn_relax_used)
				{
					QString comment = dt_list[i].comment.split("#").first().trimmed();
					CSettings item_settings1("checkbox", comment);
					item_settings1.text_color = QColor(Qt::darkBlue);	
					item_settings1.checkable = true;
					item_settings1.check_state = dt_list[i].flag;
					item_settings1.background_color = QColor(Qt::blue).lighter(190);	

					item_relax_list << item_settings1;
					CTreeWidgetItem *c_item = new CTreeWidgetItem(treeWidget, c_title_relax_data->getQSubTreeWidgetItem(), item_relax_list);
					connect(c_item, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
					c_item->linkObject(&dt_list[i]);
					c_items.append(c_item);
					c_item->show();

					sgn_relax_used = true;
				}				
				break;
			}
		case DT_AVER_RELAX:
		case DT_AVER_RELAX2:
		case DT_AVER_RELAX3:
			{
				if (!aver_relax_used)
				{
					QString comment = dt_list[i].comment.split("#").first().trimmed();
					CSettings item_settings1("checkbox", comment);
					item_settings1.text_color = QColor(Qt::darkBlue);	
					item_settings1.checkable = true;
					item_settings1.check_state = dt_list[i].flag;
					item_settings1.background_color = QColor(Qt::blue).lighter(190);

					item_relax_list << item_settings1;
					CTreeWidgetItem *c_item = new CTreeWidgetItem(treeWidget, c_title_relax_data->getQSubTreeWidgetItem(), item_relax_list);
					connect(c_item, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
					c_item->linkObject(&dt_list[i]);
					c_items.append(c_item);
					c_item->show();

					aver_relax_used = true;
				}				
				break;
			}
		case DT_GAMMA: 
			{
				CSettings item_settings1("checkbox", dt_list[i].comment);
				item_settings1.text_color = QColor(Qt::darkBlue);	
				item_settings1.checkable = true;
				item_settings1.check_state = dt_list[i].flag;
				item_settings1.background_color = QColor(Qt::blue).lighter(190);

				item_gamma_list << item_settings1;
				CTreeWidgetItem *c_item = new CTreeWidgetItem(treeWidget, c_title_gamma_data->getQSubTreeWidgetItem(), item_gamma_list);
				connect(c_item, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
				c_item->linkObject(&dt_list[i]);
				c_items.append(c_item);
				c_item->show();
				break;
			}
		case DT_DIEL: 
			{
				CSettings item_settings1("checkbox", dt_list[i].comment);
				item_settings1.text_color = QColor(Qt::darkBlue);	
				item_settings1.checkable = true;
				item_settings1.check_state = dt_list[i].flag;
				item_settings1.background_color = QColor(Qt::blue).lighter(190);

				item_sdsp_list << item_settings1;
				CTreeWidgetItem *c_item = new CTreeWidgetItem(treeWidget, c_title_sdsp_data->getQSubTreeWidgetItem(), item_sdsp_list);
				connect(c_item, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
				c_item->linkObject(&dt_list[i]);
				c_items.append(c_item);
				c_item->show();
				break;
			}
		case DT_AFR1_RX: 
		case DT_AFR2_RX: 
		case DT_AFR3_RX:
			{
				if (!afr_rx_used)
				{
					QString comment = dt_list[i].comment.split("#").first().trimmed();
					CSettings item_settings1("checkbox", comment);
					item_settings1.text_color = QColor(Qt::darkBlue);	
					item_settings1.checkable = true;
					item_settings1.check_state = dt_list[i].flag;
					item_settings1.background_color = QColor(Qt::blue).lighter(190);

					item_nmr_monitoring_list << item_settings1;
					CTreeWidgetItem *c_item = new CTreeWidgetItem(treeWidget, c_title_nmr_monitoring->getQSubTreeWidgetItem(), item_nmr_monitoring_list);
					connect(c_item, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
					c_item->linkObject(&dt_list[i]);
					c_items.append(c_item);
					c_item->show();

					afr_rx_used = true;
				}
				break;
			}
		case DT_AFR1_TX:
		case DT_AFR2_TX:
		case DT_AFR3_TX:
			{
				if (!afr_tx_used)
				{
					QString comment = dt_list[i].comment.split("#").first().trimmed();
					CSettings item_settings1("checkbox", comment);
					item_settings1.text_color = QColor(Qt::darkBlue);	
					item_settings1.checkable = true;
					item_settings1.check_state = dt_list[i].flag;
					item_settings1.background_color = QColor(Qt::blue).lighter(190);

					item_nmr_monitoring_list << item_settings1;
					CTreeWidgetItem *c_item = new CTreeWidgetItem(treeWidget, c_title_nmr_monitoring->getQSubTreeWidgetItem(), item_nmr_monitoring_list);
					connect(c_item, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
					c_item->linkObject(&dt_list[i]);
					c_items.append(c_item);
					c_item->show();

					afr_tx_used = true;
				}				
				break;
			}
		case DT_DU_T: 
		case DT_PU_T:
		case DT_TU_T: 
			{
				CSettings item_settings1("checkbox", dt_list[i].comment);
				item_settings1.text_color = QColor(Qt::darkBlue);	
				item_settings1.checkable = true;
				item_settings1.check_state = dt_list[i].flag;
				item_settings1.background_color = QColor(Qt::blue).lighter(190);

				item_logging_monitoring_list << item_settings1;
				CTreeWidgetItem *c_item = new CTreeWidgetItem(treeWidget, c_title_logging_monitoring->getQSubTreeWidgetItem(), item_logging_monitoring_list);
				connect(c_item, SIGNAL(value_changed(QObject*, QVariant&)), this, SLOT(paramValueChanged(QObject*, QVariant&)));
				c_item->linkObject(&dt_list[i]);
				c_items.append(c_item);
				c_item->show();
				break;
			}
		default: break;	
		}		
	}
}

void ToOilExportDialog::clearCTreeWidget()
{
	if (!c_items.isEmpty()) qDeleteAll(c_items);
	c_items.clear();

	if (!c_title_items.isEmpty()) qDeleteAll(c_title_items);
	c_title_items.clear();
}

void ToOilExportDialog::paramValueChanged(QObject *obj, QVariant &value)
{
	CTreeWidgetItem *ctwi = qobject_cast<CTreeWidgetItem*>(sender());
	if (ctwi)
	{
		CCheckBox *chbox = qobject_cast<CCheckBox*>(obj);
		if (chbox)
		{	
			bool bool_value = value.toBool();
			DataType *dt = (DataType*)(ctwi->getLinkedObject());
			uint8_t dt_code = dt->type;
			switch (dt_code)
			{
			case DT_SGN_RELAX:
			case DT_SGN_RELAX2:
			case DT_SGN_RELAX3:
				{
					for (int i = 0; i < dt_list.count(); i++)
					{
						//DataType data_type = dt_list.at(i);
						if (dt_list.at(i).type == DT_SGN_RELAX || dt_list.at(i).type == DT_SGN_RELAX2 || dt_list.at(i).type == DT_SGN_RELAX3) 
						{
							dt_list[i].flag = bool_value;
						}
					}
					break;
				}
			case DT_AVER_RELAX:
			case DT_AVER_RELAX2:
			case DT_AVER_RELAX3:
				{
					for (int i = 0; i < dt_list.count(); i++)
					{
						//DataType data_type = dt_list.at(i);
						if (dt_list.at(i).type == DT_AVER_RELAX || dt_list.at(i).type == DT_AVER_RELAX2 || dt_list.at(i).type == DT_AVER_RELAX3) 
						{
							dt_list[i].flag = bool_value;
						}
					}
					break;
				}
			case DT_AFR1_RX:
			case DT_AFR2_RX:
			case DT_AFR3_RX:
				{
					for (int i = 0; i < dt_list.count(); i++)
					{
						//DataType data_type = dt_list.at(i);
						if (dt_list.at(i).type == DT_AFR1_RX || dt_list.at(i).type == DT_AFR2_RX || dt_list.at(i).type == DT_AFR3_RX) 
						{
							dt_list[i].flag = bool_value;
						}
					}
					break;
				}
			case DT_AFR1_TX:
			case DT_AFR2_TX:
			case DT_AFR3_TX:
				{
					for (int i = 0; i < dt_list.count(); i++)
					{
						//DataType data_type = dt_list.at(i);
						if (dt_list.at(i).type == DT_AFR1_TX || dt_list.at(i).type == DT_AFR2_TX || dt_list.at(i).type == DT_AFR3_TX) 
						{
							dt_list[i].flag = bool_value;
						}
					}
					break;
				}
			}
			dt->flag = bool_value;
		}		
	}
}

void ToOilExportDialog::saveSettings()
{
	saving = true;
}
