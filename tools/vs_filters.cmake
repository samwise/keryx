cmake_minimum_required(VERSION 3.5)

#add protos to the library to facilitate source_group
target_sources(libspark PUBLIC
  ${Config_Protos}
  ${NonConfig_Protos}
  ${Service_Protos}  
)

function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
		set(SOURCE_FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

assign_source_group(${SparkLib_Sources} ${Config_Protos} ${NonConfig_Protos} ${Service_Protos})
