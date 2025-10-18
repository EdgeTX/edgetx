/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "edgetx.h"

// For surface radio
void drawWheel(coord_t centrex, int16_t tval)
{
  lcdDrawSquare(centrex-BOX_WIDTH/2, BOX_CENTERY-BOX_WIDTH/2, BOX_WIDTH);
  lcdDrawSolidHorizontalLine(centrex - MARKER_WIDTH /2 , BOX_CENTERY, MARKER_WIDTH);
  coord_t x1 = centrex - MARKER_WIDTH - tval / 300;
  coord_t x2 = centrex - MARKER_WIDTH + tval / 300;
  lcdDrawLine( x1, BOX_CENTERY+BOX_WIDTH / MARKER_WIDTH, x2, BOX_CENTERY-BOX_WIDTH / MARKER_WIDTH, SOLID, FORCE);
  lcdDrawLine( x1 + BOX_WIDTH/2, BOX_CENTERY+BOX_WIDTH / MARKER_WIDTH,
              x2 + BOX_WIDTH / 2, BOX_CENTERY-BOX_WIDTH / MARKER_WIDTH, SOLID, FORCE);
}

// For surface radio
void drawThrottle(coord_t centrex, int16_t wval)
{
  lcdDrawSquare(centrex-BOX_WIDTH/2, BOX_CENTERY-BOX_WIDTH/2, BOX_WIDTH);
  lcdDrawSolidHorizontalLine(centrex - MARKER_WIDTH, BOX_CENTERY, 2 * MARKER_WIDTH + 1);
  coord_t tsize = - wval / 150;
  lcdDrawLine( centrex - tsize, BOX_CENTERY, centrex, BOX_CENTERY + tsize, SOLID, FORCE);
  lcdDrawLine( centrex + tsize, BOX_CENTERY, centrex, BOX_CENTERY + tsize, SOLID, FORCE);
}
