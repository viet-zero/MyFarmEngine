# Isolated, additive packaging test: never rename/delete original game assets.
string(RANDOM LENGTH 12 ALPHABET 0123456789abcdef nonce)
set(folder "${BUILD}/fallback_case_${nonce}")
file(MAKE_DIRECTORY "${folder}/Assets/Shaders" "${folder}/Assets/Data")
file(COPY "${GAME}" DESTINATION "${folder}")
get_filename_component(game_name "${GAME}" NAME)
file(COPY "${SOURCE}/Assets/Shaders/" DESTINATION "${folder}/Assets/Shaders")
file(COPY "${SOURCE}/Gameplay/Data/Assets.csv" DESTINATION "${folder}/Assets/Data")
execute_process(COMMAND "${folder}/${game_name}" --smoke-test
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors TIMEOUT 15)
if(NOT status EQUAL 0 OR NOT errors MATCHES "Asset fallback \\[player_linh\\]")
    message(FATAL_ERROR "Missing exports did not fall back: ${status}\n${output}\n${errors}")
endif()

# A second new directory has no manifest at all.
set(empty "${BUILD}/fallback_empty_${nonce}")
file(MAKE_DIRECTORY "${empty}/Assets/Shaders")
file(COPY "${GAME}" DESTINATION "${empty}")
file(COPY "${SOURCE}/Assets/Shaders/" DESTINATION "${empty}/Assets/Shaders")
execute_process(COMMAND "${empty}/${game_name}" --smoke-test
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors TIMEOUT 15)
if(NOT status EQUAL 0 OR NOT errors MATCHES "manifest_missing")
    message(FATAL_ERROR "Missing manifest did not fall back: ${status}\n${output}\n${errors}")
endif()

# One file exists but its declared grid is invalid. It must not activate.
file(MAKE_DIRECTORY "${folder}/Assets/Export/Art/Characters")
file(COPY "${SOURCE}/Assets/Export/Art/Characters/farmer_linh.png" DESTINATION "${folder}/Assets/Export/Art/Characters")
file(WRITE "${folder}/Assets/Data/Assets.csv"
    "asset_id,kind,source,export_path,columns,rows,frame_width,frame_height,anchor_x,anchor_y,has_alpha,status,first_frame,frame_count,margin_x,margin_y,spacing_x,spacing_y\nplayer_linh,character,Original/Art/Characters/farmer_linh.png,Export/Art/Characters/farmer_linh.png,2,1,1254,1254,0.5,1,true,ready,0,1,0,0,0,0\n")
execute_process(COMMAND "${folder}/${game_name}" --smoke-test
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors TIMEOUT 15)
if(NOT status EQUAL 0 OR NOT errors MATCHES "grid_dimensions_mismatch")
    message(FATAL_ERROR "Invalid grid did not fall back: ${status}\n${output}\n${errors}")
endif()
# A corrupt PNG with otherwise valid metadata must also use the fallback.
file(COPY "${SOURCE}/Gameplay/Data/Assets.csv" DESTINATION "${folder}/Assets/Data")
file(WRITE "${folder}/Assets/Export/Art/Characters/farmer_linh.png" "invalid_png_fixture")
execute_process(COMMAND "${folder}/${game_name}" --smoke-test
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE errors TIMEOUT 15)
if(NOT status EQUAL 0 OR NOT errors MATCHES "not_png")
    message(FATAL_ERROR "Corrupt PNG did not fall back: ${status}\n${output}\n${errors}")
endif()
message(STATUS "Missing/corrupt PNG, missing manifest and invalid grid all fell back safely")
