
file (GLOB SOURCE_FILES *.cpp)
file (GLOB HEADER_FILES *.hpp)

init_target (vc_service)

set(LINK_DIR /home/ec2-user/vc_engine/vc_demo/build)
set(INC_DIR /home/ec2-user/vc_engine/vc_demo/include)


include_directories(${INC_DIR})
link_directories(${LINK_DIR})

target_link_libraries(vc_service "${TORCH_LIBRARIES}")
target_link_libraries(vc_service vc_engine)

build_executable (${TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES})

link_boost ()
final_target ()

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "examples")
