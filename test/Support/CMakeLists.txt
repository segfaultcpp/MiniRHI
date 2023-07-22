set(MINIRHI_TEST_SUPPORT_SOURCES 
    App.cpp
)

add_library(MiniRHITestSupportLib ${MINIRHI_TEST_SUPPORT_SOURCES})
target_include_directories(MiniRHITestSupportLib PUBLIC "${CMAKE_SOURCE_DIR}/include"
    PRIVATE "${MINIRHI_3RDPARTY_DIR}/include")

if(${ANDROID_ENABLE})
    target_compile_options(MiniRHITestSupportLib PRIVATE -static -pedantic -DANDROID -fPIC)
    target_link_options(MiniRHITestSupportLib PRIVATE -s -static -static-libstdc++ -lc++_static -lc++abi)
else()
    if (WIN32)
        target_link_libraries(MiniRHITestSupportLib PRIVATE "${MINIRHI_3RDPARTY_DIR}/lib/glew/glew32.lib" PRIVATE OpenGL32.lib)
    elseif (UNIX)
        find_package(SDL2 REQUIRED)
        find_package(GLEW 2.0 REQUIRED)
        find_package(OpenGL REQUIRED)
        target_link_libraries(MiniRHITestSupportLib PRIVATE MiniRHILib
        PRIVATE ${GLEW_LIBRARIES} ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})
    endif()
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    target_compile_options(MiniRHITestSupportLib PRIVATE "$<$<CONFIG:Release>:-Werror -pedantic-errors>")
    target_compile_options(MiniRHITestSupportLib PRIVATE -pedantic -Wall -Wextra)
endif()
