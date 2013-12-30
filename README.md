Tempest
=

Crossplatform 3d engine.

## Supported platforms and API:

* Windows: OpenGL2/GLSL, DirectX9/Cg

* Android: OpenGLES2

## How to build

### 1. Install dependencies:

 1.1. Install CgToolkit,              from [developer.nvidia.com/cg-toolkit-download](developer.nvidia.com/cg-toolkit-download)

 1.2. Install DirectXSDK,             from [www.microsoft.com](www.microsoft.com/en-us/download/details.aspx?id=6812)

 1.3. Install QtSDK(need qmake, plus mingw4.8+ only), from [http://qt-project.org/downloads](http://qt-project.org/downloads)

 1.4. Install Android SDK+NDK to compile android version

     *hint: you can disable directx support in .pro file.*

### 2. Build(Windows):
 2.1 open Tempest directory
 2.2 run qmake
 2.3 run make



