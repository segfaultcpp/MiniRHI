add_executable(MiniRHITest test.cpp)
target_include_directories(MiniRHITest PRIVATE "${CMAKE_SOURCE_DIR}/include"
    PRIVATE "${MINIRHI_3RDPARTY_DIR}/include")
target_link_libraries(MiniRHITest PRIVATE MiniRHILib
    PRIVATE "${MINIRHI_3RDPARTY_DIR}/lib/sdl/SDL2.lib"
    PRIVATE "${MINIRHI_3RDPARTY_DIR}/lib/sdl/SDL2main.lib")
target_compile_definitions(MiniRHITest PRIVATE STB_IMAGE_IMPLEMENTATION STBI_FAILURE_USERMSG)