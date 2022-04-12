cmake_minimum_required(VERSION 3.10)

project(kssroll LANGUAGES CXX C VERSION 0.5)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s -O3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s -O3 -Wall")


# Set a default build type if none was specified
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()



include(GNUInstallDirs)
message(STATUS "executable : " ${CMAKE_INSTALL_FULL_BINDIR} " " ${CMAKE_INSTALL_BINDIR})
message(STATUS "données : " ${CMAKE_INSTALL_FULL_DATADIR}/kssroll)
message(STATUS "locales : " ${CMAKE_INSTALL_FULL_LOCALEDIR}/fr/LC_MESSAGES/kssroll.mo)
message(STATUS "CMAKE_INSTALL_PREFIX : " ${CMAKE_INSTALL_PREFIX})
message(STATUS "CMAKE_STAGING_PREFIX : " ${CMAKE_STAGING_PREFIX})
message(STATUS "CMAKE_SYSTEM_PREFIX_PATH : " ${CMAKE_SYSTEM_PREFIX_PATH})


# config.hpp file
configure_file(
    config.hpp.in
    ${CMAKE_SOURCE_DIR}/src/config.hpp
)


set (SRC
# src/config.cpp
src/loader.cpp
src/kss_tools.cpp
src/ksswindow.cpp
src/resources.c
src/main.cpp
)

add_executable(${PROJECT_NAME}
    ${SRC} 
)


find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0 )


# threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

pkg_check_modules(MAJIMIX majimix_pa REQUIRED)
if(${MAJIMIX_FOUND})
    message(STATUS "MAJIMIX library found")
    message(STATUS "    include dir : ${MAJIMIX_INCLUDEDIR}")
    message(STATUS "           name : ${MAJIMIX_LIBRARIES}")
    message(STATUS "          files : ${MAJIMIX_LINK_LIBRARIES}")
    message(STATUS "        version : ${MAJIMIX_VERSION}")
    
    target_include_directories(${PROJECT_NAME} PRIVATE ${MAJIMIX_INCLUDEDIR})
endif()
# ]]





# libkss
add_subdirectory(dependencies)



target_link_options(kss PRIVATE "$<$<CONFIG:RELEASE>:${-s}>")
target_link_options(emu2149 PRIVATE "$<$<CONFIG:RELEASE>:${-s}>")
target_link_options(emu2212 PRIVATE "$<$<CONFIG:RELEASE>:${-s}>")
target_link_options(emu2413 PRIVATE "$<$<CONFIG:RELEASE>:${-s}>")
target_link_options(emu8950 PRIVATE "$<$<CONFIG:RELEASE>:${-s}>")
target_link_options(emu76489 PRIVATE "$<$<CONFIG:RELEASE>:${-s}>")
target_link_options(kmz80 PRIVATE "$<$<CONFIG:RELEASE>:${-s}>")




# Setting up Intl
find_package (Intl REQUIRED)
find_package(Gettext REQUIRED)
include_directories(${INTL_INCLUDE_DIRS})
link_directories(${INTL_LIBRARY_DIRS})



 

FIND_PROGRAM(GETTEXT_MSGFMT_EXECUTABLE msgfmt)

IF(NOT GETTEXT_MSGFMT_EXECUTABLE)
    MESSAGE("------
    NOTE: msgfmt not found. Translations will *not* be installed
------")
ELSE(NOT GETTEXT_MSGFMT_EXECUTABLE)

  SET(catalogname ${PROJECT_NAME})

  FILE(GLOB PO_FILES po/*.po)
  SET(GMO_FILES)

  FOREACH(_poFile ${PO_FILES})
    GET_FILENAME_COMPONENT(_poFileName ${_poFile} NAME)
    STRING(REGEX REPLACE "^${catalogname}_?" "" _langCode ${_poFileName} )
    STRING(REGEX REPLACE "\\.po$" "" _langCode ${_langCode} )

    IF( _langCode )
      GET_FILENAME_COMPONENT(_lang ${_poFile} NAME_WE)
      SET(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${_lang}.gmo)

      ADD_CUSTOM_COMMAND(OUTPUT ${_gmoFile}
        COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} --check -o ${_gmoFile} ${_poFile}
        DEPENDS ${_poFile})
      # INSTALL(FILES ${_gmoFile} DESTINATION ${LOCALE_INSTALL_DIR}/${_langCode}/LC_MESSAGES/ RENAME ${catalogname}.mo)
      INSTALL(FILES ${_gmoFile} DESTINATION ${CMAKE_INSTALL_FULL_LOCALEDIR}/${_langCode}/LC_MESSAGES/ RENAME ${catalogname}.mo)

      LIST(APPEND GMO_FILES ${_gmoFile})
    ENDIF( _langCode )

  ENDFOREACH(_poFile ${PO_FILES})

  ADD_CUSTOM_TARGET(translations ALL DEPENDS ${GMO_FILES})

ENDIF(NOT GETTEXT_MSGFMT_EXECUTABLE)





#message(${libkss_sources})
target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${GTKMM_INCLUDE_DIRS}
#    ${MAJIMIX_INCLUDE_DIRS}
    ${libkss_SOURCE_DIR}/src
    ${libkss_SOURCE_DIR}/modules
#    ${libkss_sources}
) 


target_link_libraries(${PROJECT_NAME} 
    ${GTKMM_LIBRARIES}
    ${PORTAUDIO_LIBRARIES}
    ${MAJIMIX_LIBRARIES}
#    ${MAJIMIX_LIB}
#    kss emu2149 emu2212 emu2413 emu8950 emu76489 kmz80
#    ${libkss_libs}
    Threads::Threads
)

install(TARGETS ${PROJECT_NAME} 
        RUNTIME
          DESTINATION ${CMAKE_INSTALL_BINDIR}
          COMPONENT  ${PROJECT_NAME} 
      )