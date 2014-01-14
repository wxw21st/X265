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
    # this is horrible. ugly, and hacky, and it reproduces logic found
    # in the yasm CMake modules, but this is required because of this cmake bug
    # http://www.cmake.org/Bug/print_bug_page.php?bug_id=8170
    if (X64)
        set(FLAGS -f win64 -Xvc -m amd64 -DARCH_X86_64=1 -DHAVE_ALIGNED_STACK=0)
    else()
        set(FLAGS -f win32 -Xvc -DARCH_X86_64=0 -DHAVE_ALIGNED_STACK=0 -DPREFIX)
    endif()
    if (HIGH_BIT_DEPTH)
        set(FLAGS ${FLAGS} -DHIGH_BIT_DEPTH=1 -DBIT_DEPTH=10)
    else()
        set(FLAGS ${FLAGS} -DHIGH_BIT_DEPTH=0 -DBIT_DEPTH=8)
    endif()
    foreach(ASM ${ASMS})
        set(OBJS ${OBJS} ${ASM}.obj)
        add_custom_command(
            OUTPUT ${ASM}.obj
            COMMAND ${YASM_EXECUTABLE} ARGS ${FLAGS} ${CMAKE_CURRENT_SOURCE_DIR}/${ASM} -o ${ASM}.obj
            DEPENDS ${ASM})
    endforeach()
    add_library(ASM_LIB asm-export.cpp ${ASMS} ${OBJS})
else()
    enable_language(ASM_YASM)
    add_library(ASM_LIB asm-export.cpp ${ASMS})
endif()
