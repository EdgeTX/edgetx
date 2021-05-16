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
local VALUE = 0
local COMBO = 1

local COLUMN_2 = 150

local edit = false
local page = 1
local current = 1
local refreshState = 0
local refreshIndex = 0
local calibrationState = 0
local pageOffset = 0
local calibrationStep = 0
local pages = {}
local fields = {}
local modifications = {}

local calibrationFields = {
  {"X:", VALUE, 0x9E, 0, -100, 100, "%"},
  {"Y:", VALUE, 0x9F, 0, -100, 100, "%"},
  {"Z:", VALUE, 0xA0, 0, -100, 100, "%"}
}

local function drawProgressBar()
  local width = (140 * refreshIndex) / #fields
  lcd.drawRectangle(30, 1, 144, 6)
  lcd.drawFilledRectangle(32, 3, width, 2);
end

-- Select the next or previous page
local function selectPage(step)
  page = 1 + ((page + step - 1 + #pages) % #pages)
  refreshIndex = 0
  calibrationStep = 0
  pageOffset = 0
end

-- Draw initial warning page
local function runWarningPage(event)
  lcd.clear()
  lcd.drawScreenTitle("SxR", page, #pages)
  lcd.drawText(0, 10, "Warning: this will start SxR calibration", SMLSIZE)
  lcd.drawText(0, 20, "This need to be run only once. You need a SxR,", SMLSIZE)
  lcd.drawText(0, 30, "power supply and a flat level surface (desk,...)", SMLSIZE)
  lcd.drawText(0, 40, "Press [Enter] when ready", SMLSIZE)
  lcd.drawText(0, 50, "Press [Exit] when cancel", SMLSIZE)
  if event == EVT_VIRTUAL_ENTER then
    selectPage(1)
    return 0
  elseif event == EVT_VIRTUAL_EXIT then
    return 2
  end
  return 0
end

-- Redraw the current page
local function redrawFieldsPage()
  lcd.clear()
  lcd.drawScreenTitle("SxR", page, #pages)

  if refreshIndex < #fields then
    drawProgressBar()
  end

  for index = 1, 7, 1 do
    local field = fields[pageOffset+index]
    if field == nil then
      break
    end

    local attr = current == (pageOffset+index) and ((edit == true and BLINK or 0) + INVERS) or 0

    lcd.drawText(0, 1+8*index, field[1])

    if field[4] == nil then
      lcd.drawText(COLUMN_2, 1+8*index, "---", attr)
    else
      if field[2] == VALUE then
        lcd.drawNumber(COLUMN_2, 1+8*index, field[4], LEFT + attr)
      elseif field[2] == COMBO then
        if field[4] >= 0 and field[4] < #(field[5]) then
          lcd.drawText(COLUMN_2, 1+8*index, field[5][1+field[4]], attr)
        end
      end
    end
  end
end

local function telemetryRead(field)
  return sportTelemetryPush(0x17, 0x30, 0x0C30, field)
end

local function telemetryWrite(field, value)
  return sportTelemetryPush(0x17, 0x31, 0x0C30, field + value*256)
end

local telemetryPopTimeout = 0
local function refreshNext()
  if refreshState == 0 then
    if calibrationState == 1 then
      if telemetryWrite(0x9D, calibrationStep) == true then
        refreshState = 1
        calibrationState = 2
        telemetryPopTimeout = getTime() + 80 -- normal delay is 500ms
      end
    elseif #modifications > 0 then
      telemetryWrite(modifications[1][1], modifications[1][2])
      modifications[1] = nil
    elseif refreshIndex < #fields then
      local field = fields[refreshIndex + 1]
      if telemetryRead(field[3]) == true then
        refreshState = 1
        telemetryPopTimeout = getTime() + 80 -- normal delay is 500ms
      end
    end
  elseif refreshState == 1 then
    local physicalId, primId, dataId, value = sportTelemetryPop()
    if physicalId == 0x1A and primId == 0x32 and dataId == 0x0C30 then
      local fieldId = value % 256
      if calibrationState == 2 then
        if fieldId == 0x9D then
          refreshState = 0
          calibrationState = 0
          calibrationStep = (calibrationStep + 1) % 7
        end
      else
        local field = fields[refreshIndex + 1]
        if fieldId == field[3] then
          local value = math.floor(value / 256)
          value =  bit32.band(value, 0xffff)
          if field[3] >= 0x9E and field[3] <= 0xA0 then
            local b1 = value % 256
            local b2 = math.floor(value / 256)
            value = b1*256 + b2
            value = value - bit32.band(value, 0x8000) * 2
          end
          if field[2] == COMBO and #field == 6 then
            for index = 1, #(field[6]), 1 do
              if value == field[6][index] then
                value = index - 1
                break
              end
            end
          elseif field[2] == VALUE and #field == 8 then
            value = value - field[8] + field[5]
          end
          fields[refreshIndex + 1][4] = value
          refreshIndex = refreshIndex + 1
          refreshState = 0
        end
      end
    elseif getTime() > telemetryPopTimeout then
      refreshState = 0
      calibrationState = 0
    end
  end
end

local function updateField(field)
  local value = field[4]
  if field[2] == COMBO and #field == 6 then
    value = field[6][1+value]
  elseif field[2] == VALUE and #field == 8 then
    value = value + field[8] - field[5]
  end
  modifications[#modifications+1] = { field[3], value }
end

-- Main
local function runFieldsPage(event)
  if event == EVT_VIRTUAL_EXIT then -- exit script
    return 2
  elseif event == EVT_VIRTUAL_ENTER then -- toggle editing/selecting current field
    if fields[current][4] ~= nil then
      edit = not edit
      if edit == false then
        updateField(fields[current])
      end
    end
  elseif edit then
    if event == EVT_VIRTUAL_NEXT or event == EVT_VIRTUAL_NEXT_REP then
      addField(1)
    elseif event == EVT_VIRTUAL_PREV or event == EVT_VIRTUAL_PREV_REP then
      addField(-1)
    end
  else
    if event == EVT_VIRTUAL_INC or event == EVT_VIRTUAL_INC_REP then
      selectField(1)
    elseif event == EVT_VIRTUAL_DEC or event == EVT_VIRTUAL_DEC_REP then
      selectField(-1)
    end
  end
  redrawFieldsPage()
  return 0
end

local calibrationPositionsBitmaps = { "bmp/up.bmp", "bmp/down.bmp", "bmp/left.bmp", "bmp/right.bmp", "bmp/forward.bmp", "bmp/back.bmp"  }

local function runCalibrationPage(event)
  fields = calibrationFields
  if refreshIndex == #fields then
    refreshIndex = 0
  end
  lcd.clear()
  lcd.drawScreenTitle("SxR", page, #pages)
  if(calibrationStep < 6) then
    lcd.drawText(0, 9, "Turn the SxR as shown", 0)
    lcd.drawPixmap(10, 19, calibrationPositionsBitmaps[1 + calibrationStep])
    for index = 1, 3, 1 do
      local field = fields[index]
      lcd.drawText(80, 12+10*index, field[1], 0)
      lcd.drawNumber(90, 12+10*index, field[4]/10, LEFT+PREC2)
    end

    local attr = calibrationState == 0 and INVERS or 0
    lcd.drawText(0, 56, "Press [Enter] when ready", attr)
  else
    lcd.drawText(0, 9, "Calibration completed", 0)
    lcd.drawPixmap(10, 19, "bmp/done.bmp")
    lcd.drawText(0, 56, "Press [Exit] when ready", attr)
  end
  if calibrationStep > 6 and (event == EVT_VIRTUAL_ENTER or event == EVT_VIRTUAL_EXIT) then
    return 2
  elseif event == EVT_VIRTUAL_ENTER then
    calibrationState = 1
  elseif event == EVT_VIRTUAL_EXIT then
    if calibrationStep > 0 then
      calibrationStep = 0
    end
  end
  return 0
end

-- Init
local function init()
  current, edit, refreshState, refreshIndex = 1, false, 0, 0
  pages = {
    runWarningPage,
    runCalibrationPage
  }
end

-- Main
local function run(event)
  if event == nil then
    error("Cannot be run as a model script!")
    return 2
  elseif event == EVT_VIRTUAL_NEXT_PAGE then
    selectPage(1)
  elseif event == EVT_VIRTUAL_PREV_PAGE then
    killEvents(event);
    selectPage(-1)
  end

  local result = pages[page](event)
  refreshNext()

  return result
end

return { init=init, run=run }
