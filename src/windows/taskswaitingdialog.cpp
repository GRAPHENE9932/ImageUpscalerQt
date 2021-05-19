#include <thread>
#include <iomanip>
#include <thread>

#include <QMessageBox>
#include <QFileDialog>

#include "taskswaitingdialog.h"
#include "ui_taskswaitingdialog.h"

TasksWaitingDialog::TasksWaitingDialog()
	: m_ui(new Ui::TasksWaitingDialog) {
	m_ui->setupUi(this);

	//BEGIN Connect signals
	connect(m_ui->save_button, SIGNAL(clicked()), this, SLOT(save_clicked()));
	connect(m_ui->cancel_button, SIGNAL(clicked()), this, SLOT(cancel_clicked()));
	//END Connect signals
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
	//Reading image
	m_ui->current_task_label->setText("Reading image...");
	m_ui->current_task_progressbar->setValue(0);
	m_ui->all_tasks_progressbar->setValue(0);

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

		//Wait 100 ms
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	//When completed
	m_ui->current_task_progressbar->setValue(100);
	m_ui->all_tasks_progressbar->setValue(100);
	m_ui->current_task_label->setText(QString("All tasks completed!"));
	m_ui->save_button->setEnabled(true); //Enable "Save result" button
	m_ui->cancel_button->setEnabled(false); //Disable "Cancel" button
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
		OIIO::ImageBuf output_image;
		try {
			output_image = task_queue[cur_task]->do_task(input_image);
		}
		catch (const char* str) {
			if (strcmp(str, "canc") == 0) {
				cancel_finished();
				return;
			}
			else {
				QMessageBox::critical(this, "Unknown error", str);
				return;
			}
		}
		catch (std::exception e) {
			QMessageBox::critical(this, "Unknown error", e.what());
			return;
		}
		catch (...) {
			QMessageBox::critical(this, "Unknown error", "Unknown error occured");
			return;
		}
		input_image = output_image;

		if (cancel_requested) {
			cancel_finished();
			return;
		}
	}
	tasks_complete = true;
	finished_image = input_image;
}

void TasksWaitingDialog::cancel_clicked() {
	cancel_requested = true;
	task_queue[cur_task]->cancel_requested = true;
	m_ui->cancel_button->setEnabled(false); //Disable cancel button
}

void TasksWaitingDialog::cancel_finished() {
	this->done(2); //Just close this dialog
}

void TasksWaitingDialog::save_clicked() {
	//Create the file dialog
	QFileDialog dialog(this, "Save image", "/home",
					   "PNG image (*.png);;JPEG image (*.jpg *.jpeg);;JPEG2000 image (*.jp2 *.jpg2);;\
					   Bitmap image (*.bmp);;TIFF image (*.tiff *.tif);;Icon (*.ico)");
	dialog.setFileMode(QFileDialog::FileMode::AnyFile);
	dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);

	//If accepted
	if (dialog.exec() == QDialog::DialogCode::Accepted) {
		//Extract filename
		std::string filename = dialog.selectedFiles()[0].toStdString();
		//Save the image
		if (!finished_image.write(filename))
			QMessageBox::critical(this, "Failed to write image", "Failed to write image");
		//Close this dialog
		this->done(0);
	}
}

//On close using X
void TasksWaitingDialog::reject() {
	if (!tasks_complete) {
		if (QMessageBox::question(this, "Cancel?", "Tasks are not already finished. Do you want to cancel this tasks?",
							  QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
			cancel_clicked();
		}
	}

	if (!image_saved && tasks_complete) {
		if (QMessageBox::question(this, "Save?", "You have not saved the result, close anyway?",
			QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
			QDialog::reject();
		}
	}
}
