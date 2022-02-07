/*
 * ImageUpscalerQt - task waiting dialog
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <thread>
#include <iomanip>
#include <thread>
#include <chrono>

#include <QMessageBox>
#include <QFileDialog>
#include <QTime>

#include "../functions/func.hpp"
#include "TasksWaitingDialog.hpp"
#include "ui_TasksWaitingDialog.h"

TasksWaitingDialog::TasksWaitingDialog() : m_ui(new Ui::TasksWaitingDialog) {
	m_ui->setupUi(this);

	// Set dialog icon.
	setWindowIcon(QIcon(":icon.png"));

	timer = new QTimer(this);
	timer->setInterval(250);

	// BEGIN Connect signals
	connect(m_ui->cancel_button, SIGNAL(clicked()), this, SLOT(cancel_clicked()));
	connect(timer, SIGNAL(timeout()), this, SLOT(progress_check()));
	// END Connect signals
}

TasksWaitingDialog::~TasksWaitingDialog() {

}

void TasksWaitingDialog::do_tasks(std::vector<std::shared_ptr<TaskDesc>> tasks,
								  std::vector<std::pair<QString, QString>> files) {
	worker = new Worker(tasks, files);
	tasks_complete = false;

	// Start tasks.
	elapsed_timer.start(); // Start time.
	tasks_thread = new std::thread(
		&Worker::do_tasks,
		worker,
		[this]() { // Success.
			tasks_complete = true;
		},
		[this]() { // Cancelled.
			cancelled = true;
		},
		[this](QString error) { // Error.
			error_message = error;
			error_received = true;
		}
	);
	// Start the progressbar update.
	timer->start();
}

void TasksWaitingDialog::progress_check() {
	// Progressbars.
	m_ui->current_task_progressbar->setValue(worker->cur_task_progress() * 100.0f);
	m_ui->overall_progressbar->setValue(worker->overall_progress() * 100.0f);

	// Text for current task label.
	m_ui->current_task_label->setText(worker->cur_status());

	// Text for the time label.
	m_ui->time_label->setText(func::milliseconds_to_string(elapsed_timer.elapsed()));

	if (tasks_complete) {
		// When completed.
		m_ui->current_task_progressbar->setValue(100);
		m_ui->overall_progressbar->setValue(100);
		m_ui->current_task_label->setText("All tasks completed!");
		m_ui->cancel_button->setEnabled(false); // Disable "Cancel" button.

		timer->stop(); // Stop timer.
		return;
	}

	if (cancelled) {
		this->done(2); // Just close this dialog.

		timer->stop(); // Stop timer.
		return;
	}

	if (error_received) {
		QMessageBox::critical(this, "Error", error_message);

		timer->stop(); // Stop timer.
		return;
	}
}

void TasksWaitingDialog::cancel_clicked() {
	worker->cancel();
	m_ui->cancel_button->setEnabled(false); // Disable cancel button.
}

// On close using the X button.
void TasksWaitingDialog::reject() {
	if (!tasks_complete) {
		if (QMessageBox::question(this, "Cancel?", "Tasks are not already finished. Do you want to cancel this tasks?",
			QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
			cancel_clicked();
			return;
		}
	}
	QDialog::reject();
}
