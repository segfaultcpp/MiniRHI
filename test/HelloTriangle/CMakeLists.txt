add_executable(MiniRHITestHelloTriangle main.cpp)
target_include_directories(MiniRHITestHelloTriangle PRIVATE "${CMAKE_SOURCE_DIR}/include"
    PRIVATE "${MINIRHI_3RDPARTY_DIR}/include"
    PRIVATE "${CMAKE_SOURCE_DIR}/test/Support")
if(WIN32)
target_link_libraries(MiniRHITestHelloTriangle 
    PRIVATE MiniRHILib
    PRIVATE "${MINIRHI_3RDPARTY_DIR}/lib/sdl/SDL2.lib"
    PRIVATE "${MINIRHI_3RDPARTY_DIR}/lib/sdl/SDL2main.lib"
<<<<<<< HEAD
    PRIVATE MiniRHITestSupportLib)
elseif(UNIX)
    find_package(GLEW 2.0 REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(SDL2 REQUIRED)
    target_link_libraries(MiniRHITestHelloTriangle 
        PRIVATE MiniRHILib
        PRIVATE ${GLEW_LIBRARIES} 
        PRIVATE ${SDL2_LIBRARIES}
        PRIVATE ${OPENGL_LIBRARIES}
        PRIVATE MiniRHITestSupportLib)
endif()
target_compile_definitions(MiniRHITestHelloTriangle PRIVATE STB_IMAGE_IMPLEMENTATION STBI_FAILURE_USERMSG)
=======
    PRIVATE MiniRHITestSupportLib)
>>>>>>> origin/main
