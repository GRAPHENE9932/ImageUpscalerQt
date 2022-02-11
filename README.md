# ImageUpscalerQt
# <p align="center">**The project is under development!**</p>

Contents:
* [How to use](#how-to-use)
* [Build from source](#source)
	* [Arch](#arch-build)
	* [Ubuntu](#ubuntu-build)
	* [Windows](#windows-build)

ImageUpscalerQt is a program for upscaling images using neural networks, but it also has other auxiliary functions.
![Screenshot](https://i.imgur.com/Km588DX.png)
Main functions:

* Resize with various interpolations.
* Use SRCNN (Super Resolution Convolutional Neural Network) of different architectures.
* Use FSRCNN (Fast Super Resolution Convolutional Neural Network) of different architectures.
* Convert color space (RGB to YCbCr, RGB to YCoCg and vice versa).

## How to use <a name="how-to-use"/>
1. Select the image you want to process using the **"Select image"** button in the left-top corner.
2. Every function of this program have its own task type. You can choose the task type using the combo box in the top-right corner.
3. Each task has its own parameters.
4. When all the parameters are specified, click on the **"Add task"** button.
5. Your task has appeared in the queue. You can add other tasks or manage existing using the **"Remove"**, **"Clear"**, **"Up"** and **"Down"** buttons.
6. Click the **"Start tasks"** button in the left-bottom corner.

![Screenshot](https://i.imgur.com/L1Wj66f.png)

7. Wait for tasks to be completed.
8. Save the result using the **"Save result"** button in the right-bottom corner.

# Build from source <a name="source"/>
## For Arch Linux based distributions <a name="arch-build"/>
### Install dependencies
```
$ sudo pacman -S openimageio qt5-base cmake gcc python-pip wget unzip onednn
```
### Configure and compile
```
$ git clone https://github.com/GRAPHENE9932/ImageUpscalerQt.git
$ cd ImageUpscalerQt
$ mkdir build
$ cd build
$ cmake ..
$ make
```
## For Ubuntu 21.04 or newer <a name="ubuntu-build"/>
### Install dependencies
```
$ sudo apt install libopenimageio-dev qt5-default libdnnl-dev cmake gcc git python3-pip wget unzip
```
### Configure and compile
```
$ git clone https://github.com/GRAPHENE9932/ImageUpscalerQt.git
$ cd ImageUpscalerQt
$ mkdir build
$ cd build
$ cmake ..
$ make
```
## For Windows <a name="windows-build"/>

### Install Visual Studio
You need visual studio in order to compile this project.

### Install vcpkg
Vcpkg is needed to install other packages/libraries.
Install instructions are here:
https://vcpkg.io/en/getting-started.html

Don't close the command line, we will need it later.
### Install dependencies
In directory where vcpkg installed, enter this command to install the OpenImageIO library:
```
./vcpkg install openimageio:x64-windows
./vcpkg install onednn:x64-windows
./vcpkg install qt5:x64-windows
```
### Build the program
Go to other directory where you want to build the project.
```
git clone https://github.com/GRAPHENE9932/ImageUpscalerQt.git
cd ImageUpscalerQt-master
cmake -DCMAKE_TOOLCHAIN_FILE=\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake -G "Visual Studio 16 2019" -A x64 -S . -B "build64"
cmake --build build64 --config Release
```
Now you have an .exe file with required .dlls in the \path\to\ImageUpscalerQt-master\build64\Release folder.
