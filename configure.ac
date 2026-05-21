AC_PREREQ([2.67])

AC_INIT([Embedded Camera], [1.0.0])

AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_CXX 

AM_PROG_AR

LT_INIT([disable-shared static])

AX_CXX_COMPILE_STDCXX_17


AC_CONFIG_FILES([
    Makefile
    Libraries/Makefile
    Libraries/Exception/Makefile
    Libraries/ThreadSafeQueue/Makefile
    Libraries/Database/Makefile
    Libraries/Frame/Makefile
    Libraries/SecurityService/Makefile
    Camera/Makefile
    Blurring/Makefile 
    Camera/Makefile
    Blurring/Makefile
    ServerTransmitter/Makefile                                                        
])

AC_OUTPUT