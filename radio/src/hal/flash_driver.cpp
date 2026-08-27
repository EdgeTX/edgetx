#include "hal/flash_driver.h"

typedef struct {
  const etx_flash_driver_t* drv; 
  uint32_t                  start_addr;
  uint32_t                  end_addr;
} flash_media_t;

static flash_media_t _flash_media[2] = {{nullptr, 0, 0}};

void flashRegisterDriver(uint32_t start_addr, uint32_t length,
                         const etx_flash_driver_t* drv)
{
  for (int i = 0; i < 2; i++) {
    flash_media_t& fm = _flash_media[i];
    if (fm.drv) continue;
    fm = {
      .drv = drv,
      .start_addr = start_addr,
      .end_addr = start_addr + length - 1,
    };
    return;
  }
}

const etx_flash_driver_t* flashFindDriver(uint32_t addr)
{
  for (int i = 0; i < 2; i++) {
    const flash_media_t& fm = _flash_media[i];
    if (!fm.drv) continue;
    if (fm.start_addr <= addr && fm.end_addr >= addr)
      return fm.drv;
  }

  return nullptr;
}

uint32_t flashGetSize(uint32_t addr)
{
  for (int i = 0; i < 2; i++) {
    const flash_media_t& fm = _flash_media[i];
    if (!fm.drv) continue;
    if (fm.start_addr <= addr && fm.end_addr >= addr)
      return fm.end_addr - fm.start_addr + 1;
  }

  return 0;
}
