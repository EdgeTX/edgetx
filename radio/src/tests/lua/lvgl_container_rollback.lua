-- Run as a One-Time script on a color radio, or from the native regression.
local function init()
  lvgl.clear()
  local status = lvgl.label({x = 10, y = 10, text = "Starting..."})
  local ok = pcall(function()
    lvgl.box({
      x = 10, y = 60, w = 260, h = 100,
      children = {
        {type = "label", text = "ORPHAN: this should not exist"},
        {type = "not-a-widget"}
      }
    })
  end)
  status:set({
    text = ok and "Unexpected success"
      or "Construction failed: no orphan should remain"
  })
end

return {useLvgl = true, init = init, run = function() return 0 end}
