# - Try to find log4cplus
# Once done, this will define
#
#  log4cplus_FOUND - system has log4cplus
#  log4cplus_INCLUDE_DIRS - the log4cplus include directories
#  log4cplus_LIBRARIES - link these to use log4cplus

# Include dir
find_path(LOG4CPLUS_INC
  NAMES log4cplus/version.h
  HINTS "/usr/include"
        "/usr/include/log4cplus"
        "/usr/local/include"
        "/usr/local/include/log4cplus"
        "/opt/local/include"
        "/opt/local/include/log4cplus"
)

# Finally the library itself
find_library(LOG4CPLUS_LIB
  NAMES liblog4cplus.a
  HINTS "/usr/lib"
        "/usr/local/lib"
        "/opt/local/lib"
)