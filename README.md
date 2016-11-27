![Tempest Logo](https://raw.githubusercontent.com/enotio/Tempest/master/doc_src/icon.png)
=
[![Build Status](https://travis-ci.org/enotio/Tempest.svg?branch=master)](https://travis-ci.org/enotio/Tempest)
[![Build status](https://ci.appveyor.com/api/projects/status/e8v457k9tfuk511m?svg=true)](https://ci.appveyor.com/project/Try/Tempest)

Crossplatform 3d engine.

## Supported platforms and API:

* Windows: OpenGL4/GLSL, OpenGL2/GLSL, DirectX11/HLSL, DirectX9/HLSL, DirectX9/Cg

* OSX: OpenGL2/GLSL

* Android: OpenGLES2/GLSL

* iOS: OpenGLES2/GLSL

## How to build

### 1. Install dependencies:


 1.1. Install DirectXSDK,             from [www.microsoft.com](http://www.microsoft.com/en-us/download/details.aspx?id=6812)

 1.2. Install QtSDK(need qmake, plus mingw4.8+), from [http://qt-project.org/downloads](http://qt-project.org/downloads)

 1.3. Install Android SDK+NDK to compile android version

 1.4. Install CgToolkit(optional),              from [developer.nvidia.com/cg-toolkit-download](http://developer.nvidia.com/cg-toolkit-download)

     hint: you can disable directx support in .pro file.
     hint: you can enable cg support in .pro file.

### 2. Build(Console):
 2.1 open Tempest directory

 2.2 run qmake

 2.3 run make

### 3. Build(QtCreator 3.0):
 2.1 open Tempest.pro

 2.2 configure build kit( set to MinGW4.8 )

 2.3 click build button

## Examples

### [00-Cube](https://github.com/enotio/Tempest/tree/master/Examples/Cube)
![example-01-cube](https://raw.githubusercontent.com/enotio/Tempest/master/doc_src/screens/cube.png)

### [01-Bump](https://github.com/enotio/Tempest/tree/master/Examples/Bump)
![example-02-bump](https://raw.githubusercontent.com/enotio/Tempest/master/doc_src/screens/bump.png)

### [02-Paint2d](https://github.com/enotio/Tempest/tree/master/Examples/Painting2d)
![example-03-paint2d](https://raw.githubusercontent.com/enotio/Tempest/master/doc_src/screens/paint2d.png)

### [03-Load3ds](https://github.com/enotio/Tempest/tree/master/Examples/Load3ds)
![example-04-bump](https://raw.githubusercontent.com/enotio/Tempest/master/doc_src/screens/load3ds.png)

### [04-Volume](https://github.com/enotio/Tempest/tree/master/Examples/Volume)
![example-04-bump](https://raw.githubusercontent.com/enotio/Tempest/master/doc_src/screens/volume.png)



