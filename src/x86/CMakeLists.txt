if (GCC)
    add_definitions(-DHAVE_ALIGNED_STACK=1)
    add_definitions(-Wno-error=unused-parameter)
else()
    add_definitions(-DHAVE_ALIGNED_STACK=0)
endif()

set(ASMS pixel-a.asm const-a.asm cpu-a.asm sad-a.asm sad16-a.asm)
if (X64)
    add_definitions(-DARCH_X86_64=1)
else()
    add_definitions(-DARCH_X86_64=0)
    set(ASMS ${ASMS} pixel-32.asm)
endif()

source_group(SRC FILES ${ASMS})

if(MSVC)
    # this is required because of this cmake bug
    # http://www.cmake.org/Bug/print_bug_page.php?bug_id=8170
    if(WIN32)
        set(SUFFIX obj)
    else()
        set(SUFFIX o)
    endif()

    # compile X86 arch asm files here
        foreach(ASM ${ASMS})
            set(ASM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/${ASM})
            list(APPEND ASM_SRCS ${ASM_SRC})
            list(APPEND ASM_OBJS ${ASM}.${SUFFIX})
            add_custom_command(
                OUTPUT ${ASM}.${SUFFIX}
                COMMAND ${NASM_EXECUTABLE} ARGS ${NASM_FLAGS} ${ASM_SRC} -o ${ASM}.${SUFFIX}
                DEPENDS ${ASM_SRC})
        endforeach()
    add_library(ASM_LIB asm-export.cpp ${ASM_OBJS})
else()
    enable_language(ASM_NASM)
    add_library(ASM_LIB asm-export.cpp ${ASMS})
endif()
source_group(ASM FILES ${ASM_SRCS})

