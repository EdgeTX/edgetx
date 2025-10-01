if(WIN32)
    include(InstallRequiredSystemLibraries)

    # Determine architecture
    # if(CMAKE_CL_64)
        set(MSVC_ARCH x64)
    # else()
    #     set(MSVC_ARCH x86)
    # endif()

    # Attempt to find the vcredist executable
    find_program(MSVC_REDIST_EXE
        NAMES vc_redist.${MSVC_ARCH}.exe
        PATHS
            "${MSVC_REDIST_DIR}" # If set by InstallRequiredSystemLibraries
            "$ENV{VCINSTALLDIR}Redist/MSVC/v143/" # For VS 2022
            "C:/Program Files (x86)/Microsoft Visual Studio/*/VC/Redist/MSVC/*/"
            "C:/Program Files/Microsoft Visual Studio/*/*/VC/Redist/MSVC/*/"
        NO_DEFAULT_PATH
    )

    if(MSVC_REDIST_EXE)
        message(STATUS "Found VC Redistributable: ${MSVC_REDIST_EXE}")
    else()
        message(WARNING "VC Redistributable not found.")
    endif()
endif()
