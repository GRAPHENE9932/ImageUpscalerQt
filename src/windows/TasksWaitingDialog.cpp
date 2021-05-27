#include <thread>
#include <iomanip>
#include <thread>

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

#include "TasksWaitingDialog.h"
#include "ui_TasksWaitingDialog.h"

TasksWaitingDialog::TasksWaitingDialog() : m_ui(new Ui::TasksWaitingDialog) {
	m_ui->setupUi(this);

	timer = new QTimer(this);

	//BEGIN Connect signals
	connect(m_ui->save_button, SIGNAL(clicked()), this, SLOT(save_clicked()));
	connect(m_ui->cancel_button, SIGNAL(clicked()), this, SLOT(cancel_clicked()));
	connect(timer, SIGNAL(timeout()), this, SLOT(progress_check()));
	//END Connect signals
}

TasksWaitingDialog::~TasksWaitingDialog() {

}

void TasksWaitingDialog::do_tasks(std::vector<Task*> task_queue, std::string image_filename) {
	worker = new Worker(task_queue, image_filename);
	tasks_complete = false;

	//Start tasks
	tasks_thread = new std::thread(
		&Worker::do_tasks,
		worker,
		[this]() { //Success
			tasks_complete = true;
		},
		[this]() { //Cancelled
			cancelled = true;
		},
		[this](std::string error) { //Error
			error_message = error;
			error_received = true;
		}
	);
	//Start the progressbar update
	timer->start();
}

void TasksWaitingDialog::progress_check() {
	//Progressbars
	m_ui->current_task_progressbar->setValue(worker->cur_task_progress() * 100.0F);
	m_ui->all_tasks_progressbar->setValue(worker->overall_progress() * 100.0F);

	//Text for current task label
	m_ui->current_task_label->setText(QString::fromStdString(worker->cur_status()));

	if (tasks_complete) {
		//When completed
		m_ui->current_task_progressbar->setValue(100);
		m_ui->all_tasks_progressbar->setValue(100);
		m_ui->current_task_label->setText("All tasks completed!");
		m_ui->save_button->setEnabled(true); //Enable "Save result" button
		m_ui->cancel_button->setEnabled(false); //Disable "Cancel" button

		timer->stop(); //Stop timer
		return;
	}

	if (cancelled) {
		this->done(2); //Just close this dialog

		timer->stop(); //Stop timer
		return;
	}

	if (error_received) {
		QMessageBox::critical(this, "Error", QString::fromStdString(error_message));

		timer->stop(); //Stop timer
		return;
	}
}

void TasksWaitingDialog::cancel_clicked() {
	worker->cancel();
	m_ui->cancel_button->setEnabled(false); //Disable cancel button
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
		worker->save_image(
			filename,
			[this](std::string error) {
				QMessageBox::critical(this, "Failed to write image",
					QString::fromStdString("Failed to write image:\n" + error));
			}
		);
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
