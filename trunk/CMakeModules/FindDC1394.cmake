FIND_PATH( DC1394_INCLUDE_DIR dc1394/dc1394.h
  /usr/include
  /usr/local/include
)

FIND_LIBRARY( DC1394_LIBRARIES dc1394
  /usr/lib64
  /usr/lib
  /usr/local/lib
)

SET( DC1394_FOUND "NO" )
IF(DC1394_INCLUDE_DIR)
  IF(DC1394_LIBRARIES)
    SET( DC1394_FOUND "YES" )

#The following deprecated settings are for backwards compatibility with CMake1.4
    SET (DC1394_INCLUDE_PATH ${DC1394_INCLUDE_DIR})

  ENDIF(DC1394_LIBRARIES)
ENDIF(DC1394_INCLUDE_DIR)

