
class CFS:

    def __init__(self, cfs_rgb_led, cfs_groups):
        self.rgb_led = cfs_rgb_led
        self.groups = cfs_groups

def parse_cfs(hw_defs):

    cfs_rgb_led = 0
    cfs_groups = 0

    fcfs = f'FUNCTION_SWITCHES'
    frgb = f'FUNCTION_SWITCHES_RGB_LEDS'

    if fcfs in hw_defs:
      if frgb in hw_defs:
        cfs_rgb_led = 1
      if f'RADIO_GX12' in hw_defs:
        cfs_groups = 4
      elif f'RADIO_PA01' in hw_defs:
        cfs_groups = 2
      else:
        cfs_groups = 3

    return CFS(cfs_rgb_led, cfs_groups)
