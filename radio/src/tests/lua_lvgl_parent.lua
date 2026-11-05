local function init()
  lvgl.clear()
  lvgl.label({x=10, y=10, text="This label should remain visible"})
  local parent = lvgl.box({x=20, y=50, w=200, h=120})
  assert(not pcall(function() lvgl.label(parent, {bogus=1}) end))
  assert(not pcall(function() lvgl.build(parent, {{type="bogus"}}) end))
  lvgl.label({x=10, y=100, text="This label should ALSO remain visible"})
  lvgl.hide(parent)
end

return {useLvgl=true, init=init, run=function() return 0 end}
