local weak = setmetatable({}, {__mode = "v"})
local closed = false
local frames = 0

local function init()
  lvgl.clear()
  local data = {string.rep("x", 4096)}
  local callback = function()
    assert(#data[1] == 4096)
    closed = true
  end
  weak[1], weak[2] = callback, data
  lvgl.confirm({
    title = "Reproduction",
    message = "Cancel this popup",
    cancel = callback
  })
end

local function run()
  if closed then
    frames = frames + 1
    if frames == 3 then
      collectgarbage("collect")
      assert(weak[1] == nil, "FAIL: dismissed popup leaked callback memory")
      assert(weak[2] == nil, "FAIL: dismissed popup leaked captured data")
      return 1
    end
  end
  return 0
end

return {useLvgl = true, init = init, run = run}
