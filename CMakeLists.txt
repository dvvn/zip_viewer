cmake_minimum_required(VERSION 3.19)
project(QtWidgetsApplication1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(MINIZIP_SRC
        mz_compat.c
        mz_crypt.c
        mz_os.c
        mz_strm.c
        #[[mz_strm_buf.c]]
        mz_strm_mem.c
        #[[mz_strm_split.c]]
        mz_zip.c
        #[[mz_zip_rw.c]])

if(UNIX)
    list(APPEND MINIZIP_SRC mz_os_posix.c mz_strm_os_posix.c)
elseif(WIN32)
    list(APPEND MINIZIP_SRC mz_os_win32.c mz_strm_os_win32.c)
endif()

add_executable(${PROJECT_NAME} main.cpp
        QtWidgetsApplication1.cpp QtWidgetsApplication1.h
        QtWidgetsApplication1.ui
        QtWidgetsApplication1.qrc ${MINIZIP_SRC})

find_package(Qt5 COMPONENTS Widgets REQUIRED)

qt5_use_modules(${PROJECT_NAME} Widgets)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

