# ImageUpscalerQt

ImageUpscalerQt is a program for upscaling images using neural networks, but it also has other auxiliary functions.
![Screenshot](https://i.imgur.com/Km588DX.png)
Main functions:

* Resize using bilinear, cubic, gaussian, sinc, box, triangle, lanczos3, Catmull-Rom, b-spline and Mitchell interpolation.
* Use SRCNN (Super resolution convolutional neural network) of different architectures.
* Use FSRCNN (Fast super resolution convolutional neural network) of different architectures.
* Convert color space (RGB to YCbCr, YCbCr to RGB, RGB to YCoCg, YCoCg to RGB).

## How to use
1. Select the image you want to process using the **"Select image"** button in left-top corner.
2. Every function of this program have own task type. You can choose the task type using the combo box in the top-right corner.
3. Each task has its own parameters.
4. When all the parameters are specified, click on the **"Add task"** button.
5. Your task has appeared in the queue! You can add other tasks or manage existing using the **"Remove"**, **"Clear"**, **"Up"** and **"Down"** buttons.
6. Click the **"Start tasks"** button in the left-bottom corner.

![Screenshot](https://i.imgur.com/L1Wj66f.png)

7. Wait for tasks to be completed.
8. Save result using the **"Save result"** button in the right-bottom corner.

# Build from source
## Linux
### Install OpenImageIO
For Debian-based distros: `sudo apt-get install libopenimageio2.2`

For Arch-based distros: `sudo pacman -S openimageio`
### Install Qt5
For Debian-based distros: `sudo apt-get install qt5-default`

For Arch-based destros: `sudo pacman -S qt5`
### Install LibTorch
There is two ways to install the LibTorch library: build from source and download from LibTorchs site.
#### Download from the site
* [PyTorch website](https://pytorch.org/get-started/locally/). Select Stable version -> Linux -> LibTorch -> C++/Java -> CPU and download version with cxx11 ABI.
* Extract the downloaded .zip archive to the folder you want.
#### Build from source
Open terminal in folder where you want your libtorch and enter these commands:
**WARNING: these commands may take several hours** 
```
git clone https://github.com/pytorch/pytorch.git
cd pytorch
git checkout ${PYTORCH_COMMIT_ID}
git submodule update --init --recursive
sudo pip3 install pyyaml
sudo pip3 install -r requirements.txt
USE_OPENCV=1 BUILD_TORCH=ON CMAKE_PREFIX_PATH="/usr/bin/" LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH NO_CUDA=0 USE_NNPACK=1 CC=cc CXX=c++ python3 setup.py install
```
### Build the program
Open terminal in folder where you want your ImageUpscalerQt and enter these commands:

**WARNING: replace the `<Your LibTorch installation>` with path to LibTorch.**
```
git clone https://github.com/GRAPHENE9932/ImageUpscalerQt.git
cd ImageUpscalerQt
cmake -DCMAKE_PREFIX_PATH="<Your LibTorch installation>" build
cd build
make
```

## Windows
**It is difficult and you will have to install heavyweight programs**, so I recommend just download program from the releases in this GitHub repository. But if it is necessarily...
### Install Qt5
You can download the Qt installer [Here](https://www.qt.io/download-open-source). But you also need to register a Qt account in order to proceed with the installation.

Select the MSVC 2017 64 bit toolkit.
### Install vcpkg
Vcpkg needed to install other packages/libraries.
Open the cmd.exe, enter to the directory where you want your vcpkg and enter these commands:
```
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
```
Don't close the command line, we need it later.
### Install OpenImageIO
In directory where vcpkg installed, enter this command to install the OpenImageIO library:
```
./vcpkg install openimageio:x64-windows
```
### Install LibTorch
Download it from the [PyTorchs official website](https://pytorch.org/get-started/locally/) and select Stable -> Windows -> LibTorch -> C++/Java -> CPU -> Release C++ version. Extract the downloaded .zip archive to the folder you want.
### Build the program
* Download this repository from github by zip archive or git, and extract it to the folder you want.
* Open the Qt Creator and open project you just extracted.
* Open the "Projects" tab -> click on "Build" -> Edit this in the "Initial CMake parameters"
from
```
-DCMAKE_PREFIX_PATH:STRING=%{Qt:QT_INSTALL_PREFIX}
```
to
```
-DCMAKE_PREFIX_PATH:STRING=%{Qt:QT_INSTALL_PREFIX};<path to libtorch>;<path to openimageio>;<path to openexr>;<path to zlib>
```
![Screenshot](https://i.imgur.com/OrARcD9.png)

**WARNING: replace \<marks\> with real paths to folders using this table:**

|Path|Real path|
|----|---------|
|\<path to libtorch\>|path to your libtorch|
|\<path to openimageio\>|\<Path to vcpkg\>/packages/openimageio_x64-windows|
|\<path to openexr\>|\<Path to vcpkg\>/packages/openexr_x64-windows|
|\<path to zlib\>|\<Path to vcpkg\>/packages/zlib_x64-windows|

* Make sure in the left-bottom corner "Release" is selected.

![Screenshot](https://i.imgur.com/HdVLbaS.png)
* Click on the build button.
* Copy path to folder with built application (usually, name is "build-ImageUpscalerQt-master-Desktop-Qt_5_12_11_MSVC2017_64bit-Release").
* Open folder with windeployqt.exe by cmd.exe (windeployqt usually exists in \<Qt folder\>/5.12.11/msvc2017_64/bin)
* Enter command `./windeployqt "<Path to folder with built application>"`
* Go to the folder with OpenImageIO .dll files (usually `<path to vcpkg>/packages/openimageio_x64-windows/bin`) and copy files `OpenImageIO.dll` and `OpenImageIO_Util.dll` to the folder with application.
* Go to the folder with LibTorch .dll files (usually `<path to libtorch>/lib` and copy files `asmjit.dll`, `c10.dll`, `fbgemm.dll`, `libiomp5md.dll`, `torch_cpu.dll`)
* Go to the folder with openexr .dll files (usually `<path to vcpkg>/packages/openexr_x64-windows/bin`) and copy files `Half-2_5.dll`, `lex-2_5.dll`, `IlmImf-2_5.dll`, `IlmThread-2_5.dll`, `Imath-2_5.dll`.
* Go to the folder with libjpeg-turbo .dll files (usually `<path to vcpkg>/packages/libjpeg-turbo_x64-windows/bin`) and copy file `jpeg62.dll`.
* Go to folder (usually `<path to vcpkg>/packages/libpng_x64-windows/bin`) and copy file `libpng16.dll`.
* Go to folder (usually `<path to vcpkg>/packages/liblzma_x64-windows/bin`) and copy file `lzma.dll`.
* Go to folder (usually `<path to vcpkg>/packages/tiff_x64-windows/bin`) and copy file `tiff.dll`.
* Go to folder (usually `<path to vcpkg>/packages/zlib_x64-windows/bin`) and copy file `zlib1.dll`.
* Finally... Done!