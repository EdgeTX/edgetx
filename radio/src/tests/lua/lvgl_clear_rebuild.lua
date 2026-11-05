-- Run as a One-Time script on a color-screen radio or simulator.
local parent
local replacement
local frames = 0

local function init()
  frames = 0
  lvgl.clear()
  parent = lvgl.box({x=10, y=60, w=200, h=100})
  lvgl.label(parent, {text='Old child'})
  lvgl.clear(parent)
  replacement = lvgl.label(parent, {text='Replacement child'})
end

local function run(event)
  frames = frames + 1
  if frames == 2 then
    local ok, err = pcall(function()
      replacement:set({text='Replacement still works'})
    end)
    assert(ok, err)
    return 1
  end
  return 0
end

return {init=init, run=run, useLvgl=true}
