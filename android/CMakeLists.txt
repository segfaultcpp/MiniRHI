set(ANDROIDAPP_SRC
        glue.cpp
        main.cpp
)

add_library(AndroidApp SHARED ${ANDROIDAPP_SRC})
target_compile_options(AndroidApp PRIVATE -static -fPIC -DANDROIDVERSION=29 -m64 -DANDROID -DAPPNAME=minirhi_test -fvisibility=hidden)
target_include_directories(AndroidApp PUBLIC "${CMAKE_SOURCE_DIR}/include" PRIVATE "${MINIRHI_3RDPARTY_DIR}/include")
target_link_libraries(AndroidApp PRIVATE android GLESv3 m log EGL m)
target_link_libraries(AndroidApp PRIVATE MiniRHILib)
target_link_options(AndroidApp PRIVATE -lc++_static -lc++abi -Wl,--gc-sections -s -uANativeActivity_onCreate)

add_custom_target(MiniRHIApk_Cleanup
        COMMAND ${CMAKE_COMMAND} -E rm -f minirhi_app.apk
        COMMAND ${CMAKE_COMMAND} -E rm -f minirhi_app.unsig.apk
        COMMAND ${CMAKE_COMMAND} -E rm -rf apk_extracted/
        COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_SOURCE_DIR}/android/apk/lib/arm64-v8a/lib$<TARGET_FILE_BASE_NAME:AndroidApp>.so
        VERBATIM)

add_custom_target(MiniRHIApk
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/android/apk apk/
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:AndroidApp> apk/lib/arm64-v8a/lib$<TARGET_FILE_BASE_NAME:AndroidApp>.so
        COMMAND cd apk
        COMMAND wsl.exe -d Ubuntu-22.04 --exec zip -r ../minirhi_app.unsig.apk .
        COMMAND cd ..
        COMMAND wsl.exe -d Ubuntu-22.04 --exec /bin/bash /home/dark/aa.sh
        DEPENDS "$<TARGET_FILE:AndroidApp>" MiniRHIApk_Cleanup
        VERBATIM)
