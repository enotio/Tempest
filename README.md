Tempest
=

Crossplatform 3d engine.

## Supported platforms and API:

* Windows: OpenGL2/GLSL, DirectX9/HLSL, DirectX9/Cg

* Android: OpenGLES2/GLSL

## How to build

### 1. Install dependencies:


 1.1. Install DirectXSDK,             from [www.microsoft.com](www.microsoft.com/en-us/download/details.aspx?id=6812)

 1.2. Install QtSDK(need qmake, plus mingw4.8+), from [http://qt-project.org/downloads](http://qt-project.org/downloads)

 1.3. Install Android SDK+NDK to compile android version

 1.4. Install CgToolkit(optional),              from [developer.nvidia.com/cg-toolkit-download](developer.nvidia.com/cg-toolkit-download)

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


