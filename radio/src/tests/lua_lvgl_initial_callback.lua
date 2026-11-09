-- Run as a standalone tool on a color LCD radio, or through the Lua gtest.
-- Successful completion closes the tool after a dynamic-property refresh.
local finished = false
local refreshes = 0

local function init()
  lvgl.clear()
  local ok, result = pcall(function()
    return lvgl.label({
      x = 10, y = 60,
      text = function() error('deliberate callback failure') end
    })
  end)
  if ok then
    result:set({text = function() return 'Recovered for display' end})
  end
  assert(not ok, 'FAIL: constructor ignored the initial callback failure')
  assert(type(result) == 'string', 'FAIL: missing callback error')
  assert(string.find(result, 'lua_lvgl_initial_callback.lua', 1, true),
         'FAIL: missing script context: ' .. result)
  assert(string.find(result, 'deliberate callback failure', 1, true),
         'FAIL: lost callback error: ' .. result)

  for _, value in ipairs({42, {reason = 'test'}}) do
    local success, err = pcall(function()
      return lvgl.label({text = function() error(value) end})
    end)
    assert(not success, 'FAIL: non-string callback error ignored')
    assert(err == value, 'FAIL: callback error object changed')
  end

  for _, property in ipairs({'visible', 'color', 'size', 'pos'}) do
    local success, err = pcall(function()
      return lvgl.label({
        [property] = function() error('initial ' .. property .. ' failure') end
      })
    end)
    assert(not success, 'FAIL: initial ' .. property .. ' failure ignored')
    assert(string.find(err, 'initial ' .. property .. ' failure', 1, true),
           'FAIL: lost ' .. property .. ' error')
  end

  local success, err = pcall(function()
    return lvgl.box({visible = function() error('box callback failure') end})
  end)
  assert(not success, 'FAIL: box callback failure ignored')
  assert(string.find(err, 'box callback failure', 1, true), 'FAIL: lost box error')

  for _, constructor in ipairs({'page', 'dialog', 'message'}) do
    local success, err = pcall(function()
      return lvgl[constructor]({
        visible = function() error(constructor .. ' callback failure') end
      })
    end)
    assert(not success, 'FAIL: ' .. constructor .. ' callback failure ignored')
    assert(string.find(err, constructor .. ' callback failure', 1, true),
           'FAIL: lost ' .. constructor .. ' error')
  end

  success, err = pcall(function()
    lvgl.build({
      {type = 'label', text = function() error('build callback failure') end}
    })
  end)
  assert(not success, 'FAIL: build callback failure ignored')
  assert(string.find(err, 'build callback failure', 1, true), 'FAIL: lost build error')

  local label = lvgl.label({
    visible = function() return true end,
    color = function() return COLOR_THEME_PRIMARY1 end,
    size = function() return 100, 30 end,
    pos = function() return 10, 60 end,
    text = function()
      refreshes = refreshes + 1
      return 'Success'
    end
  })
  assert(label, 'FAIL: valid dynamic label rejected')
  finished = true
end

return {
  init = init,
  run = function() return finished and refreshes > 1 and 1 or 0 end,
  useLvgl = true
}
