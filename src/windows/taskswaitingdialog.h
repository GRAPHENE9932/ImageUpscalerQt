#ifndef TASKSWAITINGDIALOG_H
#define TASKSWAITINGDIALOG_H

#include <QScopedPointer>
#include <QDialog>
#include <OpenImageIO/imagebuf.h>

#include "../tasks/Task.h"

namespace Ui {
	class TasksWaitingDialog;
}
class TasksWaitingDialog : public QDialog {
	Q_OBJECT

public:
	TasksWaitingDialog();
	~TasksWaitingDialog();

	void do_tasks(std::vector<Task*> task_queue, std::string image_filename);

private:
	QScopedPointer<Ui::TasksWaitingDialog> m_ui;
	std::vector<Task*> task_queue;
	std::string image_filename;

	unsigned short cur_task = 0;
	bool tasks_complete = false;
	bool cancel_requested = false;

	std::thread* progress_thread;
	std::thread* tasks_thread;
	OIIO::ImageBuf finished_image;

	void progress_update_per();
	void do_tasks_impl();
	void cancel_finished();

private slots:
	void cancel_clicked();
	void save_clicked();
};

#endif // TASKSWAITINGDIALOG_H
