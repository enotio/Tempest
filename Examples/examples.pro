TEMPLATE = subdirs

DESTDIR = bin
win32:{
    SUBDIRS += \
    Cube/Cube.pro \
    DxCube/DxCube.pro \
    Tesselation/Tesselation.pro
}

SUBDIRS = \
 Bump/Bump.pro \
# Cube/Cube.pro \
# DxCube/DxCube.pro \
 Gui/Gui.pro \
 #Load3ds/Load3ds.pro \
 Mrt/Mrt.pro \
 OverlayPaint/OverlayPaint.pro \
 Painting2d/Painting2d.pro \
 Pixmap/Pixmap.pro \
 Scene/Scene.pro \
 Volume/Volume.pro
