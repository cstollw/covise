SET(HEADERS
  ReadPLUTO.h
)

SET(SOURCES
  ReadPLUTO.cpp
)

#SET(EXTRASOURCES
#  ReadKiva.h
#)

ADD_COVISE_MODULE(IO ReadPLUTO ${EXTRASOURCES} )
TARGET_LINK_LIBRARIES(ReadPLUTO  coApi coAppl coCore )

COVISE_INSTALL_TARGET(ReadPLUTO)
