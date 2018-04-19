# Install script for directory: /home/avalon/catkin_ws/src/hector_gazebo/hector_gazebo_worlds

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/avalon/catkin_ws/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/avalon/catkin_ws/build/hector_gazebo/hector_gazebo_worlds/catkin_generated/installspace/hector_gazebo_worlds.pc")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/hector_gazebo_worlds/cmake" TYPE FILE FILES
    "/home/avalon/catkin_ws/build/hector_gazebo/hector_gazebo_worlds/catkin_generated/installspace/hector_gazebo_worldsConfig.cmake"
    "/home/avalon/catkin_ws/build/hector_gazebo/hector_gazebo_worlds/catkin_generated/installspace/hector_gazebo_worldsConfig-version.cmake"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/hector_gazebo_worlds" TYPE FILE FILES "/home/avalon/catkin_ws/src/hector_gazebo/hector_gazebo_worlds/package.xml")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/hector_gazebo_worlds" TYPE DIRECTORY FILES
    "/home/avalon/catkin_ws/src/hector_gazebo/hector_gazebo_worlds/launch"
    "/home/avalon/catkin_ws/src/hector_gazebo/hector_gazebo_worlds/maps"
    "/home/avalon/catkin_ws/src/hector_gazebo/hector_gazebo_worlds/Media"
    "/home/avalon/catkin_ws/src/hector_gazebo/hector_gazebo_worlds/worlds"
    )
endif()

