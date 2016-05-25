########################################################
# Files

SET (OAUTH_SRCS
  qgs_oauth_plugin.cpp
  qgs_oauth_widget.cpp
)

SET (OAUTH_UIS
  qgs_oauth_widget.ui
)

SET (OAUTH_MOC_HDRS
  qgs_oauth_plugin.h
  qgs_oauth_widget.h
)

########################################################
# Build

QT4_WRAP_UI (OAUTH_UIS_H  ${OAUTH_UIS})

QT4_WRAP_CPP (OAUTH_MOC_SRCS  ${OAUTH_MOC_HDRS})

ADD_LIBRARY (oauthtestplugin MODULE
  ${OAUTH_SRCS}
  ${OAUTH_MOC_SRCS}
  ${OAUTH_UIS_H}
)

INCLUDE_DIRECTORIES(
     ${CMAKE_CURRENT_BINARY_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}/../../ui
     ../../core
     ../../core/auth
     ../../core/geometry
     ../../gui
     ../../gui/auth
     ..
     .
     ${QCA_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(oauthtestplugin
  qgis_core
  qgis_gui
)


########################################################
# Install

INSTALL(TARGETS oauthtestplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
