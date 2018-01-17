#ifndef EXPERIMENT_SCHEDULER_H
#define EXPERIMENT_SCHEDULER_H

#include <QObject>
#include <QComboBox>
#include <QFileInfo>

#include "../LUSI/LUSI.h"


#define		nan		std::numeric_limits<double>::quiet_NaN()

namespace Scheduler
{
	enum Command { Exec_Cmd, DistanceRange_Cmd, SetPosition_Cmd, Loop_Cmd, Sleep_Cmd, End_Cmd, NoP_Cmd };
	enum WidgetType { DoubleSpinBox, SpinBox, ComboBox, LineEdit, FileBrowse };
		
	struct SettingsItem
	{
		SettingsItem(QString _title, WidgetType _wid_type, QString _units)
		{
			title = _title;
			widget_type = _wid_type;			
			units = _units;
		}

		QString title;		
		QString units;
		WidgetType widget_type;			// тип виджета для ввода параметра команды 		
	};
	typedef QList<SettingsItem*>		SettingsItemList;


	class FileBrowser : public QWidget
	{
		Q_OBJECT
		
	public:
		FileBrowser(QString _file_name, QWidget *parent = 0);

		QString getPath() { return file_path; } 
		QString getFileName() { return file_name; }

	private slots:		
		void enterDataFileName();
	
	private:
		QLineEdit *ledFileName;

		QString file_path;
		QString file_name;
		
	signals:
		void new_filename(const QFileInfo&);
	};


	class SchedulerObject : public QObject
	{
		Q_OBJECT

	public:
		SchedulerObject(Command _type = NoP_Cmd); 
		
		Command type;				
		QString cell_text;
		QString cell_text_template;
		SettingsItemList param_objects;

	signals:
		void changed();
	};

	class Exec : public SchedulerObject
	{
		Q_OBJECT

	public:
		explicit Exec(QStringList _jseqs, QString jseq_file, QString _data_file);
		~Exec();

		QStringList jseq_list;
		QString jseq_name;		
		QString data_file;
		
		QByteVector comm_prg;
		QByteVector instr_prg;

	public slots:
		void changeJSeq(const QString &_jseq) { jseq_name = _jseq; emit changed(); }
		void editFileName(const QFileInfo &_fi) { data_file = _fi.absoluteFilePath(); emit changed(); }
	};

	class DistanceRange : public SchedulerObject
	{
		Q_OBJECT

	public: 
		explicit DistanceRange();
		~DistanceRange();

		void setBounds(QPair<double,double> bounds);
		void setFromToStep(QPair<double,double> from_to, double _step, bool _do_calibr);
		void setCalibrationLength(double _len);
		double getNextPos();
		void init() { pos = nan; finished = false; }
		
	public slots:
		void changeFrom(double val);
		void changeTo(double val);
		void changeStep(double val);
		void changeDoCalibr(const QString& text);

	public:
		double from;
		double to;
		double step;	
		double upper_bound;		// in [cm]
		double lower_bound;		// in [cm]
		double calibr_len;		// in [cm]
		bool do_calibr;
		double pos;
		bool finished;
		bool is_calibration_range;	// находится ли керн в данный момент в области калибровочного образца
	};

	class SetPosition : public SchedulerObject
	{
		Q_OBJECT

	public: 
		explicit SetPosition(double _position = 100);
		~SetPosition();

		void setBounds(QPair<double,double> bounds);
		
	public slots:
		void changePosition(double _pos);
		
	public:
		double position;		// in [cm]	
		double upper_bound;		// in [cm]
		double lower_bound;		// in [cm]
	};

	class Loop : public SchedulerObject
	{
		Q_OBJECT

	public: 
		explicit Loop();
		~Loop();

		void init() { index = 0; finished = false; }

		int getNextIndex()
		{
			if (index < counts) index++;
			else finished = true;
			return index;
		}

		int index;
		int counts;
		int lower_bound;
		bool finished;

	public slots:
		void changeCounts(int val);		
	};

	class Sleep : public SchedulerObject
	{
		Q_OBJECT

	public:
		explicit Sleep();
		~Sleep();

		int delay;
		int upper_bound;

	public slots:
		void changeDelay(int val);
	};

	class End : public SchedulerObject
	{
		Q_OBJECT

	public: 
		explicit End();

		SchedulerObject *ref_obj;
	};

	class NOP : public SchedulerObject
	{
		Q_OBJECT

	public: 
		explicit NOP();
	};
	
	typedef QList<SchedulerObject*>		SchedulerObjList;


	class Engine : public QObject
	{
		Q_OBJECT

	public:
		Engine() { cur_pos = -1; }
		~Engine();
		
		SchedulerObjList getObjectList() { return obj_list; }
		
		template<class X> void add(X* obj)
		{
			SchedulerObject *sch_obj = static_cast<SchedulerObject*>(obj);
			if (sch_obj) obj_list.append(obj);
		}		
		template<class X> void insert(int index, X* obj)
		{
			if (index > obj_list.count()) return;

			SchedulerObject *sch_obj = static_cast<SchedulerObject*>(obj);
			if (sch_obj) obj_list.insert(index, obj);
		}		
		SchedulerObject* get(int index);
		SchedulerObject* next();
		void remove(int index);
		SchedulerObject* take(int index);
		void clear();
		void start() { cur_pos = 0; }
		void stop() { cur_pos = -1; }
		void setPos(int index) { cur_pos = index; }
		void setPos(Scheduler::SchedulerObject* obj);
		bool isEmpty() { return obj_list.isEmpty(); }
		//void stop();
		//void exec();
		
	private:
		SchedulerObjList obj_list;
		int cur_pos;
	};

	
	class CommandController : public QObject
	{
		Q_OBJECT

	public:
		CommandController(int _cmd_uid, Scheduler::Command _type)
		{
			cmd_uid = _cmd_uid;
			repetitions = 0;
			job_finished = false;		
			type = _type;
		}
		~CommandController() { }

		void quitThread() { emit quit_thread(); }
		bool isFinished() { return job_finished; }		

		void setCmdUid( int _uid) { cmd_uid = _uid; }
		int getRepetitions() const { return repetitions; }

		Scheduler::Command getType() const { return type; }
		
	public slots:
		void processResult(bool flag, uint32_t _uid);

	private:
		bool job_finished;	// = true, если работа завершилась		
		uint32_t cmd_uid;
		int repetitions;	
		Scheduler::Command type;

	signals:
		void finished();
		void quit_thread();
	};
	
}



#endif // EXPERIMENT_SCHEDULER_H