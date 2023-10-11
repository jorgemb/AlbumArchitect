install(
    TARGETS AlbumArchitect_exe
    RUNTIME COMPONENT AlbumArchitect_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
