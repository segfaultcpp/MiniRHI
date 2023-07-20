add_executable(MiniRHITestRendering3D main.cpp)
target_include_directories(MiniRHITestRendering3D PRIVATE "${CMAKE_SOURCE_DIR}/include"
    PRIVATE "${MINIRHI_3RDPARTY_DIR}/include"
    PRIVATE "${CMAKE_SOURCE_DIR}/test/Support")
target_link_libraries(MiniRHITestRendering3D PRIVATE MiniRHILib
    PRIVATE "${MINIRHI_3RDPARTY_DIR}/lib/sdl/SDL2.lib"
    PRIVATE "${MINIRHI_3RDPARTY_DIR}/lib/sdl/SDL2main.lib"
    PRIVATE MiniRHITestSupportLib)
target_compile_definitions(MiniRHITestRendering3D PRIVATE STB_IMAGE_IMPLEMENTATION STBI_FAILURE_USERMSG)