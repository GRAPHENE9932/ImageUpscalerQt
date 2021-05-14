#include <array>
#include <filesystem>

#include <QtDebug>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

#include "imageupscalerqt.h"
#include "ui_imageupscalerqt.h"
#include "../Algorithms.h"

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
	//END Connect signals
}

ImageUpscalerQt::~ImageUpscalerQt() = default;

//BEGIN Slots
void ImageUpscalerQt::add_task_clicked() {
	qDebug() << "Add task button clicked";

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
			std::array<unsigned char, 3> kernels;
			std::array<unsigned char, 3> paddings;
			std::array<unsigned char, 2> channels;
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
			std::array<unsigned char, 4> kernels;
			std::array<unsigned char, 4> paddings;
			std::array<unsigned char, 3> channels;
			Algorithms::parse_fsrcnn(name, &kernels, &paddings, &channels);

			//Create task
			TaskFSRCNN* cur_task = new TaskFSRCNN(kernels, paddings, channels);
			task_queue.push_back(cur_task);
			break;
		}
	}

	update_list();
}

void ImageUpscalerQt::task_kind_changed(int index) {
	qDebug() << "Task kind changed!" << index;

	switch ((TaskKind)index) {
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
				QMessageBox::critical(this, "Missing files", "Can\'t find folder with neural network parameters.\nMay be some files or folders was corrupted.");
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

		default:
			break;
	}

	//Toggle stacked widget
	m_ui->task_settings_widget->setCurrentIndex(index);
}

void ImageUpscalerQt::select_image_clicked() {
	qDebug() << "Select image button clicked!";

	QFileDialog dialog(this, "Open image", "/home",
		"All images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.ico);;\
		PNG image (*.png);;JPEG image (*.jpg, *.jpeg);;\
		Bitmap image (*.bmp);;TIFF image (*.tiff, *.tif);;Icon (*.ico)");
	dialog.setFileMode(QFileDialog::FileMode::ExistingFile);
	dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);

	if (dialog.exec() == QDialog::DialogCode::Accepted) {
		this->image_filename = dialog.selectedFiles()[0].toStdString();
		m_ui->image_path_label->setText(dialog.selectedFiles()[0]);
		qDebug() << "Selected image: " << QString::fromStdString(image_filename);
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


//END Slots

void ImageUpscalerQt::update_list() {
	m_ui->queue_list->clear();
	for (unsigned short i = 0; i < task_queue.size(); i++)
		m_ui->queue_list->addItem(QString::fromStdString(task_queue[i]->to_string(i)));
}

