/*
 * ImageUpscalerQt tasks waiting dialog
 * Copyright (C) 2021  Artem Kliminskyi <artemklim50@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <thread>
#include <iomanip>
#include <thread>
#include <chrono>

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

#include "TasksWaitingDialog.h"
#include "ui_TasksWaitingDialog.h"

TasksWaitingDialog::TasksWaitingDialog() : m_ui(new Ui::TasksWaitingDialog) {
	m_ui->setupUi(this);

	//Set window icon
	setWindowIcon(QIcon(":Icon.png"));

	timer = new QTimer(this);

	//BEGIN Connect signals
	connect(m_ui->save_button, SIGNAL(clicked()), this, SLOT(save_clicked()));
	connect(m_ui->cancel_button, SIGNAL(clicked()), this, SLOT(cancel_clicked()));
	connect(timer, SIGNAL(timeout()), this, SLOT(progress_check()));
	//END Connect signals
}

TasksWaitingDialog::~TasksWaitingDialog() {

}

void TasksWaitingDialog::do_tasks(std::vector<Task*> task_queue, QString image_filename) {
	worker = new Worker(task_queue, image_filename);
	tasks_complete = false;

	//Start tasks
	elapsed_timer.start(); //Start time
	tasks_thread = new std::thread(
		&Worker::do_tasks,
		worker,
		[this]() { //Success
			tasks_complete = true;
		},
		[this]() { //Cancelled
			cancelled = true;
		},
		[this](QString error) { //Error
			error_message = error;
			error_received = true;
		}
	);
	//Start the progressbar update
	timer->start();
}

///Convert 43200000 ms to 12:00:00
QString format_ms(long long ms_num) {
	std::chrono::milliseconds ms(ms_num);
	auto s = std::chrono::duration_cast<std::chrono::seconds>(ms);
	ms -= std::chrono::duration_cast<std::chrono::milliseconds>(s);
	auto m = std::chrono::duration_cast<std::chrono::minutes>(s);
	s -= std::chrono::duration_cast<std::chrono::seconds>(m);
	auto h = std::chrono::duration_cast<std::chrono::hours>(m);
	m -= std::chrono::duration_cast<std::chrono::minutes>(h);

	return QString("%1:%2:%3").arg(QString::number(h.count()).rightJustified(2, '0'),
								   QString::number(m.count()).rightJustified(2, '0'),
								   QString::number(s.count()).rightJustified(2, '0'));
}

void TasksWaitingDialog::progress_check() {
	//Progressbars
	m_ui->current_task_progressbar->setValue(worker->cur_task_progress() * 100.0F);
	m_ui->all_tasks_progressbar->setValue(worker->overall_progress() * 100.0F);

	//Text for current task label
	m_ui->current_task_label->setText(worker->cur_status());

	//Text for the time label
	m_ui->time_label->setText(format_ms(elapsed_timer.elapsed()));

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
		QMessageBox::critical(this, "Error", error_message);

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
					   "PNG image(*.png);;JPEG image(*.jpg *.jpeg);;JPEG2000 image(*.jp2 *.jpg2);;\
					   Bitmap image(*.bmp);;TIFF image(*.tiff *.tif);;Icon(*.ico)");
	dialog.setFileMode(QFileDialog::FileMode::AnyFile);
	dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
	dialog.setDefaultSuffix(".png");

	//If accepted
	if (dialog.exec() == QDialog::DialogCode::Accepted) {
		//Extract filename
		QString filename = dialog.selectedFiles()[0];
		//Save the image
		worker->save_image(
			filename,
			[this](QString error) {
				QMessageBox::critical(this, "Failed to write image",
					"Failed to write image:\n" + error);
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
