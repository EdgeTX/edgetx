
local toolName = "TNS|Multi LOLI RX config|TNE"

---- #########################################################################
---- #                                                                       #
---- # Copyright (C) OpenTX                                                  #
-----#                                                                       #
---- # License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html               #
---- #                                                                       #
---- # This program is free software; you can redistribute it and/or modify  #
---- # it under the terms of the GNU General Public License version 2 as     #
---- # published by the Free Software Foundation.                            #
---- #                                                                       #
---- # This program is distributed in the hope that it will be useful        #
---- # but WITHOUT ANY WARRANTY; without even the implied warranty of        #
---- # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
---- # GNU General Public License for more details.                          #
---- #                                                                       #
---- #########################################################################

local loli_nok = false
local channels={ {  768, "PWM", 100,   102, "PPM",      50,  -768, "Servo",      0, -2048, "Switch", -100 }, -- CH1
                 {  768, "PWM", 100,  -768, "Servo",      0, -2048, "Switch", -100 }, -- CH2
                 { -768, "Servo",  0, -2048, "Switch", -100 }, -- CH3
                 { -768, "Servo",  0, -2048, "Switch", -100 }, -- CH4
                 {  102, "SBUS", 50,  -768, "Servo",      0, -2048, "Switch", -100 }, -- CH5
                 { -768, "Servo",  0, -2048, "Switch", -100 }, -- CH6
                 {  768, "PWM", 100,  -768, "Servo",      0, -2048, "Switch", -100 }, -- CH7
                 { -768, "Servo",  0, -2048, "Switch", -100 } } -- CH8

local sel = 1
local edit = false

local blink = 0
local BLINK_SPEED = 15

local function drawScreenTitle(title)
    if LCD_W == 480 then
        lcd.drawFilledRectangle(0, 0, LCD_W, 30, TITLE_BGCOLOR)
        lcd.drawText(1, 5, title, MENU_TITLE_COLOR)
    else
        lcd.drawScreenTitle(title, 0, 0)
    end
end

local function LOLI_Draw_LCD(event)
  local line = 0

  lcd.clear()

  --Display settings
  local lcd_opt = 0
  if LCD_W == 480 then
    drawScreenTitle("Multi - LOLI RX configuration tool")
    x_pos = 152
	x_inc = 90
    y_pos = 40
    y_inc = 20
  else
    x_pos = 5
	x_inc = 30
    y_pos = 1
    y_inc = 8
    lcd_opt = SMLSIZE
  end

  --Multi Module detection
  if loli_nok then
    if LCD_W == 480 then
      lcd.drawText(10,50,"The LOLI protocol is not selected...", lcd_opt)
    else
      --Draw on LCD_W=128
      lcd.drawText(2,17,"LOLI protocol not selected...",SMLSIZE)
    end
    return
  end
  
  --Display current config
  if LCD_W == 480 then
	line = line + 1
	lcd.drawText(x_pos, y_pos+y_inc*line -2, "Channel", lcd_opt)
	lcd.drawText(x_pos+x_inc, y_pos+y_inc*line -2, "Function", lcd_opt)
	lcd.drawRectangle(x_pos-4, y_pos+y_inc*line -4 , 2*x_inc +2, 188)
	lcd.drawLine(x_pos-4, y_pos+y_inc*line +18, x_pos-4 +2*x_inc +1, y_pos+y_inc*line +18, SOLID, 0)
	lcd.drawLine(x_pos+x_inc -5, y_pos+y_inc*line -4, x_pos+x_inc -5, y_pos+y_inc*line -5 +188, SOLID, 0)
    line = line + 1
  end
  
  local out
  for i = 1, 8 do
    out = getValue("ch"..(i+8))
    lcd.drawText(x_pos, y_pos+y_inc*line, "CH"..i, lcd_opt)
    for j = 1, #channels[i], 3 do
      if out > channels[i][j] then
        if sel == i then
          invert = INVERS
          if edit == true then
            blink = blink + 1
            if blink > BLINK_SPEED then
              invert = 0
              if blink > BLINK_SPEED * 2 then
                blink = 0
              end
            end
          end
        else
          invert = 0
        end
        lcd.drawText(x_pos+x_inc, y_pos+y_inc*line, channels[i][j+1], lcd_opt + invert)
        break
      end
    end
    line = line + 1
  end
end

local function LOLI_Change_Value(dir)
  local pos = 0
  local out
  --look for the current position
  out = getValue("ch"..(sel+8))
  for j = 1, #channels[sel], 3 do
    if out > channels[sel][j] then
	  pos = j
	  break
	end
  end
  
  --decrement or increment
  if dir < 0 and pos > 1 then
    pos = pos - 3
  elseif dir > 0 and pos + 3 < #channels[sel] then
    pos = pos + 3
  else
    return
  end
  
  --delete all mixers for the selected channel
  local num_mix = model.getMixesCount(sel-1 +8)
  for i = 1, num_mix do
	model.deleteMix(sel-1 +8, 0);
  end

  --create new mixer
  local source_max = getFieldInfo("cyc1")
  
  local val = { name = channels[sel][pos+1],
				source = source_max.id - 1, -- MAX=100 on TX16S
				weight = channels[sel][pos+2],
				offset = 0,
				switch = 0,
				multiplex = 0,
				curveType = 0,
				curveValue = 0,
				flightModes = 0,
				carryTrim = false,
				mixWarn = 0,
				delayUp = 0,
				delayDown = 0,
				speedUp = 0,
				speedDown = 0 }
  model.insertMix(sel-1 +8, 0, val)
end

local function LOLI_Menu(event)
  if event == EVT_VIRTUAL_NEXT then
    if edit == false then
      -- not changing a value
      if sel < 8 then
        sel = sel + 1
      end
    else
      -- need to inc the value
      LOLI_Change_Value(1)
    end
  elseif event == EVT_VIRTUAL_PREV then
    if edit == false then
      -- not changing a value
      if sel > 1 then
        sel = sel - 1
      end
    else
      -- need to dec the value
      LOLI_Change_Value(-1)
    end
  elseif event == EVT_VIRTUAL_ENTER then
    if edit == false then
      edit = true
      blink = BLINK_SPEED
    else
      edit = false
    end
  end
end

-- Init
local function LOLI_Init()
  local module_conf = model.getModule(0)
  if module_conf["Type"] ~= 6 or module_conf["protocol"] ~= 82 then
    module_conf = model.getModule(1)
    if module_conf["Type"] ~= 6 or module_conf["protocol"] ~= 82 then
      loli_nok = true
    end
  end
end

-- Main
local function LOLI_Run(event)
  if event == nil then
    error("Cannot be run as a model script!")
    return 2
  elseif event == EVT_VIRTUAL_EXIT then
    return 2
  else
    LOLI_Menu(event)
    LOLI_Draw_LCD(event)
    return 0
  end
end

return { init=LOLI_Init, run=LOLI_Run }
