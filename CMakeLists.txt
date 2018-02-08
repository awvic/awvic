##---------------------------------------------------------------------------
## Author:      Jorgen Bodde
## Copyright:   (c) Jorgen Bodde
## License:     wxWidgets License
## Update:      2008/12 by Werner Smekal
##---------------------------------------------------------------------------
 
# define minimum cmake version
cmake_minimum_required(VERSION 2.6.2)
 
set(TARGET "awvic")
if (WIN32)
set(PROJECT_CMAKE_DIR "d:/mycodes/awvic/build")
# SET(CMAKE_C_COMPILER "C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/bin/cl.exe")
# SET(CMAKE_CXX_COMPILER "C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/bin/cl.exe")
# SET(CMAKE_C_LINK_EXECUTABLE "C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/bin/link.exe")
# SET(CMAKE_CXX_LINK_EXECUTABLE "C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/bin/link.exe")
else()
set(PROJECT_CMAKE_DIR "/home/octave/mycodes/awvic/build")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()
 
# Our project is called 'minimal' this is how it will be called in
# visual studio, and in our makefiles. 
project(${TARGET})
 
# Location where cmake first looks for cmake modules.
set(CMAKE_MODULE_PATH "${PROJECT_CMAKE_DIR}")

if (WIN32)
set(PROJECT_SOURCE_DIR "d:/mycodes/awvic/src")
set(ICU_INCLUDE_DIR "d:/mylibrary/icu/include")
set(JSONCPP_INCLUDE_DIR "D:/mylibrary/jsoncpp/include")
set(JSONCPP_LIBRARY "D:/mylibrary/jsoncpp/build/win32/debug/lib/jsoncpp.lib")
set(ICU_LIBRARY "d:/mylibrary/icu/lib/icudt.lib" 
                "d:/mylibrary/icu/lib/icuin.lib" 
                "d:/mylibrary/icu/lib/icuio.lib" 
                "d:/mylibrary/icu/lib/icule.lib" 
                "d:/mylibrary/icu/lib/iculx.lib" 
                "d:/mylibrary/icu/lib/icutu.lib" 
                "d:/mylibrary/icu/lib/icuuc.lib")
set(ICU_I18N_LIBRARIES "d:/mylibrary/icu/lib/icudt.lib" 
                       "d:/mylibrary/icu/lib/icuin.lib" 
                       "d:/mylibrary/icu/lib/icuio.lib" 
                       "d:/mylibrary/icu/lib/icule.lib" 
                       "d:/mylibrary/icu/lib/iculx.lib" 
                       "d:/mylibrary/icu/lib/icutu.lib" 
                       "d:/mylibrary/icu/lib/icuuc.lib")
set(PYTHON_INCLUDE_DIR "c:/python35-32/include")
set(PYTHON_LIBRARY "c:/python35-32/libs/python35.lib")
# set(PYTHON_INCLUDE_DIR "c:/python34_win32/include")
# set(PYTHON_LIBRARY "c:/python34_win32/libs/python34.lib")
else()
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/home/octave/mycodes/wx/awvic/build_cmake")
#set(CMAKE_MODULE_PATH ".")
#set(CMAKE_MODULE_PATH "/home/octave/mycodes/awvic/build_cmake")
set(PROJECT_SOURCE_DIR "/home/octave/mycodes/awvic/src")
set(ICU_INCLUDE_DIR "/usr/local/include")
#set(JSONCPP_INCLUDE_DIR "/home/octave/mylibrary/jsoncpp/include")
#set(JSONCPP_LIBRARY "/home/octave/mylibrary/jsoncpp/build/debug/lib/libjsoncpp.a")
set(JSONCPP_INCLUDE_DIR "/usr/local/include")
set(JSONCPP_LIBRARY "/usr/local/lib/libjsoncpp.a")
set(ICU_LIBRARY "/usr/local/lib/libicudata.so" 
                "/usr/local/lib/libicui18n.so" 
                "/usr/local/lib/libicuio.so" 
                "/usr/local/lib/libicule.so" 
                "/usr/local/lib/libiculx.so" 
                "/usr/local/lib/libicutu.so" 
                "/usr/local/lib/libicuuc.so")
set(ICU_I18N_LIBRARIES "/usr/local/lib/libicudata.so" 
                       "/usr/local/lib/libicui18n.so" 
                       "/usr/local/lib/libicuio.so" 
                       "/usr/local/lib/libicule.so" 
                       "/usr/local/lib/libiculx.so" 
                       "/usr/local/lib/libicutu.so" 
                       "/usr/local/lib/libicuuc.so")
set(PYTHON_INCLUDE_DIR "D:/mylibrary/jsoncpp/include")
set(PYTHON_LIBRARY "D:/mylibrary/jsoncpp/build/win32/debug/lib/jsoncpp.lib")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

include_directories(${JSONCPP_INCLUDE_DIR})
include_directories(${PYTHON_INCLUDE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

##---------------------------------------------------
## Please set your wxWidgets configuration here
##--------------------------------------------------- 
 
set(wxwidgets_path "d:/wxWidgets-3.0.3")
SET(WX_ROOT_DIR ${wxwidgets_path})
SET(wxWidgets_CONFIGURATION mwsu)
SET(wxWidgets_ROOT_DIR ${wxwidgets_path})
# SET(wxWidgets_LIB_DIR ${wxwidgets_path}/lib/vc_x64_lib)
SET(wxWidgets_LIB_DIR ${wxwidgets_path}/lib/vc_lib)

# Here you can define what libraries of wxWidgets you need for your
# application. You can figure out what libraries you need here;
# http://www.wxwidgets.org/manuals/2.8/wx_librarieslist.html
# We need the Find package for wxWidgets to work
# NOTE: if you're using aui, include aui in this required components list.
 
# It was noticed that when using MinGW gcc it is essential that 'core' is mentioned before 'base'.
find_package(wxWidgets COMPONENTS core base adv aui xml REQUIRED)
 
##---------------------------------------------------
## Actual config file starts here
##--------------------------------------------------- 
 
# wxWidgets include (this will do all the magic to configure everything)
include( "${wxWidgets_USE_FILE}" )
 
# ICU
find_package(ICU REQUIRED)
if (ICU_FOUND)
    if (ICU_VERSION VERSION_LESS "4.2")
        unset(ICU_FOUND CACHE)
        unset(ICU_INCLUDE_DIRS CACHE)
        unset(ICU_LIBRARIES CACHE)
        message(FATAL_ERROR "ICU is too old, found ${ICU_VERSION} and we need 4.2")
    endif ()
    include_directories(${ICU_INCLUDE_DIRS})
endif (ICU_FOUND)

# For convenience we define the sources as a variable. You can add 
# header files and cpp/c files and CMake will sort them out
if(WIN32)
set(SRCS "${PROJECT_SOURCE_DIR}/awvic.cpp"
         "${PROJECT_SOURCE_DIR}/svTextEditorCtrl.cpp"
         "${PROJECT_SOURCE_DIR}/svMainFrame.cpp"
         "${PROJECT_SOURCE_DIR}/svAboutDialog.cpp"
         "${PROJECT_SOURCE_DIR}/svUndoAction.cpp"
         "${PROJECT_SOURCE_DIR}/svDebugDialog01.cpp"
         "${PROJECT_SOURCE_DIR}/svCommand.cpp"
         "${PROJECT_SOURCE_DIR}/svListOfIntList.cpp"
         "${PROJECT_SOURCE_DIR}/svTextStyleList.cpp"
         "${PROJECT_SOURCE_DIR}/svTextView.cpp"
         "${PROJECT_SOURCE_DIR}/svPozList.cpp"
         "${PROJECT_SOURCE_DIR}/svFindReplaceCtrl.cpp"
         "${PROJECT_SOURCE_DIR}/svLineText.cpp"
         "${PROJECT_SOURCE_DIR}/svBufText.cpp"
         "${PROJECT_SOURCE_DIR}/svTheme.cpp"
         "${PROJECT_SOURCE_DIR}/svPreference.cpp"
         "${PROJECT_SOURCE_DIR}/svCommonLib.cpp"
         "${PROJECT_SOURCE_DIR}/svDictionary.cpp"
         "${PROJECT_SOURCE_DIR}/svAction.cpp"
         "${PROJECT_SOURCE_DIR}/svCaret.cpp"
         "${PROJECT_SOURCE_DIR}/svOpenFilesCtrl.cpp"
         "${PROJECT_SOURCE_DIR}/svTypeHintCtrl.cpp"
         "${PROJECT_SOURCE_DIR}/svCommandLineCtrl.cpp"
         "${PROJECT_SOURCE_DIR}/svListBoxCtrl.cpp"
         # "${PROJECT_SOURCE_DIR}/svTextEdit.cpp"
         # "${PROJECT_SOURCE_DIR}/svRELib.cpp"
         "${PROJECT_SOURCE_DIR}/svScrollBar.cpp")
else()
set(SRCS "${PROJECT_SOURCE_DIR}/awvic.cpp"
         "${PROJECT_SOURCE_DIR}/svTextEditorCtrl.cpp"
         "${PROJECT_SOURCE_DIR}/svMainFrame.cpp"
         "${PROJECT_SOURCE_DIR}/svAboutDialog.cpp"
         "${PROJECT_SOURCE_DIR}/svUndoAction.cpp"
         "${PROJECT_SOURCE_DIR}/svDebugDialog01.cpp"
         "${PROJECT_SOURCE_DIR}/svCommand.cpp"
         "${PROJECT_SOURCE_DIR}/svListOfIntList.cpp"
         "${PROJECT_SOURCE_DIR}/svTextStyleList.cpp"
         "${PROJECT_SOURCE_DIR}/svTextView.cpp"
         "${PROJECT_SOURCE_DIR}/svPozList.cpp"
         "${PROJECT_SOURCE_DIR}/svFindReplaceCtrl.cpp"
         "${PROJECT_SOURCE_DIR}/svLineText.cpp"
         "${PROJECT_SOURCE_DIR}/svBufText.cpp"
         "${PROJECT_SOURCE_DIR}/svTheme.cpp"
         "${PROJECT_SOURCE_DIR}/svPreference.cpp"
         "${PROJECT_SOURCE_DIR}/svCommonLib.cpp"
         "${PROJECT_SOURCE_DIR}/svDictionary.cpp"
         "${PROJECT_SOURCE_DIR}/svAction.cpp"
         "${PROJECT_SOURCE_DIR}/svCaret.cpp"
         "${PROJECT_SOURCE_DIR}/svOpenFilesCtrl.cpp"
         "${PROJECT_SOURCE_DIR}/svTypeHintCtrl.cpp"
         "${PROJECT_SOURCE_DIR}/svCommandLineCtrl.cpp"
         "${PROJECT_SOURCE_DIR}/svListBoxCtrl.cpp"
         # "${PROJECT_SOURCE_DIR}/svTextEdit.cpp"
         # "${PROJECT_SOURCE_DIR}/svRELib.cpp"
         "${PROJECT_SOURCE_DIR}/svScrollBar.cpp")
endif()
 
# If we build for windows systems, we also include the resource file
# containing the manifest, icon and other resources
#if(WIN32)
#  set(SRCS ${SRCS} minimal.rc)
#endif(WIN32)
 
# Here we define the executable minimal.exe or minimal on other systems
# the above paths and defines will be used in this build
add_executable(${TARGET} WIN32 ${SRCS})
 
# We add to our target 'minimal' the wxWidgets libraries. These are
# set for us by the find script. If you need other libraries, you
# can add them here as well. 
#target_link_libraries(${target} ${ICU_LIBRARIES} ${ICU_I18N_LIBRARIES}) 
target_link_libraries(${TARGET} ${wxWidgets_LIBRARIES} ${ICU_LIBRARIES} ${ICU_I18N_LIBRARIES} ${JSONCPP_LIBRARY} ${PYTHON_LIBRARY})

message( STATUS " ")
message( STATUS "ICU_FOUND: " ${ICU_FOUND})
message( STATUS "ICU_VERSION: " ${ICU_VERSION})
message( STATUS "CMAKE_BINARY_DIR: " ${CMAKE_BINARY_DIR})
message( STATUS "CMAKE_COMMAND: " ${CMAKE_COMMAND})
message( STATUS "CMAKE_CURRENT_BINARY_DIR: " ${CMAKE_CURRENT_BINARY_DIR})
message( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR})
message( STATUS "CMAKE_FILE_DIRECTORY: " ${CMAKE_FILE_DIRECTORY})
message( STATUS "CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR})
message( STATUS "CMAKE_PROJECT_NAME: " ${CMAKE_PROJECT_NAME})
message( STATUS "CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})
message( STATUS "PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR})
message( STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR})
message( STATUS "CMAKE_SYSTEM: " ${CMAKE_SYSTEM})
message( STATUS " ")
