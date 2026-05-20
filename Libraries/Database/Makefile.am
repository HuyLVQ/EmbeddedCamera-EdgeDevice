noinst_LTLIBRARIES = libdatabase.la

# .cc files compiled into libcommand.la
libdatabase_la_SOURCES = \
    Source/Database.cc

noinst_HEADERS = \
    Header/Database.h

# include flags for preprocessor
libdatabase_la_CPPFLAGS = \
    -I$(srcdir)/Header \
    -I$(top_srcdir)/Libraries/ThreadSafeQueue/Header \
    -I$(top_srcdir)/Libraries/Exception/Header

libcommand_la_LIBADD = \
    $(top_builddir)/Libraries/Database/libthreadsafequeue.la \
    $(top_builddir)/Libraries/Exception/libexception.la
