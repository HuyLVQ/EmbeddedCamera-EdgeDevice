noinst_LTLIBRARIES = libsecurityservice.la

# .cc files compiled into libcommand.la
libsecurityservice_la_SOURCES = \
    Source/DTLSService.cc

noinst_HEADERS = \
    Header/SecurityServiceIf.h \
    Header/DTLSService.h

# include flags for preprocessor
libsecurityservice_la_CPPFLAGS = \
    -I$(srcdir)/Header \


libsecurityservice_la_LIBADD = \
    -lssl \
    -lcrypto
