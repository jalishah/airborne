

find_program(JAVA java)
set(SM_FILE "flight.sm")

get_filename_component(SM_BASE ${SM_FILE} NAME_WE)
set(SMC_GEN ${SM_BASE}_sm.py)

# add the custom command that will generate all the files
add_custom_command(
OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/_tmp/${SMC_GEN}
COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${SM_FILE} ${CMAKE_CURRENT_BINARY_DIR}/_tmp/${SM_FILE}
COMMAND ${JAVA} -jar $ENV{MOBICOM_PROJECT_PATH}/common/tools/smc/Smc.jar -python ${CMAKE_CURRENT_BINARY_DIR}/_tmp/${SM_FILE}
DEPENDS ${JAVA} ${SM_FILE}
)


ADD_CUSTOM_TARGET(flight_sm_py ALL DEPENDS ${SMC_GEN})

PYTHON_INSTALL(statemap.py ${CMAKE_CURRENT_BINARY_DIR})
PYTHON_INSTALL(${CMAKE_CURRENT_BINARY_DIR}/_tmp/${SMC_GEN} ${CMAKE_CURRENT_BINARY_DIR})

