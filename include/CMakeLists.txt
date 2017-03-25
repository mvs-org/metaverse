INSTALL(DIRECTORY "${PROJECT_SOURCE_DIR}/include/metaverse"
                  DESTINATION include
                  FILES_MATCHING
                  PATTERN "*.h"
                  PATTERN "*.hpp")
INSTALL(DIRECTORY "${PROJECT_SOURCE_DIR}/contrib/mongoose"
                  DESTINATION include/metaverse
                  FILES_MATCHING
                  PATTERN "*.h"
                  PATTERN "*.hpp")
INSTALL(DIRECTORY "${PROJECT_SOURCE_DIR}/contrib/json"
                  DESTINATION include/metaverse
                  FILES_MATCHING
                  PATTERN "*.h"
                  PATTERN "*.hpp")
