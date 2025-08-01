#############################################################################
# Name:        build/cmake/main.cmake
# Purpose:     Main CMake file
# Author:      Tobias Taschner
# Created:     2016-09-20
# Copyright:   (c) 2016 wxWidgets development team
# Licence:     wxWindows licence
#############################################################################

list(APPEND CMAKE_MODULE_PATH "${wxSOURCE_DIR}/build/cmake/modules")

file(TO_CMAKE_PATH "${CMAKE_INSTALL_PREFIX}" CMAKE_INSTALL_PREFIX)

include(build/cmake/files.cmake)            # Files list
include(build/cmake/source_groups.cmake)    # Source group definitions
include(build/cmake/functions.cmake)        # wxWidgets functions
include(build/cmake/toolkit.cmake)          # Platform/toolkit settings
include(build/cmake/options.cmake)          # User options
include(build/cmake/init.cmake)             # Init various global build vars
include(build/cmake/pch.cmake)              # Precompiled header support

add_subdirectory(build/cmake/locale locale)
add_subdirectory(build/cmake/lib libs)
add_subdirectory(build/cmake/utils utils)

if(wxBUILD_SAMPLES)
    add_subdirectory(build/cmake/samples samples)
endif()

if(wxBUILD_TESTS)
    enable_testing()
    add_subdirectory(build/cmake/tests tests)
endif()

if(wxBUILD_DEMOS)
    add_subdirectory(build/cmake/demos demos)
endif()

if(wxBUILD_BENCHMARKS)
    add_subdirectory(build/cmake/benchmarks benchmarks)
endif()

if(NOT wxBUILD_CUSTOM_SETUP_HEADER_PATH)
    # Write setup.h after all variables are available
    include(build/cmake/setup.cmake)
endif()

if(WIN32_MSVC_NAMING)
    include(build/cmake/build_cfg.cmake)
endif()

if(NOT MSVC)
    # Write wx-config
    include(build/cmake/config.cmake)
endif()

# Install target support
include(build/cmake/install.cmake)

# Determine minimum required OS at runtime
set(wxREQUIRED_OS_DESC "${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
if(MSVC OR MINGW OR CYGWIN)
    set(wxREQUIRED_OS_DESC "Windows 7 / Windows Server 2008")
    if(wxPLATFORM_ARCH)
        wx_string_append(wxREQUIRED_OS_DESC " (${wxPLATFORM_ARCH} Edition)")
    endif()
elseif(APPLE AND NOT IPHONE)
    if(DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
        set(wxREQUIRED_OS_DESC "macOS ${CMAKE_OSX_DEPLOYMENT_TARGET} ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
endif()

# Print configuration summary
wx_print_thirdparty_library_summary()

# Avoid printing out the message if we're being reconfigured and nothing has
# changed since the previous run, so check if the current summary differs from
# the cached value.
set(wxSUMMARY_NOW
    "${CMAKE_SYSTEM_NAME}-${wxVERSION}-${wxREQUIRED_OS_DESC}-"
    "${wxBUILD_TOOLKIT}-${wxTOOLKIT_VERSION}-${wxTOOLKIT_EXTRA}-"
    "${wxBUILD_MONOLITHIC}-${wxBUILD_SHARED}-${wxBUILD_COMPATIBILITY}-"
)
if("${wxSUMMARY_NOW}" STREQUAL "${wxSUMMARY}")
  return()
endif()

set(wxSUMMARY ${wxSUMMARY_NOW} CACHE INTERNAL "internal summary of wxWidgets build options")

if(wxTOOLKIT_EXTRA)
    string(REPLACE ";" ", " wxTOOLKIT_DESC "${wxTOOLKIT_EXTRA}")
    set(wxTOOLKIT_DESC "with support for: ${wxTOOLKIT_DESC}")
endif()

message(STATUS "Configured wxWidgets ${wxVERSION} for ${CMAKE_SYSTEM_NAME}
    Min OS Version required at runtime:                ${wxREQUIRED_OS_DESC}
    Which GUI toolkit should wxWidgets use?            ${wxBUILD_TOOLKIT} ${wxTOOLKIT_VERSION} ${wxTOOLKIT_DESC}
    Should wxWidgets be compiled into single library?  ${wxBUILD_MONOLITHIC}
    Should wxWidgets be linked as a shared library?    ${wxBUILD_SHARED}
    Which wxWidgets API compatibility should be used?  ${wxBUILD_COMPATIBILITY}"
    )
