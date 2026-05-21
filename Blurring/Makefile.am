noinst_LTLIBRARIES = libblurring.la

libblurring_la_SOURCES = \
    Source/Blurring.cc

noinst_HEADERS = \
    Header/Blurring.h

libblurring_la_CPPFLAGS = \
    -I$(srcdir)/Header \
    -I$(top_srcdir)/Libraries/Database/Header \
    -I$(top_srcdir)/Libraries/Exception/Header \
    -I$(top_srcdir)/Libraries/ThreadSafeQueue/Header \
    -I$(top_srcdir)/Camera/Header

libblurring_la_LIBADD = \
    $(top_builddir)/Libraries/Database/libdatabase.la \
    $(top_builddir)/Libraries/Exception/libexception.la \
    $(top_builddir)/Libraries/ThreadSafeQueue/libthreadsafequeue.la \
    $(top_builddir)/Camera/libcamera.la