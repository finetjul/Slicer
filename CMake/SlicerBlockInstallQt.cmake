# -------------------------------------------------------------------------
# Find and install Qt
# -------------------------------------------------------------------------
set(QT_INSTALL_LIB_DIR ${Slicer_INSTALL_LIB_DIR})
if (WIN32)
  set(QT_INSTALL_LIB_DIR bin)
endif()

set(qtlibs ${Slicer_REQUIRED_QT_MODULES})
if(Slicer_QT_VERSION VERSION_EQUAL "4")
  set(qtlibs_upper)
  foreach(qtlib ${qtlibs})
    string(TOUPPER ${qtlib} qtlib_upper)
    list(APPEND qtlibs_upper "QT${qtlib_upper}")
  endforeach()
  set(qtlibs ${qtlibs_upper})

  list(FIND Slicer_REQUIRED_QT_MODULES WebKit  use_webkit)
  if( ${use_webkit} GREATER -1 )
    list(APPEND qtlibs Phonon)
  endif()
endif()

foreach(qtlib ${qtlibs})
  if (Slicer_QT_VERSION VERSION_GREATER "4")
    foreach(lib ${Qt5${qtlib}_LIBRARIES})
      get_target_property(path ${lib} IMPORTED_LOCATION_RELEASE)
      get_target_property(file ${lib} IMPORTED_SONAME_RELEASE)
      set(lib_path "${path}")
      message("INSTALL Qt5${qtlib} ${lib} -> ${lib_path}")
      if(EXISTS "${lib_path}")
        install(FILES ${lib_path} DESTINATION ${QT_INSTALL_LIB_DIR} COMPONENT Runtime)
      endif()
    endforeach()
  else()
    if(QT_${qtlib}_LIBRARY_RELEASE)
      if(APPLE)
        install(DIRECTORY "${QT_${qtlib}_LIBRARY_RELEASE}"
        DESTINATION ${QT_INSTALL_LIB_DIR} COMPONENT Runtime)
      elseif(UNIX)
        # Install .so and versioned .so.x.y
        get_filename_component(QT_LIB_DIR_tmp ${QT_${qtlib}_LIBRARY_RELEASE} PATH)
        get_filename_component(QT_LIB_NAME_tmp ${QT_${qtlib}_LIBRARY_RELEASE} NAME)
        install(DIRECTORY ${QT_LIB_DIR_tmp}/
          DESTINATION ${QT_INSTALL_LIB_DIR} COMPONENT Runtime
          FILES_MATCHING PATTERN "${QT_LIB_NAME_tmp}*"
          PATTERN "${QT_LIB_NAME_tmp}*.debug" EXCLUDE)
      elseif(WIN32)
        get_filename_component(QT_DLL_PATH_tmp ${QT_QMAKE_EXECUTABLE} PATH)
        set(qtlib_path "${QT_DLL_PATH_tmp}/${qtlib}${Slicer_QT_VERSION}.dll")
        if(EXISTS "${qtlib_path}")
          install(FILES ${qtlib_path} DESTINATION ${QT_INSTALL_LIB_DIR} COMPONENT Runtime)
        endif()
      endif()
    endif()
  endif()
endforeach()
