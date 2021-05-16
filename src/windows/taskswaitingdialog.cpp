#include <thread>
#include <iomanip>
#include <thread>

#include <QMessageBox>

#include "taskswaitingdialog.h"
#include "ui_taskswaitingdialog.h"

TasksWaitingDialog::TasksWaitingDialog()
	: m_ui(new Ui::TasksWaitingDialog) {
	m_ui->setupUi(this);
}

TasksWaitingDialog::~TasksWaitingDialog() {

}

void TasksWaitingDialog::do_tasks(std::vector<Task*> task_queue, std::string image_filename) {
	this->task_queue = task_queue;
	this->image_filename = image_filename;
	cur_task = 0;
	tasks_complete = false;

	//Progress update in one thread
	progress_thread = new std::thread(&TasksWaitingDialog::progress_update_per, this);
	//Start tasks in other thread
	tasks_thread = new std::thread(&TasksWaitingDialog::do_tasks_impl, this);
}

void TasksWaitingDialog::progress_update_per() {
	while (!tasks_complete) {
		//Prepare percents
		unsigned char cur_task_percents = (unsigned char)(task_queue[cur_task]->progress() * 100.0F);
		unsigned char all_tasks_percents = (unsigned char)(((float)cur_task + task_queue[cur_task]->progress()) /
			(float)task_queue.size() * 100.0F);
		//Set this percents to progressbars
		m_ui->current_task_progressbar->setValue(cur_task_percents);
		m_ui->all_tasks_progressbar->setValue(all_tasks_percents);

		//Prepare text for current task label
		std::stringstream ss;
		//Task 1/1: Unknown task (100%)
		ss << "Task " << cur_task + 1 << '/' << task_queue.size() << ": " << task_queue[cur_task]->to_string();
		//Add "(xxx%) if not 0%
		if (cur_task_percents != 0)
			ss << " (" << +cur_task_percents << "%)";
		//Set this text
		m_ui->current_task_label->setText(QString::fromStdString(ss.str()));

		//Wait 250 ms
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
	//When completed
	m_ui->current_task_progressbar->setValue(100);
	m_ui->all_tasks_progressbar->setValue(100);
	m_ui->current_task_label->setText(QString("All tasks completed!"));
}

void TasksWaitingDialog::do_tasks_impl() {
	//Load image
	OIIO::ImageBuf input_image(image_filename);
	if (!input_image.read()) {
		QMessageBox::critical(this, "Failed to read image",
							  "Failed to read image. The file may be either damaged, not supported or corrupted");
		return;
	}

	//Do tasks
	for (cur_task = 0; cur_task < task_queue.size(); cur_task++) {
		OIIO::ImageBuf output_image = task_queue[cur_task]->do_task(input_image);
		input_image = output_image;
	}
	tasks_complete = true;
}
