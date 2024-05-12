#pragma once

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

typedef struct
{
  /* Control functions */
  int32_t (*Init             )(void*, uint32_t, uint32_t);
  int32_t (*DeInit           )(void*);
  int32_t (*ReadID           )(void*, uint32_t*);
  int32_t (*DisplayOn        )(void*);
  int32_t (*DisplayOff       )(void*);
  int32_t (*SetBrightness    )(void*, uint32_t);
  int32_t (*GetBrightness    )(void*, uint32_t*);
  int32_t (*SetOrientation   )(void*, uint32_t);
  int32_t (*GetOrientation   )(void*, uint32_t*);

  /* Drawing functions*/
  int32_t ( *SetCursor       ) (void*, uint32_t, uint32_t);
  int32_t ( *DrawBitmap      ) (void*, uint32_t, uint32_t, uint8_t *);
  int32_t ( *FillRGBRect     ) (void*, uint32_t, uint32_t, uint8_t*, uint32_t, uint32_t);
  int32_t ( *DrawHLine       ) (void*, uint32_t, uint32_t, uint32_t, uint32_t);
  int32_t ( *DrawVLine       ) (void*, uint32_t, uint32_t, uint32_t, uint32_t);
  int32_t ( *FillRect        ) (void*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
  int32_t ( *GetPixel        ) (void*, uint32_t, uint32_t, uint32_t*);
  int32_t ( *SetPixel        ) (void*, uint32_t, uint32_t, uint32_t);
  int32_t ( *GetXSize        ) (void*, uint32_t *);
  int32_t ( *GetYSize        ) (void*, uint32_t *);
} LCD_Drv_t;

#ifdef __cplusplus
}
#endif
