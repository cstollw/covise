# @file
#
# Root-file for the Covise configure/build setup process
#
# @author Blasius Czink
#

# requirements to cmake version

# 2.8.7 is too old for identifying clang
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.10)

set(COVISE_BUILDING_COVISE TRUE)

# allow only out-of-source builds
STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" in_source_build)
IF(in_source_build)
  MESSAGE(FATAL_ERROR "A separate build directory is required, please create one and run cmake from this directory")
ENDIF(in_source_build)

# covise project
PROJECT(covise CXX C)

set(COVISE_EXPORTS_FILE "covise-exports.cmake")

########## General setup ##########

# this is used to override some of cmake's original find-modules
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

###########workaround to find vtk-5.8
#INCLUDE_DIRECTORIES(vtk-5.8)
#LINK_DIRECTORIES(/usr/lib64/vtk-5.8)
#######################################

IF(WIN32)
  SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/windows;${CMAKE_MODULE_PATH}")
ENDIF()
IF(UNIX)
  SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/unix;${CMAKE_MODULE_PATH}")
ENDIF()
IF(APPLE)
  SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/apple;${CMAKE_MODULE_PATH}")
ENDIF()
IF(MINGW)
  SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/mingw;${CMAKE_MODULE_PATH}")
ENDIF(MINGW)

# set COVISEDIR, BASEARCHSUFFIX and some other cmake-variables
SET(COVISEDIR "${CMAKE_SOURCE_DIR}")
SET(ARCHSUFFIX "$ENV{ARCHSUFFIX}")
# archsuffix has to be set
IF(ARCHSUFFIX STREQUAL "")
   MATH(EXPR BITS '${CMAKE_SIZEOF_VOID_P}*8')
   SET(ARCHSUFFIX ${CMAKE_SYSTEM_NAME}${BITS})
   MESSAGE(WARNING "Environment variable ARCHSUFFIX was not set, using ARCHSUFFIX=${ARCHSUFFIX}")
ENDIF()

STRING(TOLOWER "${ARCHSUFFIX}" ARCHSUFFIX)
STRING(REGEX REPLACE "opt$" "" BASEARCHSUFFIX "${ARCHSUFFIX}")
SET(DBG_ARCHSUFFIX "${BASEARCHSUFFIX}")
STRING(REGEX REPLACE "xenomai$" "" BASEARCHSUFFIX "${BASEARCHSUFFIX}")
STRING(REGEX REPLACE "mpi$" "" BASEARCHSUFFIX "${BASEARCHSUFFIX}")
IF(ARCHSUFFIX STREQUAL DBG_ARCHSUFFIX)
  SET(CMAKE_BUILD_TYPE "Debug")
ELSE()
  SET(CMAKE_BUILD_TYPE "RelWithDebInfo")
ENDIF()
MESSAGE(CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

IF(UNIX AND NOT APPLE)
  EXECUTE_PROCESS(COMMAND readlink -e ${COVISEDIR}
      OUTPUT_VARIABLE CANONICAL_COVISE_DIR
      ERROR_VARIABLE CANONICAL_COVISE_DIR_error
      RESULT_VARIABLE CANONICAL_COVISE_DIR_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)
ELSE()
  SET(CANONICAL_COVISE_DIR ${COVISEDIR})
ENDIF()

# figure out COVISE version
FIND_PACKAGE(Git)
# TODO: get version from COVISE_VERSION environment variable if it is set

set(COVISE_VERSION_YEAR 2015)
set(COVISE_VERSION_MONTH 1)
set(COVISE_VERSION_DAY 0)
set(COVISE_GIT_DATE 1970-01-01)
set(COVISE_VERSION_HASH "")
if (GIT_FOUND)
   execute_process(COMMAND ${GIT_EXECUTABLE} log -n1 --format=%t/%ci
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
      OUTPUT_VARIABLE GIT_LOG_OUT
      ERROR_VARIABLE GIT_LOG_ERROR
      RESULT_VARIABLE GIT_LOG_RESULT
      OUTPUT_STRIP_TRAILING_WHITESPACE)

   if(NOT ${GIT_LOG_RESULT} EQUAL 0)
      message("Command \"${GIT_EXECUTABLE} log -n1 --format=%H/%ci ${dir}\" failed with output:\n${GIT_LOG_ERROR}")
      set(COVISE_VERSION "2015.1")
      set(COVISE_VERSION_HASH "unknown")
   else()
      string(REGEX REPLACE "^([a-z0-9]*)/(....)-(..)-(..).*" "\\2-\\3-\\4" COVISE_GIT_DATE "${GIT_LOG_OUT}")
      string(REGEX REPLACE "^([a-z0-9]*)/(....)-(..)-(..).*" "\\1" COVISE_VERSION_HASH "${GIT_LOG_OUT}")
      string(REGEX REPLACE "^([a-z0-9]*)/(....)-(..)-(..).*" "\\2" COVISE_VERSION_YEAR "${GIT_LOG_OUT}")
      string(REGEX REPLACE "^([a-z0-9]*)/(....)-(..)-(..).*" "\\3" COVISE_VERSION_MONTH "${GIT_LOG_OUT}")
      string(REGEX REPLACE "^0([0-9]*)" "\\1" COVISE_VERSION_MONTH "${COVISE_VERSION_MONTH}")
      string(REGEX REPLACE "^([a-z0-9]*)/(....)-(..)-(..).*" "\\4" COVISE_VERSION_DAY "${GIT_LOG_OUT}")
      string(REGEX REPLACE "^0([0-9]*)" "\\1" COVISE_VERSION_DAY "${COVISE_VERSION_DAY}")
      #message("git log output: ${GIT_LOG_OUT}")
   endif()
endif()
SET(COVISE_MAJOR_VERSION "${COVISE_VERSION_YEAR}")
SET(COVISE_MINOR_VERSION "${COVISE_VERSION_MONTH}")
SET(COVISE_PATCH_VERSION "${COVISE_VERSION_DAY}")
SET(COVISE_VERSION "${COVISE_VERSION_YEAR}.${COVISE_VERSION_MONTH}-${COVISE_VERSION_HASH}")

MESSAGE("COVISEDIR=${COVISEDIR}")
MESSAGE("ARCHSUFFIX=${ARCHSUFFIX}, BASEARCHSUFFIX=${BASEARCHSUFFIX}")
MESSAGE("COVISE_VERSION = ${COVISE_VERSION}, Last git change on ${COVISE_GIT_DATE}")

# COVISE global options
OPTION(COVISE_USE_CPP11 "enable C++ 11 features" ON)
OPTION(COVISE_USE_FOLDERS "Enable solution folders in Visual Studio. Disable for Express versions." ON)
OPTION(COVISE_PLACE_BINARIES_INSOURCE "Place COVISE binaries in <COVISEDIR>/<archsuffix> (default). Otherwise in <CMAKE_BINARY_DIR>/<archsuffix>." ON)
OPTION(COVISE_EXPORT_TO_INSTALL "Place COVISE exported targets cmake-file in <install directory>/<archsuffix>/lib (default). Otherwise in build directory." ON)
IF(UNIX)
  OPTION(COVISE_WARNING_IS_ERROR "Treat warnings as errors" ON)
  OPTION(COVISE_USE_FORTRAN "Enable Modules using Fortran" ON)
ELSE(UNIX)
  OPTION(COVISE_WARNING_IS_ERROR "Treat warnings as errors" OFF)
  OPTION(COVISE_USE_FORTRAN "Enable Modules using Fortran" ON)
ENDIF(UNIX)

option(COVISE_NATIVE_ARCH "Optimize for system that is used to compile - may be incompatible with other systems" ON)

option(COVISE_SANITIZE_UNDEFINED "Build with -fsanitize=undefined" OFF)
option(COVISE_SANITIZE_ADDRESS "Build with -fsanitize=address" OFF)
option(COVISE_SANITIZE_THREAD "Build with -fsanitize=thread" OFF)

OPTION(COVISE_BUILD_STATIC "Build static COVISE libraries" OFF)                                                                                                                                                                                                                               
OPTION(COVISE_BUILD_SYS "Build COVISE system applications" ON)
OPTION(COVISE_BUILD_MODULES "Build COVISE modules" ON)

OPTION(COVISE_BUILD_WEBSERVICE "Build web service interface" OFF)
OPTION(COVISE_BUILD_DRIVINGSIM "Build driving simulator " OFF)

OPTION(COVISE_USE_CUDA "Use CUDA" ON)

OPTION(COVISE_USE_QT4 "Use Qt 4" OFF)
OPTION(COVISE_USE_VIRVO "Use Virvo volume rendering library" ON)

IF(COVISE_USE_FOLDERS)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
ENDIF(COVISE_USE_FOLDERS)
# enable "Fortran" (keep it optional and check availability with CMAKE_Fortran_COMPILER_WORKS where needed)
IF(COVISE_USE_FORTRAN)
  ENABLE_LANGUAGE(Fortran OPTIONAL)
ENDIF(COVISE_USE_FORTRAN)

SET(CPACK_GENERATOR "TGZ") 
SET(CPACK_PACKAGE_FILE_NAME covise-${ARCHSUFFIX})
INCLUDE(CPackConfig)
INCLUDE(CPack)

# COVISE helper macros 
INCLUDE(Qt4-5)
# preset some initial values for install dir and CFLAGS etc.
INCLUDE(CovisePresets)
covise_cmake_policy()

INCLUDE(CoviseHelperMacros)

# global feature tests
INCLUDE(CoviseGlobalFeatureTests)

# Create macro list for USING
CREATE_USING()

# find important covise dependencies
IF(APPLE)
  FIND_LIBRARY(APP_SERVICES_LIBRARY ApplicationServices)
  MARK_AS_ADVANCED(APP_SERVICES_LIBRARY)
ENDIF(APPLE)
IF(COVISE_BUILD_WEBSERVICE)
  COVISE_FIND_PACKAGE(GSOAP)
ENDIF(COVISE_BUILD_WEBSERVICE)
find_qt()
COVISE_FIND_PACKAGE(Threads)
COVISE_FIND_PACKAGE(OpenSSL)
COVISE_FIND_PACKAGE(OpenGL)
COVISE_FIND_PACKAGE(GLEW) # virvo depends on it
if (NOT GLEW_FOUND)
   set (COVISE_USE_VIRVO OFF)
   message("Not building Virvo: no GLEW")
endif()
COVISE_FIND_PACKAGE(GLUT)
COVISE_FIND_PACKAGE(ZLIB)
COVISE_FIND_PACKAGE(JPEGTURBO)
if (NOT JPEGTURBO_FOUND)
   COVISE_FIND_PACKAGE(JPEG)
endif()
COVISE_FIND_PACKAGE(PNG)
COVISE_FIND_PACKAGE(TIFF REQUIRED)
COVISE_FIND_PACKAGE(FFMPEG)
COVISE_FIND_PACKAGE(Steereo)
COVISE_FIND_PACKAGE(Inventor)
COVISE_FIND_PACKAGE(XercesC REQUIRED)
covise_find_package(OpenSceneGraph 3.2.0 COMPONENTS osgDB osgUtil osgViewer osgParticle osgText osgSim osgGA osgFX)
covise_find_package(VTK COMPONENTS vtkIOLegacy vtkFiltersCore vtkCommonCore vtkImagingCore vtkCommonDataModel vtkCommonExecutionModel NO_MODULE)

covise_find_cuda()

COVISE_FIND_PACKAGE(Cg)

set(PythonLibs_FIND_VERSION 3.0)
set(Python_ADDITIONAL_VERSIONS 3.4 3.5)
set(saved_prefix_path ${CMAKE_PREFIX_PATH})
set(CMAKE_PREFIX_PATH "$ENV{EXTERNLIBS}/python" ${CMAKE_PREFIX_PATH})
COVISE_FIND_PACKAGE(PythonLibs)
set(${CMAKE_PREFIX_PATH} ${saved_prefix_path})

IF(COVISE_EXPORT_TO_INSTALL)
   set(COVISE_EXPORTS_PATH "${COVISEDIR}/${ARCHSUFFIX}")
ELSE()
   set(COVISE_EXPORTS_PATH "${CMAKE_BINARY_DIR}")
ENDIF()
FILE(WRITE ${COVISE_EXPORTS_PATH}/${COVISE_EXPORTS_FILE} "")

# descend to sources
ADD_SUBDIRECTORY(src)

# docs
OPTION(COVISE_GENERATE_DOCS "Set to ON to generate the documentation with doxygen." OFF)
IF(COVISE_GENERATE_DOCS)
  COVISE_FIND_PACKAGE(Doxygen)
  CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/Doxyfile_Covise.in ${CMAKE_BINARY_DIR}/Doxyfile_Covise)
  ADD_CUSTOM_COMMAND(
      OUTPUT ${CMAKE_BINARY_DIR}/doc/html/index.html
      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/Doxyfile_Covise
      DEPENDS ${CMAKE_SOURCE_DIR}/Doxyfile_Covise.in
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
  ADD_CUSTOM_TARGET(Docs ALL DEPENDS ${CMAKE_BINARY_DIR}/doc/html/index.html)
  INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/doc DESTINATION ${CMAKE_INSTALL_PREFIX})
ENDIF(COVISE_GENERATE_DOCS)

# Install

SET(install_files_exclude "(/CVS/|/\\.svn/|/\\.hg/|/\\.git/|\\.swp$|\\.#|/#|/\\.gitignore|/\\.hgignore|\\.html\\.in$|\\.tex\\.in$|\\.tex$)")

#CONFIGURE_FILE("${COVISEDIR}/cmake/FindCovise.cmake" "${CMAKE_BINARY_DIR}/cmake/FindCovise.cmake" COPYONLY)
#CONFIGURE_FILE("${COVISEDIR}/cmake/FindZLIB.cmake" "${CMAKE_BINARY_DIR}/cmake/FindZLIB.cmake" COPYONLY)
CONFIGURE_FILE("${COVISEDIR}/cmake/CoviseHelperMacros.cmake" "${CMAKE_BINARY_DIR}/cmake/CoviseHelperMacros.cmake" COPYONLY)
INSTALL(DIRECTORY "${CMAKE_BINARY_DIR}/cmake" DESTINATION "${CMAKE_INSTALL_PREFIX}")

INSTALL(FILES .covise .covise.sh .covise-functions.sh .env .env.sh 
              README-3rd-party.txt README-ARCHSUFFIX.txt
              DESTINATION .)

INSTALL(FILES config/config.xml config/config-colormaps.xml
        DESTINATION config)

INSTALL(DIRECTORY share CgPrograms bin bitmaps icons materials scripts doc/pdf doc/html
        USE_SOURCE_PERMISSIONS
        DESTINATION .
        REGEX "${install_files_exclude}" EXCLUDE)

# in order to have an "uninstall" target
CONFIGURE_FILE(
  "${COVISEDIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

configure_file(
  "${COVISEDIR}/cmake/CoviseOptions.cmake.in"
  "${COVISE_EXPORTS_PATH}/CoviseOptions.cmake" @ONLY)

ADD_CUSTOM_TARGET(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

IF(${CMAKE_VERSION} VERSION_GREATER "2.8.2")
  INCLUDE(FeatureSummary)
  FEATURE_SUMMARY(WHAT ALL)
ENDIF()
