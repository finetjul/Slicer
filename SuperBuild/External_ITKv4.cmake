
# Make sure this file is included only once
get_filename_component(CMAKE_CURRENT_LIST_FILENAME ${CMAKE_CURRENT_LIST_FILE} NAME_WE)
if(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED)
  return()
endif()
set(${CMAKE_CURRENT_LIST_FILENAME}_FILE_INCLUDED 1)

# Sanity checks
if(DEFINED ITK_DIR AND NOT EXISTS ${ITK_DIR})
  message(FATAL_ERROR "ITK_DIR variable is defined but corresponds to non-existing directory")
endif()

# Set dependency list
set(ITKv4_DEPENDENCIES "zlib")
if(Slicer_BUILD_DICOM_SUPPORT)
  list(APPEND ITKv4_DEPENDENCIES DCMTK)
endif()

# Include dependent projects if any
SlicerMacroCheckExternalProjectDependency(ITKv4)
set(proj ITKv4)

if(NOT DEFINED ITK_DIR)
  #message(STATUS "${__indent}Adding project ${proj}")

  set(EXTERNAL_PROJECT_OPTIONAL_ARGS)

  # Set CMake OSX variable to pass down the external project
  if(APPLE)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
      -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
      -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
  endif()

  if(NOT CMAKE_CONFIGURATION_TYPES)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_ARGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE})
  endif()

  if(NOT DEFINED git_protocol)
      set(git_protocol "git")
  endif()

  set(ITKv4_REPOSITORY ${git_protocol}://itk.org/ITK.git)
  set(ITKv4_GIT_TAG ee6472628dcbb12d2723f04dbf3b14764de2b4ff)

  ExternalProject_Add(${proj}
    GIT_REPOSITORY ${ITKv4_REPOSITORY}
    GIT_TAG ${ITKv4_GIT_TAG}
    SOURCE_DIR ${proj}
    BINARY_DIR ${proj}-build
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DITK_INSTALL_ARCHIVE_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DITK_INSTALL_LIBRARY_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DBUILD_TESTING:BOOL=OFF
      -DBUILD_EXAMPLES:BOOL=OFF
      -DITK_LEGACY_REMOVE:BOOL=OFF
      -DITKV3_COMPATIBILITY:BOOL=ON
      -DITK_BUILD_ALL_MODULES:BOOL=ON
      -DModule_ITKReview:BOOL=ON
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DITK_INSTALL_NO_DEVELOPMENT:BOOL=ON
      -DITK_BUILD_ALL_MODULES:BOOL=ON
      -DKWSYS_USE_MD5:BOOL=ON # Required by SlicerExecutionModel
      -DITK_WRAPPING:BOOL=OFF #${BUILD_SHARED_LIBS} ## HACK:  QUICK CHANGE
      # DCMTK
      -DITK_USE_SYSTEM_DCMTK:BOOL=ON
      -DDCMTK_DIR:PATH=${DCMTK_DIR}
      -DModule_ITKIODCMTK:BOOL=${Slicer_BUILD_DICOM_SUPPORT}
      # ZLIB
      -DITK_USE_SYSTEM_ZLIB:BOOL=ON
      -DZLIB_ROOT:PATH=${SLICER_ZLIB_ROOT}
      -DZLIB_INCLUDE_DIR:PATH=${SLICER_ZLIB_INCLUDE_DIR}
      -DZLIB_LIBRARY:FILEPATH=${SLICER_ZLIB_LIBRARY}
      ${EXTERNAL_PROJECT_OPTIONAL_ARGS}
    INSTALL_COMMAND ""
    DEPENDS
      ${ITKv4_DEPENDENCIES}
    )
  set(ITK_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

else()
  # The project is provided using ITK_DIR, nevertheless since other project may depend on ITK,
  # let's add an 'empty' one
  SlicerMacroEmptyExternalProject(${proj} "${ITKv4_DEPENDENCIES}")
endif()
