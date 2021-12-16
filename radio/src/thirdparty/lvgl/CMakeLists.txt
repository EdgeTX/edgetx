if(ESP_PLATFORM)

    file(GLOB_RECURSE SOURCES src/*.c)

    idf_build_get_property(LV_MICROPYTHON LV_MICROPYTHON)

    if (LV_MICROPYTHON)
        idf_component_register(SRCS ${SOURCES}
                               INCLUDE_DIRS . src ../
                               REQUIRES main)
    else()
        idf_component_register(SRCS ${SOURCES}
                               INCLUDE_DIRS . src ../)
    endif()

    target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DLV_CONF_INCLUDE_SIMPLE")

    if (CONFIG_LV_ATTRIBUTE_FAST_MEM_USE_IRAM)
        target_compile_definitions(${COMPONENT_LIB} PUBLIC "-DLV_ATTRIBUTE_FAST_MEM=IRAM_ATTR")
    endif()

elseif(ZEPHYR_BASE)

    if(CONFIG_LVGL)

        zephyr_include_directories(${ZEPHYR_BASE}/lib/gui/lvgl)

        target_include_directories(lvgl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

        zephyr_compile_definitions(LV_CONF_KCONFIG_EXTERNAL_INCLUDE=<autoconf.h>)

        zephyr_library()

        file(GLOB_RECURSE SOURCES src/*.c)
        zephyr_library_sources(${SOURCES})

    endif(CONFIG_LVGL)

else()


  #  file(GLOB_RECURSE LVGL_SOURCES src/*.c)
  #  file(GLOB_RECURSE LVGL_EXAMPLE_SOURCES examples/*.c)
#    file(GLOB_RECURSE LVGL_SOURCES RELATIVE ${CMAKE_CURRENT_LIST_DIR}/src/*.c)
#    file(GLOB_RECURSE LVGL_EXAMPLE_SOURCES RELATIVE ${CMAKE_CURRENT_LIST_DIR}/examples/*.c)
set(LVGL_SOURCES
gpu/lv_gpu_nxp_pxp_osa.c
gpu/lv_gpu_sdl.c
gpu/lv_gpu_nxp_vglite.c
gpu/lv_gpu_nxp_pxp.c
gpu/lv_gpu_stm32_dma2d.c
gpu/sdl/lv_gpu_sdl_draw_blend.c
gpu/sdl/lv_gpu_sdl_mask.c
gpu/sdl/lv_gpu_sdl_lru.c
gpu/sdl/lv_gpu_sdl_draw_rect.c
gpu/sdl/lv_gpu_sdl_draw_arc.c
gpu/sdl/lv_gpu_sdl_stack_blur.c
gpu/sdl/lv_gpu_sdl_draw_img.c
gpu/sdl/lv_gpu_sdl_utils.c
gpu/sdl/lv_gpu_sdl_draw_label.c
gpu/sdl/lv_gpu_sdl_draw_line.c
gpu/sdl/lv_gpu_sdl_texture_cache.c
core/lv_group.c
core/lv_obj.c
core/lv_event.c
core/lv_indev.c
core/lv_obj_scroll.c
core/lv_obj_draw.c
core/lv_disp.c
core/lv_obj_style.c
core/lv_theme.c
core/lv_obj_style_gen.c
core/lv_refr.c
core/lv_indev_scroll.c
core/lv_obj_class.c
core/lv_obj_tree.c
core/lv_obj_pos.c
font/lv_font_montserrat_20.c
font/lv_font_unscii_8.c
font/lv_font_montserrat_48.c
font/lv_font_montserrat_28.c
font/lv_font_montserrat_26.c
font/lv_font_montserrat_38.c
font/lv_font_montserrat_18.c
font/lv_font_montserrat_8.c
font/lv_font_montserrat_44.c
font/lv_font_montserrat_40.c
font/lv_font_dejavu_16_persian_hebrew.c
font/lv_font_montserrat_32.c
font/lv_font_montserrat_46.c
font/lv_font_montserrat_14.c
font/lv_font_montserrat_12_subpx.c
font/lv_font_montserrat_34.c
font/lv_font_montserrat_16.c
font/lv_font_montserrat_42.c
font/lv_font_montserrat_12.c
font/lv_font_simsun_16_cjk.c
font/lv_font_montserrat_30.c
font/lv_font_montserrat_22.c
font/lv_font_montserrat_10.c
font/lv_font_fmt_txt.c
font/lv_font_montserrat_24.c
font/lv_font_loader.c
font/lv_font_montserrat_28_compressed.c
font/lv_font.c
font/lv_font_montserrat_36.c
font/lv_font_unscii_16.c
misc/lv_timer.c
misc/lv_txt.c
misc/lv_gc.c
misc/lv_style_gen.c
misc/lv_templ.c
misc/lv_color.c
misc/lv_fs.c
misc/lv_printf.c
misc/lv_style.c
misc/lv_log.c
misc/lv_area.c
misc/lv_math.c
misc/lv_tlsf.c
misc/lv_anim_timeline.c
misc/lv_ll.c
misc/lv_bidi.c
misc/lv_mem.c
misc/lv_utils.c
misc/lv_async.c
misc/lv_anim.c
misc/lv_txt_ap.c
draw/lv_draw_mask.c
draw/lv_img_cache.c
draw/lv_draw_blend.c
draw/lv_draw_line.c
draw/lv_img_buf.c
draw/lv_draw_rect.c
draw/lv_draw_label.c
draw/lv_draw_img.c
draw/lv_draw_arc.c
draw/lv_draw_triangle.c
draw/lv_img_decoder.c
hal/lv_hal_indev.c
hal/lv_hal_disp.c
hal/lv_hal_tick.c
extra/others/snapshot/lv_snapshot.c
extra/layouts/grid/lv_grid.c
extra/layouts/flex/lv_flex.c
extra/libs/bmp/lv_bmp.c
extra/libs/ffmpeg/lv_ffmpeg.c
extra/libs/gif/lv_gif.c
extra/libs/gif/gifdec.c
extra/libs/freetype/lv_freetype.c
extra/libs/qrcode/lv_qrcode.c
extra/libs/qrcode/qrcodegen.c
extra/libs/sjpg/tjpgd.c
extra/libs/sjpg/lv_sjpg.c
extra/libs/fsdrv/lv_fs_win32.c
extra/libs/fsdrv/lv_fs_stdio.c
extra/libs/fsdrv/lv_fs_posix.c
extra/libs/fsdrv/lv_fs_fatfs.c
extra/libs/rlottie/lv_rlottie.c
extra/libs/png/lv_png.c
extra/libs/png/lodepng.c
extra/lv_extra.c
extra/widgets/imgbtn/lv_imgbtn.c
extra/widgets/win/lv_win.c
extra/widgets/colorwheel/lv_colorwheel.c
extra/widgets/list/lv_list.c
extra/widgets/chart/lv_chart.c
extra/widgets/calendar/lv_calendar_header_arrow.c
extra/widgets/calendar/lv_calendar.c
extra/widgets/calendar/lv_calendar_header_dropdown.c
extra/widgets/tileview/lv_tileview.c
extra/widgets/led/lv_led.c
extra/widgets/spinner/lv_spinner.c
extra/widgets/tabview/lv_tabview.c
extra/widgets/meter/lv_meter.c
extra/widgets/spinbox/lv_spinbox.c
extra/widgets/animimg/lv_animimg.c
extra/widgets/span/lv_span.c
extra/widgets/keyboard/lv_keyboard.c
extra/widgets/msgbox/lv_msgbox.c
extra/themes/basic/lv_theme_basic.c
extra/themes/default/lv_theme_default.c
extra/themes/mono/lv_theme_mono.c
widgets/lv_line.c
widgets/lv_dropdown.c
widgets/lv_objx_templ.c
widgets/lv_canvas.c
widgets/lv_switch.c
widgets/lv_btnmatrix.c
widgets/lv_bar.c
widgets/lv_roller.c
widgets/lv_img.c
widgets/lv_btn.c
widgets/lv_checkbox.c
widgets/lv_label.c
widgets/lv_table.c
widgets/lv_slider.c
widgets/lv_textarea.c
widgets/lv_arc.c
)
#message(STATUS "LVGL ${LVGL_SOURCES}")
#[[
    file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c)
    file(GLOB_RECURSE EXAMPLE_SOURCES ${CMAKE_CURRENT_LIST_DIR}/examples/*.c)
    if(MICROPY_DIR)
        # with micropython, build lvgl as interface library
        # link chain is: lvgl_interface [lvgl] → usermod_lvgl_bindings [lv_bindings] → usermod [micropython] → firmware [micropython]
        add_library(lvgl_interface INTERFACE)
        # ${SOURCES} must NOT be given to add_library directly for some reason (won't be built)
        target_sources(lvgl_interface INTERFACE ${SOURCES})
        # Micropython builds with -Werror; we need to suppress some warnings, such as:
        #
        # /home/test/build/lv_micropython/ports/rp2/build-PICO/lv_mp.c:29316:16: error: 'lv_style_transition_dsc_t_path_xcb_callback' defined but not used [-Werror=unused-function]
        # 29316 | STATIC int32_t lv_style_transition_dsc_t_path_xcb_callback(const struct _lv_anim_t * arg0)
        #       |                ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        target_compile_options(lvgl_interface INTERFACE -Wno-unused-function)
    else(MICROPY_DIR)
        # without micropython, build lvgl and examples libs normally
        # default linux build uses this scope
        add_library(lvgl STATIC ${SOURCES})
        add_library(lvgl_examples STATIC ${EXAMPLE_SOURCES})

        include_directories(${CMAKE_SOURCE_DIR})

        # Lbrary and headers can be installed to system using make install
        file(GLOB LVGL_PUBLIC_HEADERS
          "${CMAKE_SOURCE_DIR}/lv_conf.h"
          "${CMAKE_SOURCE_DIR}/lvgl.h")

        if("${LIB_INSTALL_DIR}" STREQUAL "")
            set(LIB_INSTALL_DIR "lib")
        endif()
        if("${INC_INSTALL_DIR}" STREQUAL "")
            set(INC_INSTALL_DIR "include/lvgl")
        endif()

        install(DIRECTORY "${CMAKE_SOURCE_DIR}/src"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/${INC_INSTALL_DIR}/"
            FILES_MATCHING
            PATTERN "*.h")

        set_target_properties(lvgl PROPERTIES
            OUTPUT_NAME lvgl
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            PUBLIC_HEADER "${LVGL_PUBLIC_HEADERS}")

        install(TARGETS lvgl
            ARCHIVE DESTINATION "${LIB_INSTALL_DIR}"
            PUBLIC_HEADER DESTINATION "${INC_INSTALL_DIR}")

    endif(MICROPY_DIR)
]]
endif()


