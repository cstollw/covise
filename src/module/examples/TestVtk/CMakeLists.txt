USE_VTK()
SET(HEADERS
  TestVtk.h
)
SET(SOURCES
  TestVtk.cpp
)
covise_add_module(Examples TestVtk ${EXTRASOURCES} ${SOURCES} ${HEADERS})
