PROJECT(DumpCFXPlugin)

# for dump_cfx, PLEASE ADAPT!
FIND_PATH(CFX_LIBRARY_PATH libmeshexport.a ${CFX_LIBRARY_PATH};/pub/flowvis/local/ansys-11/v110/CFX/lib/linux)
FIND_PATH(CFX_INCLUDE_PATH cfxExport.h     ${CFX_INCLUDE_PATH};/pub/flowvis/local/ansys-11/v110/CFX/include)

INCLUDE_DIRECTORIES(${CFX_INCLUDE_PATH})

SET(CFX_LINK_DIRS ${CFX_LIBRARY_PATH})

IF(WIN32)
  SET(CFX_LINK_DIRS ${CFX_LIBRARY_PATH};${CFX_LIBRARY_PATH}/MSWin32-x86-perlio/auto/CFX5/Units)
ENDIF(WIN32)

IF (NOT WIN32)
  SET(CFX_LIBS meshexport io)

  IF(EXISTS ${CFX_LIBRARY_PATH}/libcclapilt.a)
    SET(CFX_LIBS ${CFX_LIBS} cclapilt)
  ENDIF(EXISTS ${CFX_LIBRARY_PATH}/libcclapilt.a)

  IF(${CFX_INCLUDE_PATH} MATCHES "-10")
    SET(CFX_LIBS ${CFX_LIBS} pgtapi units)
  ENDIF(${CFX_INCLUDE_PATH} MATCHES "-10")

  IF(${CFX_INCLUDE_PATH} MATCHES "-11")
    SET(CFX_LIBS ${CFX_LIBS} pgtapi units)
  ENDIF(${CFX_INCLUDE_PATH} MATCHES "-11")

# added for cmake 2.6 and paraview 3.3
  IF(${CFX_INCLUDE_PATH} MATCHES "v11")
    SET(CFX_LIBS ${CFX_LIBS} pgtapi units)
  ENDIF(${CFX_INCLUDE_PATH} MATCHES "v11")

  IF(EXISTS ${CFX_LIBRARY_PATH}/libcclapilt.a)
    SET(CFX_LIBS ${CFX_LIBS} cclapilt)
  ENDIF(EXISTS ${CFX_LIBRARY_PATH}/libcclapilt.a)
ELSE (NOT WIN32)
  SET(CFX_LIBS libmeshexport libio libcclapilt libpgtapi units)
ENDIF (NOT WIN32)

LINK_DIRECTORIES(${LINK_DIRECTORIES} ${CFX_LINK_DIRS} ${CFX_LIBRARY_PATH}) 

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../../../modules/impl/dump_cfx)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../)

ADD_PARAVIEW_PLUGIN(
  UNIVIZ_DumpCFX_SMPlugin "1.0"
  SERVER_MANAGER_XML 
    ${CMAKE_CURRENT_SOURCE_DIR}/../DumpCFX_Server.xml
  SERVER_MANAGER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../vtkDumpCFX.cxx
  SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../impl/dump_cfx/cfx_export_lib.cpp
)

TARGET_LINK_LIBRARIES(
  UNIVIZ_DumpCFX_SMPlugin
  ${UNIVIZ_PLUGIN_LIBS}
  ${CFX_LIBS}
)
  
ADD_PARAVIEW_PLUGIN(
  UNIVIZ_DumpCFX_GUIPlugin
  "1.0"
  GUI_RESOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/DumpCFX.qrc 
)

