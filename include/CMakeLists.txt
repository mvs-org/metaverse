INSTALL(DIRECTORY "${PROJECT_SOURCE_DIR}/include/bitcoin"
                  DESTINATION include/mvs
                  FILES_MATCHING
                  PATTERN "*.h"
                  PATTERN "*.hpp" EXCLUDE)
INSTALL(DIRECTORY "${PROJECT_SOURCE_DIR}/include/zmq"
                  DESTINATION include/mvs
                  FILES_MATCHING
                  PATTERN "*.h"
                  PATTERN "*.hpp" EXCLUDE)
INSTALL(DIRECTORY "${PROJECT_SOURCE_DIR}/include/mvs"
                  DESTINATION include/mvs
                  FILES_MATCHING
                  PATTERN "*.h"
                  PATTERN "*.hpp" EXCLUDE)
INSTALL(DIRECTORY "${PROJECT_SOURCE_DIR}/contrib/mongoose"
                  DESTINATION include/mvs
                  FILES_MATCHING
                  PATTERN "*.h"
                  PATTERN "*.hpp" EXCLUDE)
INSTALL(DIRECTORY "${PROJECT_SOURCE_DIR}/contrib/json"
                  DESTINATION include/mvs
                  FILES_MATCHING
                  PATTERN "*.h"
                  PATTERN "*.hpp" EXCLUDE)
