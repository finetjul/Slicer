
#-----------------------------------------------------------------------------
set(MODULE_NAME GradientAnisotropicDiffusion)

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES ${ITK_LIBRARIES}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
