function (setLinker executable)

    macro (list_contains result request)
      set (${result})
      foreach (listItem ${ARGN})
        if (${request} STREQUAL ${listItem})
          set (${result} TRUE)
        endif ()
      endforeach ()
    endmacro()

    # Set the correct linker language
    get_property(_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
    list_contains(use_c C ${_languages})
    if (use_c)
        set(linker_language C)
    else ()
        list_contains(use_cxx CXX ${_languages})
        if (use_cxx)
            set(linker_language CXX)
        else ()
            message(FATAL_ERROR
                "PropWare requires at linking with C or CXX. Please enable at least one of those languages")
        endif ()
    endif ()

    SET_TARGET_PROPERTIES(${executable}
        PROPERTIES
        LINKER_LANGUAGE
        ${linker_language})

endfunction ()