include(FetchContent)

function(solar_setup_cspice)
    if(TARGET cspice::cspice)
        return()
    endif()

    if(WIN32)
        set(cspice_url
            "https://naif.jpl.nasa.gov/pub/naif/toolkit/C/PC_Windows_VisualC_64bit/packages/cspice.zip")
        set(cspice_hash
            "")
        set(cspice_library_name "cspice.lib")
    elseif(UNIX AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(cspice_url
            "https://naif.jpl.nasa.gov/pub/naif/toolkit/C/PC_Linux_GCC_64bit/packages/cspice.tar.Z")
        set(cspice_hash
            "SHA256=60a95b51a6472f1afe7e40d77ebdee43c12bb5b8823676ccc74692ddfede06ce")
        set(cspice_library_name "cspice.a")
    else()
        message(FATAL_ERROR "SOLAR currently supports CSPICE on 64-bit Linux and Windows")
    endif()

    if(cspice_hash)
        FetchContent_Declare(cspice URL "${cspice_url}" URL_HASH "${cspice_hash}"
                             DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    else()
        FetchContent_Declare(cspice URL "${cspice_url}" DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    endif()
    FetchContent_MakeAvailable(cspice)

    # CMake versions differ in whether extraction retains the archive's top-level
    # `cspice` directory. Accept both official package layouts.
    if(EXISTS "${cspice_SOURCE_DIR}/cspice/include/SpiceUsr.h")
        set(cspice_root "${cspice_SOURCE_DIR}/cspice")
    elseif(EXISTS "${cspice_SOURCE_DIR}/include/SpiceUsr.h")
        set(cspice_root "${cspice_SOURCE_DIR}")
    else()
        message(FATAL_ERROR "CSPICE package layout is invalid: SpiceUsr.h was not found")
    endif()

    add_library(cspice_toolkit STATIC IMPORTED GLOBAL)
    set_target_properties(cspice_toolkit PROPERTIES
        IMPORTED_LOCATION "${cspice_root}/lib/${cspice_library_name}"
        INTERFACE_INCLUDE_DIRECTORIES "${cspice_root}/include"
    )
    add_library(cspice::cspice ALIAS cspice_toolkit)
endfunction()
