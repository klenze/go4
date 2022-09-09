# go4 version 6.2.99

set(GO4_MAJOR_VERSION 6)
set(GO4_MINOR_VERSION 2)
set(GO4_PATCH_VERSION 99)
set(GO4_VERSION ${GO4_MAJOR_VERSION}.${GO4_MINOR_VERSION}.${GO4_PATCH_VERSION})

if(APPLE)
   set(_platform Darwin)
elseif(MSVC)
   if("${CMAKE_GENERATOR_PLATFORM}" MATCHES "x64")
      set(_platform WIN64)
   else()
      set(_platform WIN32)
   endif()
else()
   set(GO4_WITH_RFIO 1)
   set(_platform Linux)
endif()

if(qt6)
   set(_qtvers Qt6)
else()
   set(_qtvers Qt5)
endif()

set(GO4_PLATFORM "${_platform}" CACHE STRING "Go4 platform definitions")
set(GO4_DEFINITIONS "" CACHE STRING "Go4 extra definitions")
set(GO4_CXX_FLAGS "" CACHE STRING "Go4 CXX flags")
set(GO4_C_FLAGS "" CACHE STRING "Go4 C flags")
set(GO4_QTVERSION "${_qtvers}" CACHE STRING "Go4 qt version")

set(GO4_ROOT_DIR $ENV{ROOTSYS})
if(GO4_ROOT_DIR)
   if(MSVC)
     string(REGEX REPLACE "\\\\" "/" GO4_ROOT_DIR ${GO4_ROOT_DIR})
   endif()
   set(SETTING_GO4_ROOT_DIR "set(GO4_ROOT_DIR \"${GO4_ROOT_DIR}\")")
   set(SETTINGS_ROOTSYS "##### ROOT-related settings ###########\n\
export ROOTSYS=${GO4_ROOT_DIR}\n\
export PATH=${GO4_ROOT_DIR}/bin:$PATH\n\
export LD_LIBRARY_PATH=${GO4_ROOT_DIR}/lib:$LD_LIBRARY_PATH\n\
export ROOT_INCLUDE_PATH=${CMAKE_BINARY_DIR}/include:${GO4_ROOT_DIR}/include:$ROOT_INCLUDE_PATH\n\
export PYTHONPATH=${GO4_ROOT_DIR}/lib:$PYTHONPATH\n\
export CMAKE_PREFIX_PATH=${GO4_ROOT_DIR}:$CMAKE_PREFIX_PATH")
else()
   set(SETTING_GO4_ROOT_DIR "")
   set(SETTINGS_ROOTSYS "")
endif()

configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/Go4Config.cmake.in
               ${CMAKE_BINARY_DIR}/Go4Config.cmake @ONLY NEWLINE_STYLE UNIX)

configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/Go4Config.cmake.in
               ${CMAKE_BINARY_DIR}/_install/Go4Config.cmake @ONLY NEWLINE_STYLE UNIX)

configure_file(${CMAKE_SOURCE_DIR}/cmake/modules/Go4Macros.cmake
               ${CMAKE_BINARY_DIR}/Go4Macros.cmake COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/Go4UseFile.cmake.in
               ${CMAKE_BINARY_DIR}/Go4UseFile.cmake @ONLY NEWLINE_STYLE UNIX)

install(FILES ${CMAKE_BINARY_DIR}/_install/Go4Config.cmake
              ${CMAKE_BINARY_DIR}/Go4Macros.cmake
              ${CMAKE_BINARY_DIR}/Go4UseFile.cmake
              DESTINATION ${GO4_INSTALL_MAINDIR}
              COMPONENT headers)

# ================== Copy support files for plain Makefile ==========

if (NOT MSVC)
   execute_process(COMMAND ${ROOT_BINDIR}/root-config --version OUTPUT_VARIABLE _root_vers_)
   execute_process(COMMAND ${ROOT_BINDIR}/root-config --exec-prefix OUTPUT_VARIABLE _root_exec_prefix_)
   execute_process(COMMAND ${ROOT_BINDIR}/root-config --cflags OUTPUT_VARIABLE _root_cflags_)
   execute_process(COMMAND ${ROOT_BINDIR}/root-config --libs OUTPUT_VARIABLE _root_libs_)

   set(_go4top_ ${CMAKE_BINARY_DIR})
   set(_go4bin_ ${CMAKE_BINARY_DIR}/bin)
   set(_go4lib_ ${CMAKE_BINARY_DIR}/lib)
   set(_go4inc_ ${CMAKE_BINARY_DIR}/include)

   configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/go4login.in
                  ${CMAKE_BINARY_DIR}/go4login @ONLY NEWLINE_STYLE UNIX)

   configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/Makefile.gener.in
                  ${CMAKE_BINARY_DIR}/build/Makefile.gener @ONLY NEWLINE_STYLE UNIX)

   set(_go4top_ ${GO4_INSTALL_FULL_MAINDIR})
   set(_go4bin_ ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
   set(_go4lib_ ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
   set(_go4inc_ ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})

   configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/go4login.in
                  ${CMAKE_BINARY_DIR}/_install/go4login @ONLY NEWLINE_STYLE UNIX)

   configure_file(${CMAKE_SOURCE_DIR}/cmake/scripts/Makefile.gener.in
                  ${CMAKE_BINARY_DIR}/_install/Makefile.gener @ONLY NEWLINE_STYLE UNIX)

   file(COPY ${CMAKE_SOURCE_DIR}/Makefile.config DESTINATION ${CMAKE_BINARY_DIR})
   file(COPY ${CMAKE_SOURCE_DIR}/Makefile.rules DESTINATION ${CMAKE_BINARY_DIR})
   file(COPY ${CMAKE_SOURCE_DIR}/build/cleanlib.sh DESTINATION ${CMAKE_BINARY_DIR}/build)
   file(COPY ${CMAKE_SOURCE_DIR}/build/cxx.sh DESTINATION ${CMAKE_BINARY_DIR}/build)
   file(COPY ${CMAKE_SOURCE_DIR}/build/depend.sh DESTINATION ${CMAKE_BINARY_DIR}/build)
   file(COPY ${CMAKE_SOURCE_DIR}/build/makedict.sh DESTINATION ${CMAKE_BINARY_DIR}/build)
   file(COPY ${CMAKE_SOURCE_DIR}/build/makelib6.sh DESTINATION ${CMAKE_BINARY_DIR}/build)
   file(COPY ${CMAKE_SOURCE_DIR}/build/makemap.sh DESTINATION ${CMAKE_BINARY_DIR}/build)
   file(COPY ${CMAKE_SOURCE_DIR}/build/Makefile.Linux DESTINATION ${CMAKE_BINARY_DIR}/build)
   file(COPY ${CMAKE_SOURCE_DIR}/build/Makefile.Darwin DESTINATION ${CMAKE_BINARY_DIR}/build)

   install(FILES ${CMAKE_BINARY_DIR}/Makefile.config ${CMAKE_BINARY_DIR}/Makefile.rules DESTINATION ${GO4_INSTALL_MAINDIR} COMPONENT headers)
   install(FILES ${CMAKE_BINARY_DIR}/_install/Makefile.gener DESTINATION ${GO4_INSTALL_BUILDDIR} COMPONENT headers)
   install(DIRECTORY ${CMAKE_BINARY_DIR}/build/ DESTINATION ${GO4_INSTALL_BUILDDIR} COMPONENT headers)
   install(DIRECTORY ${CMAKE_BINARY_DIR}/build/ DESTINATION ${GO4_INSTALL_BUILDDIR}  COMPONENT headers FILES_MATCHING PATTERN "*.sh" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
   install(FILES ${CMAKE_BINARY_DIR}/_install/go4login DESTINATION ${GO4_INSTALL_MAINDIR} COMPONENT applications)
endif()


# search for extra ROOT libraries
foreach(_cpt Gui XMLIO RHTTP RHTTPSniff Ged Spectrum ASImage Minuit ROOTWebDisplay ROOTQt5WebDisplay WebGui6)
  find_library(ROOT_${_cpt}_LIBRARY ${_cpt} HINTS ${ROOT_LIBRARY_DIR})
  if(ROOT_${_cpt}_LIBRARY)
    mark_as_advanced(ROOT_${_cpt}_LIBRARY)
  endif()
endforeach()

