noinst_LTLIBRARIES = libcamera.la

libcamera_la_SOURCES = \
    Source/Camera.cc 

noinst_HEADERS = \
    Header/Camera.h 

# include flags for preprocessor
libcamera_la_CPPFLAGS = \
    -I$(srcdir)/Header \
    -I$(top_srcdir)/Libraries/Database/Header \
    -I$(top_srcdir)/Libraries/Exception/Header \
    -I$(top_srcdir)/Libraries/ThreadSafeQueue/Header

libcamera_la_LIBADD = \
    $(top_builddir)/Libraries/Database/libdatabase.la \
    $(top_builddir)/Libraries/Exception/libexception.la \
    $(top_builddir)/Libraries/ThreadSafeQueue/libthreadsafequeue.la \
    -lv4l2
