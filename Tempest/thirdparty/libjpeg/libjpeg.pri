INCLUDEPATH  += $$PWD/
DEFINES      += _CRT_SECURE_NO_WARNINGS

win32-msvc*:QMAKE_CFLAGS += /wd4996 /wd4244 /wd4018 /wd4305

HEADERS += \
  $$PWD/jconfig.h \
  $$PWD/jdct.h \
  $$PWD/jerror.h \
  $$PWD/jinclude.h \
  $$PWD/jmemsys.h \
  $$PWD/jmorecfg.h \
  $$PWD/jpegint.h \
  $$PWD/jpeglib.h \
  $$PWD/jversion.h

SOURCES += \
  $$PWD/md5/md5.c \
  $$PWD/cdjpeg.c \
  $$PWD/jaricom.c \
  $$PWD/jcapimin.c \
  $$PWD/jcapistd.c \
  $$PWD/jcarith.c \
  $$PWD/jccoefct.c \
  $$PWD/jccolext.c \
  $$PWD/jccolor.c \
  $$PWD/jcdctmgr.c \
  $$PWD/jcext.c \
  $$PWD/jchuff.c \
  $$PWD/jcinit.c \
  $$PWD/jcmainct.c \
  $$PWD/jcmarker.c \
  $$PWD/jcmaster.c \
  $$PWD/jcomapi.c \
  $$PWD/jcparam.c \
  $$PWD/jcphuff.c \
  $$PWD/jcprepct.c \
  $$PWD/jctrans.c \
  $$PWD/jdapimin.c \
  $$PWD/jdapistd.c \
  $$PWD/jdarith.c \
  $$PWD/jdatadst-tj.c \
  $$PWD/jdatadst.c \
  $$PWD/jdatasrc-tj.c \
  $$PWD/jdatasrc.c \
  $$PWD/jdcoefct.c \
  $$PWD/jdcol565.c \
  $$PWD/jdcolext.c \
  $$PWD/jdcolor.c \
  $$PWD/jddctmgr.c \
  $$PWD/jdhuff.c \
  $$PWD/jdinput.c \
  $$PWD/jdmainct.c \
  $$PWD/jdmarker.c \
  $$PWD/jdmaster.c \
  $$PWD/jdmerge.c \
  $$PWD/jdmrg565.c \
  $$PWD/jdmrgext.c \
  $$PWD/jdphuff.c \
  $$PWD/jdpostct.c \
  $$PWD/jdsample.c \
  $$PWD/jdtrans.c \
  $$PWD/jerror.c \
  $$PWD/jfdctflt.c \
  $$PWD/jfdctfst.c \
  $$PWD/jfdctint.c \
  $$PWD/jidctflt.c \
  $$PWD/jidctfst.c \
  $$PWD/jidctint.c \
  $$PWD/jidctred.c \
  $$PWD/jmemmgr.c \
  $$PWD/jmemnobs.c \
  $$PWD/jquant1.c \
  $$PWD/jquant2.c \
  $$PWD/jsimd_none.c \
  $$PWD/jstdhuff.c \
  $$PWD/jutils.c \
  $$PWD/rdbmp.c \
  $$PWD/rdcolmap.c \
  $$PWD/rdgif.c \
  $$PWD/rdjpeg.c \
  $$PWD/rdpng.c \
  $$PWD/rdppm.c \
  $$PWD/rdrle.c \
  $$PWD/rdswitch.c \
  $$PWD/rdtarga.c \
  $$PWD/tjutil.c \
  $$PWD/transupp.c \
  $$PWD/turbojpeg.c \
  $$PWD/wrbmp.c \
  $$PWD/wrgif.c \
  $$PWD/wrppm.c \
  $$PWD/wrrle.c \
  $$PWD/wrtarga.c \
  $$PWD/jcsample.c

