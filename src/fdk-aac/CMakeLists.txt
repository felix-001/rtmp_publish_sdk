include(ExternalProject)
ExternalProject_Add(fdk-aac
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    CONFIGURE_COMMAND ./autogen.sh && ./configure --enable-shared=no --host=${CROSS_COMPILE} CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
    BUILD_COMMAND make
    BUILD_IN_SOURCE 1
)
