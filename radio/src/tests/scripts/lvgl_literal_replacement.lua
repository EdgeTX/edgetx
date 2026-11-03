local calls = 0
local callsAfterReplacement = 0
local frames = 0

local function init()
  lvgl.clear()
  local label = lvgl.label({
    text = function()
      calls = calls + 1
      return "DYNAMIC"
    end
  })
  label:set({text = "LITERAL"})
  callsAfterReplacement = calls
end

local function run()
  frames = frames + 1
  if frames == 1 then
    return 0
  end
  assert(calls == callsAfterReplacement,
         "text callback still active after literal replacement")
  return 1
end

return {useLvgl = true, init = init, run = run}
