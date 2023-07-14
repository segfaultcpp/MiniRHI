add_executable(MiniRHITestHelloTriangle main.cpp)
target_include_directories(MiniRHITestHelloTriangle PRIVATE "${CMAKE_SOURCE_DIR}/include"
    PRIVATE "${MINIRHI_3RDPARTY_DIR}/include"
    PRIVATE "${CMAKE_SOURCE_DIR}/test/Support")
target_link_libraries(MiniRHITestHelloTriangle PRIVATE MiniRHILib
    PRIVATE "${MINIRHI_3RDPARTY_DIR}/lib/sdl/SDL2.lib"
    PRIVATE "${MINIRHI_3RDPARTY_DIR}/lib/sdl/SDL2main.lib"
    PRIVATE MiniRHITestSupportLib)
target_compile_definitions(MiniRHITestHelloTriangle PRIVATE STB_IMAGE_IMPLEMENTATION STBI_FAILURE_USERMSG)