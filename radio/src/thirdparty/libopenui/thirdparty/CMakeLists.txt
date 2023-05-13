set(LVGL_SOURCES_MINIMAL
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

  draw/lv_draw_mask.c
  draw/lv_img_cache.c
  draw/sw/lv_draw_sw_gradient.c
  draw/sw/lv_draw_sw.c
  draw/sw/lv_draw_sw_blend.c
  draw/sw/lv_draw_sw_letter.c
  draw/sw/lv_draw_sw_arc.c
  draw/sw/lv_draw_sw_polygon.c
  draw/sw/lv_draw_sw_img.c
  draw/sw/lv_draw_sw_line.c
  draw/sw/lv_draw_sw_dither.c
  draw/sw/lv_draw_sw_rect.c
  draw/lv_draw_line.c
  draw/lv_draw.c
  draw/lv_img_buf.c
  draw/lv_draw_rect.c
  draw/lv_draw_label.c
  draw/lv_draw_img.c
  draw/lv_draw_arc.c
  draw/lv_draw_triangle.c
  draw/lv_img_decoder.c

  # STM32 specific
  draw/stm32_dma2d/lv_gpu_stm32_dma2d.c

  # SDL specific
  draw/sdl/lv_draw_sdl_rect.c
  draw/sdl/lv_draw_sdl_arc.c
  draw/sdl/lv_draw_sdl.c
  draw/sdl/lv_draw_sdl_mask.c
  draw/sdl/lv_draw_sdl_polygon.c
  draw/sdl/lv_draw_sdl_stack_blur.c
  draw/sdl/lv_draw_sdl_utils.c
  draw/sdl/lv_draw_sdl_img.c
  draw/sdl/lv_draw_sdl_bg.c
  draw/sdl/lv_draw_sdl_label.c
  draw/sdl/lv_draw_sdl_texture_cache.c
  draw/sdl/lv_draw_sdl_line.c
  draw/sdl/lv_draw_sdl_composite.c
  
  hal/lv_hal_indev.c
  hal/lv_hal_disp.c
  hal/lv_hal_tick.c

  font/lv_font.c
  font/lv_font_loader.c
  font/lv_font_fmt_txt.c
  
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
  misc/lv_lru.c
  misc/lv_tlsf.c
  misc/lv_anim_timeline.c
  misc/lv_ll.c
  misc/lv_bidi.c
  misc/lv_mem.c
  misc/lv_utils.c
  misc/lv_async.c
  misc/lv_anim.c
  misc/lv_txt_ap.c

  extra/lv_extra.c
)

set(LVGL_SOURCES
  ${LVGL_SOURCES_MINIMAL}
  # font/lv_font_unscii_8.c
  # font/lv_font_unscii_16.c
  # font/lv_font_simsun_16_cjk.c
  # font/lv_font_dejavu_16_persian_hebrew.c
  # font/lv_font_montserrat_8.c
  # font/lv_font_montserrat_10.c
  # font/lv_font_montserrat_12.c
  # font/lv_font_montserrat_12_subpx.c
  # font/lv_font_montserrat_14.c
  # font/lv_font_montserrat_16.c
  # font/lv_font_montserrat_18.c
  # font/lv_font_montserrat_20.c
  # font/lv_font_montserrat_22.c
  # font/lv_font_montserrat_24.c
  # font/lv_font_montserrat_26.c
  # font/lv_font_montserrat_28.c
  # font/lv_font_montserrat_28_compressed.c
  # font/lv_font_montserrat_30.c
  # font/lv_font_montserrat_32.c
  # font/lv_font_montserrat_34.c
  # font/lv_font_montserrat_36.c
  # font/lv_font_montserrat_38.c
  # font/lv_font_montserrat_40.c
  # font/lv_font_montserrat_42.c
  # font/lv_font_montserrat_44.c
  # font/lv_font_montserrat_46.c
  # font/lv_font_montserrat_48.c
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
  extra/others/monkey/lv_monkey.c
  extra/others/gridnav/lv_gridnav.c
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
  extra/widgets/imgbtn/lv_imgbtn.c
  extra/widgets/menu/lv_menu.c
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
  # extra/themes/basic/lv_theme_basic.c
  # extra/themes/default/lv_theme_default.c
  # extra/themes/mono/lv_theme_mono.c
)

foreach(LVGL_FILE ${LVGL_SOURCES_MINIMAL})
  set(LVGL_SRC_FILES_MINIMAL ${LVGL_SRC_FILES_MINIMAL} ${LVGL_SRC_DIR}/${LVGL_FILE})
endforeach()

foreach(LVGL_FILE ${LVGL_SOURCES})
  set(LVGL_SRC_FILES ${LVGL_SRC_FILES} ${LVGL_SRC_DIR}/${LVGL_FILE})
endforeach()

set(SRC ${SRC} ${LVGL_SRC_FILES})
