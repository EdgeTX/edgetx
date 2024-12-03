#include "opentx.h"

#define INAVLITE_AFHDS2A
#define INAVLITE_CRSF

static const int8_t sine[32] = {
  0, 24, 48, 70, 90, 106, 117, 125, 127, 125, 117, 106, 90, 70, 48, 24,
  0,-25,-49,-71,-91,-107,-118,-126,-128,-126,-118,-107,-91,-71,-49,-25
};

#define INAV_BATTP_X   30
#define INAV_BATTP_Y    9
#define INAV_VOLT_X    LCD_W
#define INAV_VOLT_Y     1
#define INAV_CELLV_X   30
#define INAV_CELLV_Y   26
#define INAV_CURRENT_X 30
#define INAV_CURRENT_Y 43

#define INAV_GSPD_X    19
#define INAV_GSPD_Y    57

#define INAV_DIST_X    38
#define INAV_DIST_Y    57

#define INAV_ALT_X     91
#define INAV_ALT_Y     57

#define INAV_GALT_X    LCD_W - 5
#define INAV_GALT_Y    43

#define INAV_FM_X      (LCD_W / 2)
#define INAV_FM_Y       9

#define INAV_SATS_X    LCD_W
#define INAV_SATS_Y     8

#define INAV_ARM_X      18
#define INAV_ARM_Y      9

#define BBOX_CENTER_X  (LCD_W / 2)
#define BBOX_CENTER_Y  (36)
#define BBOX_SIZE      (21)

#define HOME_ICON '\xc7'
#define SATS_ICON '\xd1'

//struct Point2D {
//  int8_t x;
//  int8_t y;
//};

struct InavData {
  int32_t homeLat;
  int32_t homeLon;
  int32_t currentLat;
  int32_t currentLon;
  // uint8_t homeHeading;
  uint8_t heading;
  uint8_t armed = 1;
  uint8_t lastMode = 0;
};

static InavData inavData; // = (InavData *)&reusableBuffer.cToolData[0];

/*
static Point2D rotate(Point2D *p, uint8_t angle) {
  Point2D rotated;
  int8_t sinVal = sine[angle];
  int8_t cosVal = sine[(angle + 8) & 0x1F];
  rotated.x = (p->x * cosVal - p->y * sinVal) >> 7;
  rotated.y = (p->y * cosVal + p->x * sinVal) >> 7;
  return rotated;
}
*/

static void inavSetHome() {
  inavData.homeLat = inavData.currentLat;
  inavData.homeLon = inavData.currentLon;
  // inavData.homeHeading = inavData.heading;
  //audioEvent(AU_SPECIAL_SOUND_WARN1);
  audioEvent(AU_SPECIAL_SOUND_TADA);
}

static void inavDrawHome(uint8_t x, uint8_t y) {
  lcdDrawChar(x - 2, y - 3, HOME_ICON);
}

// points: left, right, tip is (0,0) and rotated
static void inavDrawCraft(uint8_t x, uint8_t y) {
  constexpr int8_t pLX = -3;
  constexpr int8_t pLY = 10;
  constexpr int8_t pRX =  3;
  constexpr int8_t pRY = 10;
  uint8_t angle = inavData.heading;
  int8_t sinVal = sine[angle];
  int8_t cosVal = sine[(angle + 8) & 0x1F];

  // rotate
  int8_t rotatedPLX = (pLX * cosVal - pLY * sinVal) >> 7;
  int8_t rotatedPLY = (pLY * cosVal + pLX * sinVal) >> 7;
  int8_t rotatedPRX = (pRX * cosVal - pRY * sinVal) >> 7;
  int8_t rotatedPRY = (pRY * cosVal + pRX * sinVal) >> 7;

  uint8_t tPLX = x + rotatedPLX;
  uint8_t tPLY = y + rotatedPLY;
  uint8_t tPRX = x + rotatedPRX;
  uint8_t tPRY = y + rotatedPRY;

  // translate and draw
  lcdDrawLine(x, y, tPLX, tPLY, SOLID, FORCE);
  lcdDrawLine(x, y, tPRX, tPRY, SOLID, FORCE);
  lcdDrawLine(tPLX, tPLY, tPRX, tPRY, DOTTED, FORCE);
}

//FM2 5-MANUAL, 1-ACRO, 1-AIR, 0-ANGLE, 7-HRZN, 2-ALTHOLD, 8-POSHOLD, 6-RTH, 3-WP, 3-CRUISE, 4-LAUNCH, 9-FAILSAFE
static void inavDrawAFHDS2AFM(uint8_t mode) {
  static const char modeText[10][8] = {
    {'A','N','G','L','E','\0',' ',' '},
    {'A','C','R','O',' ','A','I','R'},
    {'A','L','T',' ','H','O','L','D'},
    {'W','P','C','R','U','I','S', 'E'},
    {'L','A','U','N','C','H','\0',' '},
    {'M','A','N','U','A','L','\0',' '},
    {'R','T','H','\0',' ',' ',' ',' '},
    {'H','O','R','I','Z','O','N','\0'},
    {'P','O','S',' ','H','O','L','D'},
    {'F','A','I','L','S','A','F','E'},
  };

  lcdDrawSizedText(INAV_FM_X, INAV_FM_Y, modeText[mode], 8, SMLSIZE | CENTERED);

  if(inavData.lastMode != mode) {
    audioEvent(AU_SPECIAL_SOUND_WARN2);
  }
  inavData.lastMode = mode;
}

static void inavDraw() {
  lcdDrawSolidVerticalLine(36, FH, LCD_H - FH, FORCE);
  lcdDrawSolidVerticalLine(LCD_W - 31, FH, LCD_H - FH, FORCE);
  lcdDrawSolidVerticalLine(LCD_W - 27, FH, LCD_H - FH, FORCE);
  lcdDrawSolidHorizontalLine(0, 55, 36, FORCE);
  lcdDrawSolidHorizontalLine(LCD_W - 26, 51, 32, FORCE);
  lcdDrawLine(LCD_W - 30, (LCD_H / 2) + FH / 2, LCD_W - 28, (LCD_H / 2) + FH / 2, DOTTED, FORCE);

  uint8_t rxBatt = 0, sats = 0;
  int32_t dist = 0, alt = 0, galt = 0, speed = 0, current = 0;

  int8_t rssi = 0;
  int16_t vspd = 0;

  for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
    if (!isTelemetryFieldAvailable(i)) break;

    TelemetryItem & telemetryItem = telemetryItems[i];

    if (telemetryProtocol == PROTOCOL_TELEMETRY_CROSSFIRE) {
#if defined(INAVLITE_CRSF)
      TelemetrySensor & sensor = g_model.telemetrySensors[i];

      if (strstr(sensor.label, ZSTR_RX_RSSI1)) { // RSSI
        rssi = telemetryItem.value;
      } else if (strstr(sensor.label, ZSTR_ALT)) { // Altitude
        alt = telemetryItem.value;
      } else if (strstr(sensor.label, ZSTR_GPSALT)) { // GPS altitude
        galt = telemetryItem.value;
      } else if (strstr(sensor.label, ZSTR_VSPD)) { // VSpd
        vspd = telemetryItem.value;
      } else if (strstr(sensor.label, ZSTR_BATT_PERCENT)) { // batt percent
        drawValueWithUnit(INAV_BATTP_X, INAV_BATTP_Y, telemetryItem.value, sensor.unit, DBLSIZE | RIGHT);
      } else if (strstr(sensor.label, ZSTR_A4)) { // average cell value
        drawValueWithUnit(INAV_CELLV_X, INAV_CELLV_Y, telemetryItem.value, sensor.unit, PREC2 | DBLSIZE | RIGHT);
      } else if (strstr(sensor.label, ZSTR_BATT)) { // Voltage
        rxBatt = telemetryItem.value;
      } else if (strstr(sensor.label, ZSTR_CURR)) { // Current
        current = telemetryItem.value;
      } else if (strstr(sensor.label, ZSTR_FLIGHT_MODE)) { // flight mode
        lcdDrawSizedText(INAV_FM_X, INAV_FM_Y, telemetryItem.text, sizeof(telemetryItem.text), CENTERED);
      // } else if (sensor.id == TEMP2_ID) { // GPS lock status, accuracy, home reset trigger, and number of satellites.

      } else if (strstr(sensor.label, ZSTR_DIST) || strstr(sensor.label, "0420")) { // Distance, "0420" for INAV and Betaflight
        dist = telemetryItem.value;
      } else if (strstr(sensor.label, ZSTR_HDG)) { // Heading
        // inavData.heading = ((telemetryItem.value / (10 ^ sensor.prec)) * 100) / 1125;
        inavData.heading = convertTelemetryValue(telemetryItem.value, sensor.unit, sensor.prec, sensor.unit, 2) / 1125;
      } else if (strstr(sensor.label, ZSTR_GSPD)) { // GPS Speed
        speed = telemetryItem.value;
      } else if (strstr(sensor.label, ZSTR_SATELLITES)) { // GPS Sats
        sats = telemetryItem.value;
      } else if (strstr(sensor.label, ZSTR_GPS)) { // GPS coords
        inavData.currentLat = telemetryItem.gps.longitude;
        inavData.currentLon = telemetryItem.gps.latitude;
      }
#endif // INAVLITE_CRSF
    } else if (telemetryProtocol == PROTOCOL_TELEMETRY_FLYSKY_IBUS) {
#if defined(INAVLITE_AFHDS2A)
      if (g_model.telemetrySensors[i].id == 0xfc) { // RX RSSI
        rssi = telemetryItem.value;
      }
      if (g_model.telemetrySensors[i].id == 0x80) { // GPS
        inavData.currentLat = telemetryItem.gps.latitude;
        inavData.currentLon = telemetryItem.gps.longitude;
      }

      switch(g_model.telemetrySensors[i].instance) { // inav index - 1
        case 1: // voltage sensor
          rxBatt = telemetryItem.value / 10; // scale down to PREC1
          // draw in place of CRSF cell voltage
          drawValueWithUnit(INAV_CELLV_X, INAV_CELLV_Y, rxBatt, UNIT_VOLTS, PREC1 | DBLSIZE | RIGHT);
          break;
        case 2: // 3. GPS Status, truncated to just Fix in flysky_ibus.cpp
          sats = telemetryItem.value;
          break;
        case 3: // 4. Current in Amperes
          current = telemetryItem.value / 10;
          break;
        // case 4: // 5. Heading (Course in degree)
        //   inavData.heading = telemetryItem.value / 1125; // div by 5.625 => 64 degrees
        //   break;
        case 6: // 7. Climb
          vspd = telemetryItem.value;
          break;
        case 7: // 8. Yaw
          inavData.heading = telemetryItem.value / 1125; // div by 5.625 => 64 degrees
          break;
        case 8: // 9. Dist
          dist = telemetryItem.value;
          break;
        case 9: // 10. Armed
          if(telemetryItem.value != inavData.armed) {
            if(telemetryItem.value == 0) {
              inavData.homeLat = 0;
              inavData.homeLon = 0;
            } else {
              audioEvent(AU_SPECIAL_SOUND_SIREN);
            }
          }
          inavData.armed = telemetryItem.value;  
        break;
        case 10: // 11. Speed
          speed = telemetryItem.value;
          break;
        case 11: // 12. GPS Latitude
          //inavData.currentLat = telemetryItem.value;
          break;
        case 12: // 13. GPS Longitude
          //inavData.currentLon = telemetryItem.value;
          break;
        case 13: // 14. GALT
          galt = (int16_t)(telemetryItem.value) / 10;
          break;
        case 14: // 15. ALT
          alt = (int16_t)(telemetryItem.value) / 10;
          break;
        case 15: // 16. MODE - no need to decode from Status
          inavDrawAFHDS2AFM(telemetryItem.value);
          break;
      }
#endif // INAVLITE_AFHDS2A
    }
  }

  // Fake HDOP for CRSF and AFHDS2A
  // When GPS accuracy (HDOP) is displayed as a decimal, the range is 0.8 - 5.3 and it's rounded to the nearest 0.5 HDOP.
  // This is due to HDOP being sent as a single integer from 0 to 9, not as the actual HDOP decimal value (not applicable to Crossfire)
  // data.satellites = math.min(data.satellites, 99) + (math.floor(math.min(data.satellites + 10, 25) * 0.36 + 0.5) * 100) + (data.satellites >= 6 and 1000 or 0)
  // data.hdop = math.floor(data.satellites * 0.01) % 10
  // (9 - data.hdop) * 0.5 + 0.8
  // lcdDrawText(LCD_W, INAV_SATS_Y + 14, "HDOP", SMLSIZE | RIGHT);
  // lcdDrawNumber(LCD_W, INAV_SATS_Y + 21, (9 - (sats % 10)) * 5 + 8, PREC1 | MIDSIZE | RIGHT);

  drawValueWithUnit(INAV_CURRENT_X, INAV_CURRENT_Y, current, UNIT_AMPS, PREC1 | MIDSIZE | RIGHT);

  drawValueWithUnit(LCD_W - 11, 53, rssi, UNIT_DB, MIDSIZE | RIGHT);
  drawValueWithUnit(INAV_GSPD_X, INAV_GSPD_Y, speed, UNIT_KMH, PREC1 | RIGHT);

  drawValueWithUnit(INAV_DIST_X, INAV_DIST_Y, dist, UNIT_METERS, 0);
  drawValueWithUnit(INAV_ALT_X, INAV_ALT_Y, alt, UNIT_METERS, RIGHT);

  lcdDrawChar(INAV_SATS_X - 25, INAV_SATS_Y + 4, SATS_ICON);
  lcdDrawNumber(INAV_SATS_X, INAV_SATS_Y, sats, MIDSIZE | RIGHT);
  drawValueWithUnit(INAV_GALT_X, INAV_GALT_Y, galt, UNIT_METERS, RIGHT);

  lcdDrawNumber(INAV_SATS_X-8, INAV_SATS_Y + 18, vspd, SMLSIZE | RIGHT);

  static const char armText[2][5] = {
    {'A', 'R', 'M', 'E', 'D'},
    {'O', 'F', 'F', '\0', ' '},
  };  

  lcdDrawSizedText(INAV_ARM_X, INAV_ARM_Y, armText[inavData.armed], 5, SMLSIZE | CENTERED);

  drawValueWithUnit(LCD_W - 6, 0, rxBatt, UNIT_VOLTS, PREC1 | RIGHT);
  drawTelemetryTopBar(); // after rxBatt to add INVERS

  int32_t h = inavData.homeLat - inavData.currentLat;
  int32_t w = inavData.homeLon - inavData.currentLon;
  int32_t d = isqrt32((w * w) + (h * h));

  int32_t scaleFactor = limit<int32_t>(1, (d / BBOX_SIZE), INT16_MAX);

  // calculate center
  int32_t centerLon = (inavData.homeLon + inavData.currentLon) / 2;
  int32_t centerLat = (inavData.homeLat + inavData.currentLat) / 2;

  // translate to center
  int32_t translatedHomeLon = inavData.homeLon - centerLon;
  int32_t translatedHomeLat = inavData.homeLat - centerLat;
  int32_t translatedCurrentLon = inavData.currentLon - centerLon;
  int32_t translatedCurrentLat = inavData.currentLat - centerLat;

  // rotate to homeHeading
  // ...

  // scale
  int8_t scaledHomeLon = translatedHomeLon / scaleFactor;
  int8_t scaledHomeLat = translatedHomeLat / scaleFactor;
  int8_t scaledCurrentLon = translatedCurrentLon / scaleFactor;
  int8_t scaledCurrentLat = translatedCurrentLat / scaleFactor;

  if (sats >= 6 && inavData.homeLat == 0) {
    inavSetHome();
  }

  // translate to LCD center space and draw
  inavDrawHome(BBOX_CENTER_X + scaledHomeLat, BBOX_CENTER_Y - scaledHomeLon);
  inavDrawCraft(BBOX_CENTER_X + scaledCurrentLat, BBOX_CENTER_Y - scaledCurrentLon);

  // draw VSpd line
  vspd = limit<int16_t>(-5, vspd / 4, 5);
  lcdDrawLine(LCD_W - 30, ((LCD_H / 2) + FH / 2) - 10 + vspd, LCD_W - 28, ((LCD_H / 2) + FH / 2) - 10 - vspd, SOLID, FORCE);
  lcdDrawLine(LCD_W - 30, ((LCD_H / 2) + FH / 2) -  9 + vspd, LCD_W - 28, ((LCD_H / 2) + FH / 2) -  9 - vspd, SOLID, FORCE);
}

void menuRadioInavLite(event_t event) {
  if (event != 0xff) {
    globalData.cToolRunning = 1;
    lcdClear(); // when run as telemetry screen clear is done elsewhere
  }

  if (event == EVT_KEY_LONG(KEY_EXIT)) { // exit on long press CANCEL
    globalData.cToolRunning = 0;
    popMenu();
  } else if (event == EVT_KEY_LONG(KEY_ENTER)) { // set home on long press OK
    inavSetHome();
  }

  inavDraw();
}
