bin_PROGRAMS = client

client_SOURCES = \
    Source/ServerTransmitter.cc \
    Source/main.cc

client_CPPFLAGS = \
    -I$(top_srcdir) \
    -I$(srcdir)/Header \
    -I$(top_srcdir)/Libraries/ThreadSafeQueue/Header \
    -I$(top_srcdir)/Libraries/Database/Header \
    -I$(top_srcdir)/Libraries/Exception/Header \
    -I$(top_srcdir)/Libraries/ThreadSafeQueue/Header \
    -I$(top_srcdir)/Libraries/Frame/Header \
    -I$(top_srcdir)/Camera/Header \
    -I$(top_srcdir)/Blurring/Header


client_LDADD = \
    $(top_builddir)/Libraries/Database/libdatabase.la \
    $(top_builddir)/Libraries/Exception/libexception.la \
    $(top_builddir)/Libraries/ThreadSafeQueue/libthreadsafequeue.la \
    $(top_builddir)/Libraries/Frame/libframe.la \
    $(top_builddir)/Libraries/SecurityService/libsecurityservice.la \
    $(top_builddir)/Camera/libcamera.la \
    $(top_builddir)/Blurring/libblurring.la \
    -lssl \
    -lcrypto
