project(CIRCE)

set(IMGUI_SOURCES
        circe/imgui/imgui.cpp
        circe/imgui/imgui_demo.cpp
        circe/imgui/imgui_draw.cpp
        circe/imgui/imgui_widgets.cpp
        circe/imgui/ImGuiFileDialog.cpp
        circe/imgui/TextEditor.cpp
        )

set(IMGUI_HEADERS
        circe/imgui/imconfig.h
        circe/imgui/imgui.h
        circe/imgui/imgui_internal.h
        circe/imgui/imstb_rectpack.h
        circe/imgui/imstb_textedit.h
        circe/imgui/imstb_truetype.h
        circe/imgui/ImGuiFileDialog.h
        circe/imgui/ImGuiFileDialogConfig.h
        circe/imgui/TextEditor.h
        )

set(CIRCE_HEADERS
        circe/colors/color.h
        circe/colors/color_palette.h
        circe/common/bitmask_operators.h
        circe/gl/graphics/compute_shader.h
        circe/gl/graphics/post_effect.h
        circe/gl/graphics/shader.h
        circe/gl/graphics/shader_manager.h
        circe/gl/graphics/shadow_map.h
        circe/gl/helpers/bvh_model.h
        circe/gl/helpers/camera_model.h
        circe/gl/helpers/cartesian_grid.h
        circe/gl/helpers/geometry_drawers.h
        circe/gl/helpers/grid_model.h
        #        circe/helpers/hemesh_model.h
        circe/gl/helpers/quad_tree_model.h
        circe/gl/helpers/scene_handle.h
        circe/gl/helpers/vector_grid.h
        #        circe/helpers/tmesh_model.h
        # imgui
        circe/gl/imgui/imgui_impl_glfw.h
        circe/gl/imgui/imgui_impl_opengl3.h
        circe/gl/io/buffer.h
        circe/gl/io/display_renderer.h
        circe/gl/io/framebuffer.h
        circe/gl/io/graphics_display.h
        circe/gl/io/image_texture.h
        circe/gl/io/font_texture.h
        circe/gl/io/render_texture.h
        circe/gl/io/screen_quad.h
        circe/gl/io/storage_buffer.h
        circe/gl/io/texture.h
        circe/gl/io/texture_parameters.h
        circe/io/utils.h
        circe/gl/io/viewport_display.h
        circe/gl/io/user_input.h
        circe/gl/scene/bvh.h
        circe/gl/scene/scene_model.h
        circe/scene/camera_interface.h
        circe/scene/camera_projection.h
        circe/scene/light.h
        circe/scene/material.h
        circe/gl/scene/mesh_utils.h
        circe/scene/model.h
        circe/scene/shapes.h
        circe/gl/scene/quad.h
        circe/gl/scene/scene.h
        circe/gl/scene/scene_mesh.h
        circe/gl/scene/scene_object.h
        circe/gl/scene/triangle_mesh.h
        circe/gl/scene/volume_box.h
        circe/gl/scene/wireframe_mesh.h
        circe/gl/storage/buffer_interface.h
        circe/gl/storage/device_memory.h
        circe/gl/storage/index_buffer.h
        circe/gl/storage/vertex_array_object.h
        circe/gl/storage/uniform_buffer.h
        circe/gl/storage/vertex_buffer.h
        circe/gl/ui/app.h
        circe/gl/ui/interactive_object_interface.h
        circe/gl/ui/modifier_cursor.h
        circe/gl/ui/scene_app.h
        circe/gl/ui/text_renderer.h
        circe/gl/ui/text_object.h
        circe/gl/ui/font_manager.h
        circe/ui/trackball.h
        circe/ui/trackball_interface.h
        circe/ui/track_mode.h
        circe/ui/ui_camera.h
        circe/gl/utils/open_gl.h
        circe/gl/utils/win32_utils.h
        circe/gl/utils/base_app.h
        circe/circe.h
        circe/io.h
        )

set(CIRCE_SOURCES
        circe/gl/graphics/compute_shader.cpp
        circe/gl/graphics/post_effect.cpp
        circe/gl/graphics/shader.cpp
        circe/gl/graphics/shader_manager.cpp
        circe/gl/graphics/shadow_map.cpp
        circe/gl/helpers/cartesian_grid.cpp
        circe/gl/helpers/geometry_drawers.cpp
        # imgui
        circe/gl/imgui/imgui_impl_glfw.cpp
        circe/gl/imgui/imgui_impl_opengl3.cpp
        circe/gl/io/buffer.cpp
        circe/gl/io/display_renderer.cpp
        circe/gl/io/framebuffer.cpp
        circe/gl/io/graphics_display.cpp
        circe/gl/io/image_texture.cpp
        circe/gl/io/render_texture.cpp
        circe/gl/io/font_texture.cpp
        circe/gl/io/storage_buffer.cpp
        circe/gl/io/screen_quad.cpp
        circe/gl/io/texture.cpp
        circe/io/utils.cpp
        circe/gl/io/viewport_display.cpp
        circe/gl/scene/bvh.cpp
        circe/gl/scene/mesh_utils.cpp
        circe/scene/model.cpp
        circe/scene/shapes.cpp
        circe/gl/scene/quad.cpp
        circe/gl/scene/scene_mesh.cpp
        circe/gl/scene/scene_model.cpp
        circe/gl/scene/triangle_mesh.cpp
        circe/gl/scene/volume_box.cpp
        circe/gl/scene/wireframe_mesh.cpp
        circe/gl/storage/buffer_interface.cpp
        circe/gl/storage/device_memory.cpp
        circe/gl/storage/index_buffer.cpp
        circe/gl/storage/vertex_array_object.cpp
        circe/gl/storage/uniform_buffer.cpp
        circe/gl/storage/vertex_buffer.cpp
        circe/gl/ui/app.cpp
        circe/gl/ui/text_renderer.cpp
        circe/gl/ui/text_object.cpp
        circe/gl/ui/font_manager.cpp
        circe/ui/trackball_interface.cpp
        circe/ui/ui_camera.cpp
        circe/gl/utils/open_gl.cpp
        circe/gl/utils/base_app.cpp
        circe/circe.cpp
        circe/io.cpp
        circe/gl/scene/instance_set.cpp circe/gl/scene/instance_set.h)

add_library(circe STATIC ${CIRCE_SOURCES} ${CIRCE_HEADERS} ${IMGUI_HEADERS} ${IMGUI_SOURCES})
set_target_properties(circe PROPERTIES
        OUTPUT_NAME "circe"
        FOLDER "CIRCE")

target_compile_definitions(circe PUBLIC -DASSETS_PATH="${CIRCE_SOURCE_DIR}/examples/assets" -DSHADERS_PATH="${CIRCE_SOURCE_DIR}/examples/shaders")
target_include_directories(circe PUBLIC
        ${CIRCE_SOURCE_DIR}
        ${STB_INCLUDES}
        ${PLY_INCLUDES}
        ${TINYOBJ_INCLUDE_DIR}
        ${GLAD_INCLUDES}
        ${GLFW_INCLUDE_DIR}
        ${DIRENT_INCLUDE_DIR}
        )
target_link_libraries(circe PUBLIC ponos
        ${GLFW_LIBRARIES}
        ${PLY_LIBS} ${TINYOBJ_LIBRARIES} glad
        )
# if(glfw AND glad)
add_dependencies(circe glfw glad)
# endif(glfw AND glad)

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif (BUILD_EXAMPLES)
if (BUILD_TESTS)
    # add_subdirectory(tests)
endif (BUILD_TESTS)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/circe
        DESTINATION ${INSTALL_PATH}/include
        FILES_MATCHING REGEX "(.*\\.[inl|h])")

install(TARGETS circe #aergia_shared
        LIBRARY DESTINATION ${INSTALL_PATH}/lib
        ARCHIVE DESTINATION ${INSTALL_PATH}//lib
        )
