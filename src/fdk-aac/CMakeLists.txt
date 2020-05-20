include(ExternalProject)
ExternalProject_Add(libfdk-aac
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    CONFIGURE_COMMAND ./autogen.sh && ./configure --prefix=${CMAKE_CURRENT_SOURCE_DIR}/output/${HOST} --enable-shared=no --host=${CROSS_COMPILE} CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} CXXFLAGS=-std=gnu++98
    BUILD_COMMAND make && make install
    BUILD_IN_SOURCE 1
)
