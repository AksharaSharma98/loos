set(LOOS_USER_TOOLS
    model_calc
    simple_model_calc
    simple_model_transform
    traj_calc
    traj_transform
)

foreach(TOOL ${LOOS_USER_TOOLS})
    add_executable(${TOOL} ${TOOL}.cpp)
    target_link_libraries(${TOOL} loos)
    install(TARGETS ${TOOL})
endforeach()
