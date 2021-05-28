#ifndef TASKSWAITINGDIALOG_H
#define TASKSWAITINGDIALOG_H

#include <QScopedPointer>
#include <QDialog>
#include <QTimer>
#include <OpenImageIO/imagebuf.h>

#include "../tasks/Task.h"
#include "../tasks/Worker.h"

namespace Ui {
	class TasksWaitingDialog;
}
class TasksWaitingDialog : public QDialog {
	Q_OBJECT

public:
	TasksWaitingDialog();
	~TasksWaitingDialog();

	void do_tasks(std::vector<Task*> task_queue, QString image_filename);

private:
	QScopedPointer<Ui::TasksWaitingDialog> m_ui;

	QTimer* timer;

	bool tasks_complete = false;
	bool cancelled = false;
	bool error_received = false;
	QString error_message;

	bool image_saved = false;

	std::thread* progress_thread;
	std::thread* tasks_thread;

	Worker* worker;

	void reject();

private slots:
	void cancel_clicked();
	void save_clicked();
	void progress_check();
};

#endif // TASKSWAITINGDIALOG_H
