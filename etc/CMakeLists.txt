SET(etc_FILES
  mvs.conf
  index.html
)

INSTALL(FILES ${etc_FILES} DESTINATION bin)
INSTALL(FILES ${etc_FILES} DESTINATION etc)
