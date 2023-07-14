set(MINIRHI_SOURCES 
    Buffer.cpp 
    Format.cpp 
    MiniRHI.cpp 
    RenderCommands.cpp 
    Shader.cpp 
    Texture.cpp
    )
    
set(MINIRHI_BUILD_FILES_TEST_SOURCES dummy.cpp)
set(GLEW_USE_STATIC_LIBS OFF)
add_library(MiniRHILib ${MINIRHI_SOURCES})
target_include_directories(MiniRHILib PUBLIC "${CMAKE_SOURCE_DIR}/include"
    PRIVATE "${MINIRHI_3RDPARTY_DIR}/include")

if(${ANDROID_ENABLE})
    target_compile_options(MiniRHILib PRIVATE -static -pedantic -DANDROID -fPIC)
    target_link_options(MiniRHILib PRIVATE -s -static -static-libstdc++ -lc++_static -lc++abi)
else()
    target_link_libraries(MiniRHILib PRIVATE "${MINIRHI_3RDPARTY_DIR}/lib/glew/glew32.lib" PRIVATE OpenGL32.lib)
endif()

target_compile_options(MiniRHILib PRIVATE "$<$<CONFIG:Release>:-Werror -pedantic-errors>")

if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    target_compile_options(MiniRHILib PRIVATE -pedantic -Wall -Wextra)
endif()