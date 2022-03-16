set(PROFILE_FILES
    profile_geometry
    profile_body
    profile_state
    profile_deserialisation
    profile_serialisation
    profile_barrier
)

foreach(PROFILE ${PROFILE_FILES})
    add_executable(${PROFILE} ${PROFILE}.cpp)
    target_link_libraries(${PROFILE} opera)
endforeach()

add_custom_target(profiles)
add_dependencies(profiles ${PROFILE_FILES})
