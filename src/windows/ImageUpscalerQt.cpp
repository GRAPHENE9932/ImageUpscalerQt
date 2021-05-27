#include <array>
#include <filesystem>
#include <sstream>

#include <QtDebug>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

#include "ImageUpscalerQt.h"
#include "ui_ImageUpscalerQt.h"
#include "../Algorithms.h"
#include "../tasks/TaskResize.h"
#include "../tasks/TaskSRCNN.h"
#include "../tasks/TaskFSRCNN.h"
#include "../tasks/TaskConvertColorSpace.h"
#include "TasksWaitingDialog.h"

#define VERSION "1.0"

ImageUpscalerQt::ImageUpscalerQt(QWidget *parent) : QMainWindow(parent),
    m_ui(new Ui::ImageUpscalerQt) {
    m_ui->setupUi(this);

	//BEGIN Connect signals
	connect(m_ui->add_task_button, SIGNAL(clicked()), this, SLOT(add_task_clicked()));
	connect(m_ui->task_kind_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(task_kind_changed(int)));
	connect(m_ui->select_image_button, SIGNAL(clicked()), this, SLOT(select_image_clicked()));
	connect(m_ui->remove_task_button, SIGNAL(clicked()), this, SLOT(remove_task_clicked()));
	connect(m_ui->clear_tasks_button, SIGNAL(clicked()), this, SLOT(clear_tasks_clicked()));
	connect(m_ui->up_task_button, SIGNAL(clicked()), this, SLOT(move_task_up_clicked()));
	connect(m_ui->down_task_button, SIGNAL(clicked()), this, SLOT(move_task_down_clicked()));
	connect(m_ui->resize_x, SIGNAL(valueChanged(int)), this, SLOT(resize_x_changed(int)));
	connect(m_ui->resize_y, SIGNAL(valueChanged(int)), this, SLOT(resize_y_changed(int)));
	connect(m_ui->keep_image_ratio_radio, SIGNAL(toggled(bool)), this, SLOT(keep_ratio_toggled(bool)));
	connect(m_ui->start_tasks_button, SIGNAL(clicked()), this, SLOT(start_tasks_clicked()));
	connect(m_ui->about_button, SIGNAL(clicked()), this, SLOT(about_clicked()));
	//END Connect signals
}

ImageUpscalerQt::~ImageUpscalerQt() = default;

//BEGIN Slots
void ImageUpscalerQt::add_task_clicked() {
	//Add task
	switch ((TaskKind)m_ui->task_kind_combobox->currentIndex()) {
		case TaskKind::resize: {
			TaskResize* cur_task = new TaskResize(
				m_ui->resize_x->value(),
				m_ui->resize_y->value(),
				(Interpolation)m_ui->resize_interpolation_combobox->currentIndex()
			);

			task_queue.push_back(cur_task);
			break;
		}
		case TaskKind::srcnn: {
			//Get SRCNN name
			std::string name = m_ui->srcnn_architecture_combobox->currentText().toStdString();
			//Parse SRCNN
			std::array<unsigned short, 3> kernels;
			std::array<unsigned short, 3> paddings;
			std::array<unsigned short, 2> channels;
			Algorithms::parse_srcnn(name, &kernels, &paddings, &channels);

			//Create task
			TaskSRCNN* cur_task = new TaskSRCNN(kernels, paddings, channels);
			task_queue.push_back(cur_task);
			break;
		}
		case TaskKind::fsrcnn: {
			//Get FSRCNN name
			std::string name = m_ui->fsrcnn_architecture_combobox->currentText().toStdString();
			//Parse FSRCNN
			std::array<unsigned short, 4> kernels;
			std::array<unsigned short, 4> paddings;
			std::array<unsigned short, 3> channels;
			Algorithms::parse_fsrcnn(name, &kernels, &paddings, &channels);

			//Create task
			TaskFSRCNN* cur_task = new TaskFSRCNN(kernels, paddings, channels);
			task_queue.push_back(cur_task);
			break;
		}
		case TaskKind::convert_color_space: {
			TaskConvertColorSpace* cur_task = new TaskConvertColorSpace(
				(ColorSpaceConversion)m_ui->color_space_combobox->currentIndex()
			);

			task_queue.push_back(cur_task);
		}
	}

	update_list();
}

void ImageUpscalerQt::task_kind_changed(int index) {
	switch ((TaskKind)index) {
		case TaskKind::resize: {
			//Is image selected and valid?
			if (!image_spec.undefined()) {
				//Set image width and height the same
				m_ui->resize_x->setValue(image_spec.width);
				m_ui->resize_y->setValue(image_spec.height);
				//Enable keep ratio radio button
				m_ui->keep_image_ratio_radio->setEnabled(true);
				m_ui->keep_image_ratio_radio->setChecked(true);
			}
			else {
				//Disable keep ratio radio button
				m_ui->keep_image_ratio_radio->setEnabled(false);
				m_ui->keep_image_ratio_radio->setChecked(false);
			}

		}

		case TaskKind::srcnn: {
			//Get folder that contains .pt files
			std::string nn_storage_path = QDir::currentPath().toStdString() + "/SRCNN/";

			//Check if this folder exists
			if (!std::filesystem::exists(nn_storage_path)) {
				QMessageBox::critical(this, "Missing files", "Can\'t find folder with neural network parameters.\nMay be some files or folders was corrupted.");
				m_ui->srcnn_architecture_combobox->clear();
				return;
			}

			//Get all architecture names
			QStringList names;
			std::filesystem::directory_iterator end;
			for (std::filesystem::directory_iterator iter(nn_storage_path); iter != end; iter++) {
				//Check name if it's srcnn architecture
				std::string cur_name = iter->path().stem();
				std::string cur_extension = iter->path().extension();
				if (Algorithms::parse_srcnn(cur_name, nullptr, nullptr, nullptr) && cur_extension == ".pt")
					names.push_back(QString::fromStdString(cur_name));
			}

			//Check if this architecture names exists
			if (names.size() == 0) {
				QMessageBox::critical(this, "Missing files",
									  "Can\'t find files with neural network parameters with correct file name.\nMay be some files was corrupted.");
				m_ui->srcnn_architecture_combobox->clear();
				return;
			}
			//Insert they into combobox
			m_ui->srcnn_architecture_combobox->clear();
			m_ui->srcnn_architecture_combobox->addItems(names);

			break;
		}

		case TaskKind::fsrcnn: {
			//Get folder that contains .pt files
			std::string nn_storage_path = QDir::currentPath().toStdString() + "/FSRCNN/";

			//Check if this folder exists
			if (!std::filesystem::exists(nn_storage_path)) {
				QMessageBox::critical(this, "Missing files",
									  "Can\'t find folder with neural network parameters.\nMay be some files or folders was corrupted.");
				m_ui->fsrcnn_architecture_combobox->clear();
				return;
			}

			//Get all architecture names
			QStringList names;
			std::filesystem::directory_iterator end;
			for (std::filesystem::directory_iterator iter(nn_storage_path); iter != end; iter++) {
				//Check name if it's fsrcnn architecture
				std::string cur_name = iter->path().stem();
				std::string cur_extension = iter->path().extension();
				if (Algorithms::parse_fsrcnn(cur_name, nullptr, nullptr, nullptr) && cur_extension == ".pt")
					names.push_back(QString::fromStdString(cur_name));
			}

			//Check if this architecture names exists
			if (names.size() == 0) {
				QMessageBox::critical(this, "Missing files",
									  "Can\'t find files with neural network parameters with correct file name.\nMay be some files was corrupted.");
				m_ui->fsrcnn_architecture_combobox->clear();
				return;
			}

			//Insert they into combobox
			m_ui->fsrcnn_architecture_combobox->clear();
			m_ui->fsrcnn_architecture_combobox->addItems(names);

			break;
		}

		case TaskKind::convert_color_space: {
			//Just do nothing . _.
		}
	}

	//Toggle stacked widget
	m_ui->task_settings_widget->setCurrentIndex(index);
}

void ImageUpscalerQt::select_image_clicked() {
	QFileDialog dialog(this, "Open image", "/home",
		"All images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.ico);;\
		PNG image (*.png);;JPEG image (*.jpg *.jpeg);;JPEG2000 image (*.jp2 *.jpg2)\
		Bitmap image (*.bmp);;TIFF image (*.tiff *.tif);;Icon (*.ico)");
	dialog.setFileMode(QFileDialog::FileMode::ExistingFile);
	dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);

	//If accepted
	if (dialog.exec() == QDialog::DialogCode::Accepted) {
		this->image_filename = dialog.selectedFiles()[0].toStdString();
		m_ui->image_path_label->setText(dialog.selectedFiles()[0]);

		//Get image spec and check image
		auto input = OIIO::ImageInput::open(image_filename);
		if (!input) {
			QMessageBox::critical(this, "Failed to read image",
								  "Failed to read image. The file may be either damaged, not supported or corrupted");
			return;
		}
		image_spec = input->spec();

		//Update task creation menu
		task_kind_changed(m_ui->task_kind_combobox->currentIndex());
		update_list(); //Update list
	}
}

void ImageUpscalerQt::remove_task_clicked() {
	auto selected = m_ui->queue_list->selectionModel()->selectedIndexes();
	for (unsigned short i = 0; i < selected.size(); i++)
		task_queue.erase(task_queue.begin() + selected[i].row());

	update_list();
}

void ImageUpscalerQt::clear_tasks_clicked() {
	task_queue.clear();
	update_list();
}

void ImageUpscalerQt::move_task_up_clicked() {
	auto selected_list = m_ui->queue_list->selectionModel()->selectedIndexes();
	//Display error message if either selected more than 1 task or no one selected.
	if (selected_list.size() != 1) {
		QMessageBox::information(this, "Unable to move tasks",
								 "Unable to move tasks. You should select one task");
		return;
	}

	//Extract the integer
	int index = selected_list[0].row();

	//Just ignore if task is already on the top
	if (index == 0)
		return;

	//Swap
	std::swap(task_queue[index], task_queue[index - 1]);

	//Update and restore selection
	update_list();
	m_ui->queue_list->setCurrentRow(index - 1);
}

void ImageUpscalerQt::move_task_down_clicked() {
	auto selected_list = m_ui->queue_list->selectionModel()->selectedIndexes();
	//Display error message if either selected more than 1 task or no one selected.
	if (selected_list.size() != 1) {
		QMessageBox::information(this, "Unable to move tasks",
								 "Unable to move tasks. You should select one task");
		return;
	}

	//Extract the integer
	int index = selected_list[0].row();

	//Just ignore if task is already on the bottom
	if (index == task_queue.size() - 1)
		return;

	//Swap
	std::swap(task_queue[index], task_queue[index + 1]);

	//Update and restore selection
	update_list();
	m_ui->queue_list->setCurrentRow(index + 1);
}

void ImageUpscalerQt::resize_x_changed(int value) {
	//If keep ratio enabled
	if (m_ui->keep_image_ratio_radio->isChecked()) {
		//Calculate dependent y
		float ratio = (float)image_spec.height / image_spec.width;
		int y = ratio * value;
		//Set it to the spinbox
		m_ui->resize_y->blockSignals(true);
		m_ui->resize_y->setValue(y);
		m_ui->resize_y->blockSignals(false);
	}
}

void ImageUpscalerQt::resize_y_changed(int value) {
	//If keep ratio enabled
	if (m_ui->keep_image_ratio_radio->isChecked()) {
		//Calculate dependent x
		float ratio = (float)image_spec.width / image_spec.height;
		int x = ratio * value;
		//Set it to the spinbox
		m_ui->resize_x->blockSignals(true);
		m_ui->resize_x->setValue(x);
		m_ui->resize_x->blockSignals(false);
	}
}

void ImageUpscalerQt::keep_ratio_toggled(bool checked) {
	if (checked)
		resize_x_changed(m_ui->resize_x->value());
}
//END Slots

void ImageUpscalerQt::update_list() {
	//Update list
	m_ui->queue_list->clear();
	for (unsigned short i = 0; i < task_queue.size(); i++)
		m_ui->queue_list->addItem(QString::fromStdString(task_queue[i]->to_string(i)));

	//Status label
	if (task_queue.size() == 0) {
		m_ui->status_label->setText("No tasks in queue");
		return;
	}
	if (image_spec.undefined()) {
		m_ui->status_label->setText("Image is not selected");
		return;
	}
	//Calculate "before" and "after" resolution
	int before_width = image_spec.width;
	int before_height = image_spec.height;
	int after_width = before_width;
	int after_height = before_height;
	for (unsigned short i = 0; i < task_queue.size(); i++) {
		switch (task_queue[i]->task_kind) {
			case TaskKind::fsrcnn: {
				after_width += 2;
				after_height += 2;
			}
			case TaskKind::resize: {
				after_width = ((TaskResize*)task_queue[i])->x_size;
				after_height = ((TaskResize*)task_queue[i])->y_size;
			}
			default:
				break;
		}
	}
	std::stringstream ss;
	ss << before_width << 'x' << before_height << " -> " << after_width << 'x' << after_height;
	m_ui->status_label->setText(ss.str().c_str());
}

void ImageUpscalerQt::start_tasks_clicked() {
	//Special cases
	if (image_spec.undefined()) {
		QMessageBox::warning(this, "No image", "No image selected, so you can\'t start the tasks.");
		return;
	}
	if (task_queue.empty()) {
		QMessageBox::warning(this, "No tasks", "Tasks queue is empty, so you can\'t start them.");
		return;
	}

	TasksWaitingDialog* dialog = new TasksWaitingDialog();
	dialog->open();
	dialog->do_tasks(task_queue, image_filename);
}

void ImageUpscalerQt::about_clicked() {
	QMessageBox::about(this, "About the ImageUpscalerQt",
					   QString::fromStdString(std::string("ImageUpscalerQt version ") + VERSION + "\n\nImageUpscalerQt - program for image upscaling using the neural networks, but it also have other auxiliary functions.\n\nMade by Artem Kliminskyi in Ukraine, Zhytomyr"));
}
