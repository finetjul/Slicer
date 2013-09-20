################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) Kitware Inc.
#
#  See COPYRIGHT.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

#
# The CMake code used to find Qt4 has been factored out into this CMake script so that
# it can be used in both Slicer/CMakelists.txt and Slicer/UseSlicer.cmake
#

macro(__SlicerBlockFindQtAndCheckVersion_find_qt)
  set(qtproject)
  set(command_separated_module_list)

  if (Slicer_QT_VERSION VERSION_GREATER "4")
    foreach(module ${Slicer_REQUIRED_QT_MODULES})
      find_package(Qt5${module} REQUIRED)
      set(command_separated_module_list "${command_separated_module_list}${module}, ")
    endforeach()
    set(qtproject "Qt5Core")
  else()
    find_package(Qt4)
    set(qtproject "QT")
    if(NOT QT4_FOUND)
      message(FATAL_ERROR "error: Qt ${Slicer_REQUIRED_QT_VERSION} or Qt ${Slicer_EXPERIMENTAL_QT_VERSION} was not found on your system. You probably need to set the QT_QMAKE_EXECUTABLE variable.")
    endif()
  endif()

  # Check version
  if( NOT "${${qtproject}_VERSION}" VERSION_EQUAL "${Slicer_REQUIRED_QT_VERSION}"
      AND NOT "${QT_VERSION}" VERSION_EQUAL "${Slicer_EXPERIMENTAL_QT_VERSION}")
    message(FATAL_ERROR "error: Slicer requires Qt ${Slicer_REQUIRED_QT_VERSION} or Qt ${Slicer_EXPERIMENTAL_QT_VERSION}-- you cannot use Qt ${QT_VERSION}. ${extra_error_message}")
  endif()

  if (Slicer_QT_VERSION VERSION_EQUAL "4")
    # Check if all expected Qt modules have been discovered
    foreach(module ${Slicer_REQUIRED_QT_MODULES})
      string(TOUPPER ${module} module_upper)
      if(NOT "${QT_QT${module_upper}_FOUND}")
        message(FATAL_ERROR "error: Missing Qt module ${module}")
      endif()
      if(NOT ${module} STREQUAL "Core" AND NOT ${module} STREQUAL "Gui")
        set(QT_USE_QT${module_upper} ON)
      endif()
      set(command_separated_module_list "${command_separated_module_list}${module}, ")
    endforeach()
  endif()
  message(STATUS "Configured ${PROJECT_NAME} with Qt ${${qtproject}_VERSION} (using modules: ${command_separated_module_list})")
endmacro()

# Sanity checks - Check if variable are defined
set(expected_defined_vars
  Slicer_REQUIRED_QT_VERSION
  Slicer_REQUIRED_QT_MODULES
  )
foreach(v ${expected_defined_vars})
  if(NOT DEFINED ${v})
    message(FATAL_ERROR "error: ${v} CMake variable is not defined.")
  endif()
endforeach()

if (Slicer_QT_VERSION VERSION_EQUAL "4")
  # Check QT_QMAKE_EXECUTABLE provided by VTK
  set(extra_error_message)
  if(DEFINED VTK_QT_QMAKE_EXECUTABLE)
    #message(STATUS "Checking VTK_QT_QMAKE_EXECUTABLE ...")
    if(NOT EXISTS "${VTK_QT_QMAKE_EXECUTABLE}")
      message(FATAL_ERROR "error: You should probably re-configure VTK. VTK_QT_QMAKE_EXECUTABLE points to a nonexistent executable: ${VTK_QT_QMAKE_EXECUTABLE}")
    endif()
    set(QT_QMAKE_EXECUTABLE ${VTK_QT_QMAKE_EXECUTABLE})
    set(extra_error_message "You should probably reconfigure VTK.")
  endif()

  # Check QT_QMAKE_EXECUTABLE provided by CTK
  if(DEFINED CTK_QT_QMAKE_EXECUTABLE)
    #message(STATUS "Checking CTK_QT_QMAKE_EXECUTABLE ...")
    if(NOT EXISTS "${CTK_QT_QMAKE_EXECUTABLE}")
      message(FATAL_ERROR "error: You should probably re-configure CTK. CTK_QT_QMAKE_EXECUTABLE points to a nonexistent executable: ${CTK_QT_QMAKE_EXECUTABLE}")
    endif()
    set(QT_QMAKE_EXECUTABLE ${CTK_QT_QMAKE_EXECUTABLE})
    set(extra_error_message "You should probably reconfigure VTK.")
  endif()

endif()

__SlicerBlockFindQtAndCheckVersion_find_qt()

if (Slicer_QT_VERSION VERSION_EQUAL "4")
  include(${QT_USE_FILE})
endif()
