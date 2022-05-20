cmake_minimum_required (VERSION 3.16)

# Add source to this project's executable.
file(GLOB SRC CONFIGURE_DEPENDS *.cpp)
file(GLOB INC ../inc/*.h ../inc/*.hpp ../interface/*.h)
file(GLOB INL ../inc/*.inl *.inl)

add_library (CMPlantuml SHARED ${SRC} ${INC} ${INL} CMPlantuml.def)
target_include_directories(CMPlantuml
    PRIVATE ../inc
    PRIVATE ../interface
    PRIVATE ./)

target_precompile_headers(CMPlantuml PUBLIC ../inc/pch.h)

source_group(Inlines FILES ${INL})

#Copy plugin dll to the Plugin folder
add_custom_command(
    TARGET CMPlantuml 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:CMPlantuml>
        %APPDATA%/ColumnMode/Plugins/$<TARGET_FILE_BASE_NAME:CMPlantuml>/$<TARGET_FILE_NAME:CMPlantuml>
)