/**
 * @file lv_theme.h
 *
 */

#ifndef ETX_LV_THEME_H
#define ETX_LV_THEME_H

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the theme
 * @param color_primary the primary color of the theme
 * @param color_secondary the secondary color for the theme
 * @param font pointer to a font to use.
 * @return a pointer to reference this theme later
 */
lv_theme_t* etx_lv_theme_init(lv_disp_t* disp, lv_color_t color_primary,
                              lv_color_t color_secondary, bool dark,
                              const lv_font_t* font);

/**
 * Get default theme
 * @return a pointer to default theme, or NULL if this is not initialized
 */
lv_theme_t * etx_lv_theme_get(void);

/**
 * Check if default theme is initialized
 * @return true if default theme is initialized, false otherwise
 */
bool etx_lv_theme_is_inited(void);

#endif /*LV_THEME_H*/
