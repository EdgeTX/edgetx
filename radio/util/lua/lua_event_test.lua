local toolName = "TNS|LUA event tests|TNE"
---- #########################################################################
---- #                                                                       #
---- # Copyright (C) EdgeTX
---- #
---- # Based on code named
---- #   opentx - https://github.com/opentx/opentx
---- #   th9x - http://code.google.com/p/th9x
---- #   er9x - http://code.google.com/p/er9x
---- #   gruvin9x - http://code.google.com/p/gruvin9x
---- #
---- # License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
---- #
---- # This program is free software; you can redistribute it and/or modify
---- # it under the terms of the GNU General Public License version 2 as
---- # published by the Free Software Foundation.
---- #
---- # This program is distributed in the hope that it will be useful,
---- # but WITHOUT ANY WARRANTY; without even the implied warranty of
---- # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
---- # GNU General Public License for more details.
---- #                                                                       #
---- #########################################################################

local virtual_next = 0
local virtual_prev = 0
local virtual_inc = 0
local virtual_dec = 0
local virtual_next_rept = 0
local virtual_prev_rept = 0
local virtual_page_next = 0
local virtual_page_prev = 0
local virtual_enter = 0
local virtual_enter_long = 0
local virtual_menu = 0
local virtual_menu_long = 0

-- Init
local function init()

end

-- Main
local function run(event)
  if event == nil then
    error("Cannot be run as a model script!")
    return 2
  end
  if event == EVT_VIRTUAL_EXIT then
    return 2
  end
  if event == EVT_VIRTUAL_NEXT then
    virtual_next = virtual_next + 1
  end
  if event == EVT_VIRTUAL_PREV then
    virtual_prev = virtual_prev + 1
  end
  if event == EVT_VIRTUAL_INC then
    virtual_inc = virtual_inc + 1
  end
  if event == EVT_VIRTUAL_DEC then
    virtual_dec = virtual_dec + 1
  end
  if event == EVT_VIRTUAL_NEXT_REPT then
    virtual_next_rept = virtual_next_rept + 1
  end
  if event == EVT_VIRTUAL_PREV_REPT then
    virtual_prev_rept = virtual_prev_rept + 1
  end
  if event == EVT_VIRTUAL_NEXT_PAGE then
    virtual_page_next = virtual_page_next +1
  end
  if event == EVT_VIRTUAL_PREV_PAGE then
    virtual_page_prev = virtual_page_prev + 1
    killEvents(event);
  end
  if event == EVT_VIRTUAL_ENTER then
    virtual_enter = virtual_enter + 1
  end
  if event == EVT_VIRTUAL_MENU then
    virtual_menu = virtual_menu + 1
  end

  lcd.clear()
  lcd.drawScreenTitle("LUA EVENT TEST", 0, 0)
  lcd.drawText(1, 10, "PREV:"..virtual_prev)
  lcd.drawText(LCD_W/2, 10, "NEXT: "..virtual_next)
  lcd.drawText(1, 20, "DEC:"..virtual_dec)
  lcd.drawText(LCD_W/2, 20, "INC:"..virtual_inc)
  lcd.drawText(1, 30, "NXT PAGE: "..virtual_page_next)
  lcd.drawText(LCD_W/2, 30, "PRV PAGE: "..virtual_page_prev)
  lcd.drawText(1, 50, "ENTER: "..virtual_enter)
  lcd.drawText(LCD_W/2, 50, "MENU: "..virtual_menu)
  return 0
end

return { init=init, run=run }

