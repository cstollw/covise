
USING(ITK)

SET(HEADERS
  
)

SET(SOURCES
  Register.cpp
)

SET(EXTRASOURCES
  Register.h
)

ADD_COVISE_MODULE(Filter Register ${EXTRASOURCES} )
TARGET_LINK_LIBRARIES(Register coApi coAppl coCore ${ITK_LIBRARIES})

COVISE_INSTALL_TARGET(Register)
