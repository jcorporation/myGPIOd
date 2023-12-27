message("Installing libgpiod from source")

include(FetchContent)

set(INSTALL ${PROJECT_BINARY_DIR})
set(DOWNLOAD_VERSION "2.1")

list(APPEND CMAKE_PREFIX_PATH ${INSTALL})
find_package(PkgConfig REQUIRED)

if(EXISTS "${PROJECT_SOURCE_DIR}/libgpiod-${DOWNLOAD_VERSION}.tar.gz")
    message("Found libgpiod archive")
    FetchContent_Declare(libgpiod
        URL "${PROJECT_SOURCE_DIR}/libgpiod-${DOWNLOAD_VERSION}.tar.gz"
    )
else()
    message("Cloning libgpiod")
    FetchContent_Declare(libgpiod
        GIT_REPOSITORY "https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git"
        GIT_TAG "v${DOWNLOAD_VERSION}"
        UPDATE_DISCONNECTED ON
    )
endif()

FetchContent_Populate(libgpiod)

set(ENV${CC} ${CMAKE_C_COMPILER})

# Configure
execute_process(
    COMMAND ${libgpiod_SOURCE_DIR}/autogen.sh --host=${CMAKE_SYSTEM_PROCESSOR} -enable-tools=no --prefix=${INSTALL}
    WORKING_DIRECTORY ${libgpiod_BINARY_DIR}
)

# Build
execute_process(
    COMMAND make
    WORKING_DIRECTORY ${libgpiod_BINARY_DIR}
)

# Install
execute_process(
    COMMAND make install
    WORKING_DIRECTORY ${libgpiod_BINARY_DIR}
)

set(LIBGPIOD_INCLUDE_DIRS "${PROJECT_BINARY_DIR}/include")
set(LIBGPIOD_LIBRARIES "${PROJECT_BINARY_DIR}/lib/libgpiod.a")
