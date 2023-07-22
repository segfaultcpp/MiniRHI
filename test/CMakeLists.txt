add_subdirectory(Support)

if ("HelloTriangle" IN_LIST MINIRHI_TEST_EXAMPLES)
    add_subdirectory(HelloTriangle)
endif()

if ("Rendering3D" IN_LIST MINIRHI_TEST_EXAMPLES)
    add_subdirectory(Rendering3D)
endif()