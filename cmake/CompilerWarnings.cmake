function(solar_set_project_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /Zc:preprocessor)
    else()
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wshadow
            -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual
        )
    endif()
endfunction()

