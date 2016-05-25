########################################################
# OAuth O2 static lib

ADD_DEFINITIONS(-Do2_WITH_QT5=0)
ADD_DEFINITIONS(-Do2_WITH_OAUTH1=1)
ADD_DEFINITIONS(-Do2_WITH_TWITTER=1)
ADD_SUBDIRECTORY(o2)

########################################################
# Method plugin

SET(AUTH_BASIC_SRCS
  qgsauthoauthmethod.cpp
  qgsauthoauthedit.cpp
)

SET(AUTH_BASIC_HDRS
  qgsauthoauthmethod.h
  qgsauthoauthedit.h
)

SET(AUTH_BASIC_MOC_HDRS
  qgsauthoauthmethod.h
  qgsauthoauthedit.h
)

SET(AUTH_BASIC_UIS qgsauthoauthedit.ui)

INCLUDE_DIRECTORIES (
  ../../core
  ../../core/auth
  ../../core/geometry
  o2/src
)
INCLUDE_DIRECTORIES (SYSTEM
  ${QCA_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES (
  ../../gui
  ../../gui/auth
  ${CMAKE_CURRENT_BINARY_DIR}
)

QT4_WRAP_UI (AUTH_BASIC_UIS_H ${AUTH_BASIC_UIS})

QT4_WRAP_CPP(AUTH_BASIC_MOC_SRCS ${AUTH_BASIC_MOC_HDRS})

ADD_LIBRARY (oauthauthmethod MODULE ${AUTH_BASIC_SRCS} ${AUTH_BASIC_HDRS} ${AUTH_BASIC_MOC_SRCS} ${AUTH_BASIC_UIS_H})

TARGET_LINK_LIBRARIES (oauthauthmethod
  qgis_core
  qgis_gui
  o2
)

INSTALL(TARGETS oauthauthmethod
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
