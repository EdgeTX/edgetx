-- Run as a One-Time script in a color-screen simulator.
local finished = false

local function init()
  lvgl.clear()

  local function rejectsKey(call)
    for _, key in ipairs({false, 1, {}, function() end}) do
      local ok, err = pcall(call, key)
      assert(not ok, "FAIL: invalid property key was accepted")
      assert(type(err) == "string" and string.find(err, "string expected", 1, true),
             "FAIL: expected a Lua string-key type error: " .. tostring(err))
    end
  end

  rejectsKey(function(key) lvgl.box({[key] = 1}) end)
  rejectsKey(function(key)
    lvgl.build({{type = "box", [key] = 1}})
  end)

  local objects = lvgl.build({{type = "box", name = "parent",
                              children = {{type = "rectangle", name = "child",
                                           x = 1, y = 2, w = 3, h = 4}}}})
  assert(objects.parent ~= nil and objects.child ~= nil,
         "FAIL: valid numeric object-list indices")

  local box = lvgl.box({x = 10, y = 20, w = 100, h = 60,
                        borderPad = {left = 1, right = 2, top = 3, bottom = 4}})
  assert(box ~= nil, "FAIL: valid box parameters")
  rejectsKey(function(key) box:set({[key] = 1}) end)
  rejectsKey(function(key) box:set({borderPad = {[key] = 1}}) end)
  box:set({x = 12, borderPad = 2})

  local page = lvgl.page({title = "Property keys",
                         prevButton = {press = function() end, active = function() return true end},
                         nextButton = {press = function() end, active = function() return true end}})
  assert(page ~= nil, "FAIL: valid page parameters")
  rejectsKey(function(key) page:set({prevButton = {[key] = 1}}) end)
  rejectsKey(function(key) page:set({nextButton = {[key] = 1}}) end)
  page:set({title = "Valid properties still work"})

  lvgl.clear()
  finished = true
end

local function run()
  return finished and 1 or 0
end

return {init = init, run = run, useLvgl = true}
