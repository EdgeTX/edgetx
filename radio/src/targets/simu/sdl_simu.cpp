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

#include <SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#if !SDL_VERSION_ATLEAST(2,0,19)
#error This backend requires SDL 2.0.19+ because of SDL_RenderGeometryRaw() function
#endif

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wunused-function"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb_image.h"

#if defined(STBI_NO_STDIO)
  #error "STBI_NO_STDIO is defined"
#endif

#include "simu.h"
#include "simuaudio.h"
#include "hal/key_driver.h"

#include "audio.h"
#include "debug.h"
#include "opentx.h"

#include <algorithm>

#define TIMER_INTERVAL 10 // 10ms

SDL_Window* window;
SDL_Renderer* renderer;

SDL_Texture* screen_frame_buffer;
SDL_Texture* gimbal_frame;
SDL_Texture* stick_dot;

static ImVec2 stick_l_pos(0.5f, 0.5f);
static ImVec2 stick_r_pos(0.5f, 0.5f);

static const unsigned char _icon_png[] = {
  #include "icon.lbm"
};

static const unsigned char _gimbal_frame_png[] = {
  #include "gimbal_frame.lbm"
};

static const unsigned char _stick_dot_png[] = {
  #include "stick_dot.lbm"
};

static void _set_pixel(uint8_t* pixel, const SDL_Color& color)
{
  pixel[0] = color.a;
  pixel[1] = color.b;
  pixel[2] = color.g;
  pixel[3] = color.r;
}

static void _blit_simu_screen_color(void* screen_buffer, Uint32 format, int w, int h, int pitch)
{
  pixel_t* src_buffer = simuLcdBuf;
  uint8_t* line_buffer = (uint8_t*)screen_buffer;

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      pixel_t z = *src_buffer++;
      // Alpha
      line_buffer[0] = SDL_ALPHA_OPAQUE;
      // Blue
      line_buffer[1] = (((z & 0x001F) << 3) & 0x00F8) + ((z & 0x001C) >> 2);
      // Green
      line_buffer[2] = ((z & 0x07E0) >> 3) + ((z & 0x0600) >> 9);
      // Red
      line_buffer[3] = ((z & 0xF800) >> 8) + ((z & 0xE000) >> 13);
      line_buffer += SDL_BYTESPERPIXEL(format);
    }
  }
}

static void _blit_simu_screen_1bit(void* screen_buffer, Uint32 format, int w, int h, int pitch)
{
  const SDL_Color on_color = {.r = 0, .g = 0, .b = 0};

  uint8_t* line_buffer = (uint8_t*)screen_buffer;

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      SDL_Color c = on_color;
      if (simuLcdBuf[x + (y / 8) * w] & (1 << (y % 8))) {
        c.a = SDL_ALPHA_OPAQUE;
      } else {
        c.a = SDL_ALPHA_TRANSPARENT;
      }
      _set_pixel(line_buffer, c);
      line_buffer += SDL_BYTESPERPIXEL(format);
    }
  }
}

static inline uint8_t _4bit_blend(uint16_t px, uint16_t bg)
{
  uint16_t c = bg * (16 - px);
  return c / 16;
}

static void _blit_simu_screen_4bit(void* screen_buffer, Uint32 format, int w, int h, int pitch)
{
  const SDL_Color on_color = {.r = 0, .g = 0, .b = 0};

  uint8_t* line_buffer = (uint8_t*)screen_buffer;

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {

      pixel_t p = simuLcdBuf[y / 2 * LCD_W + x];
      uint8_t z = (y & 1) ? (p >> 4) : (p & 0x0F);

      SDL_Color c = on_color;
      c.a = (uint16_t)z * SDL_ALPHA_OPAQUE / 16;

      _set_pixel(line_buffer, c);
      line_buffer += SDL_BYTESPERPIXEL(format);
    }
  }
}

void refreshDisplay(SDL_Texture* screen)
{
  if (simuLcdRefresh) {

    // fetch texture format
    Uint32 format = 0;
    int width = 0, height = 0;

    if (SDL_QueryTexture(screen, &format, nullptr, &width, &height) != 0) {
      TRACE("SDL_QueryTexture: %s", SDL_GetError());
      return;
    }
    
    // raw pixel buffer
    void* screen_buffer = nullptr;

    // length of one row in bytes
    int pitch = 0;

    if (SDL_LockTexture(screen, nullptr, &screen_buffer, &pitch) != 0) {
      TRACE("SDL_LockTexture: %s", SDL_GetError());
      return;
    }

    if (LCD_DEPTH == 1) {
      _blit_simu_screen_1bit(screen_buffer, format, width, height, pitch);
    } else if (LCD_DEPTH == 4) {
      _blit_simu_screen_4bit(screen_buffer, format, width, height, pitch);
    } else if (LCD_DEPTH == 16) {
      _blit_simu_screen_color(screen_buffer, format, width, height, pitch);
    }

    SDL_UnlockTexture(screen);
    simuLcdRefresh = false;
    lcdFlushed();
  }
}

Uint32 timer_10ms_cb(Uint32 interval, void* name)
{
  per10ms();
  return TIMER_INTERVAL;
}

void handleTouchEvents(const ImVec2& screen_p0, float scale_ratio)
{
#if defined(HARDWARE_TOUCH)
  static bool mouse_was_down = false;

  if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
    
    ImGuiIO& io = ImGui::GetIO();
    const ImVec2 mouse_pos(io.MousePos.x - screen_p0.x, io.MousePos.y - screen_p0.y);
    ImGui::Text("Mouse: x = %f; y = %f", mouse_pos.x, mouse_pos.y);

    const ImVec2 scaled_pos(mouse_pos.x / scale_ratio, mouse_pos.y / scale_ratio);
    ImGui::Text("Pos: x = %f; y = %f", scaled_pos.x, scaled_pos.y);

    touchPanelDown((short)scaled_pos.x, (short)scaled_pos.y);
    mouse_was_down = true;
  } else if (mouse_was_down) {
    touchPanelUp();
    mouse_was_down = false;        
  }
#endif
}

bool simuProcessEvents(SDL_Event& event)
{
  if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
    const auto& key_event = event.key;

    bool key_handled = false;
    uint8_t key = 0;
    
    switch(key_event.keysym.sym) {

    case SDLK_ESCAPE:
      key = KEY_EXIT;
      key_handled = true;
      break;
      
    case SDLK_RETURN:
      key = KEY_ENTER;
      key_handled = true;
      break;

    case SDLK_LEFT:
      if (keysGetSupported() & (1 << KEY_LEFT)) {
        key = KEY_LEFT;
        key_handled = true;
      }
      break;
      
    case SDLK_RIGHT:
      if (keysGetSupported() & (1 << KEY_RIGHT)) {
        key = KEY_RIGHT;
        key_handled = true;
      }
      break;

    case SDLK_UP:
      if (keysGetSupported() & (1 << KEY_UP)) {
        key = KEY_UP;
        key_handled = true;
      }
      break;
      
    case SDLK_DOWN:
      if (keysGetSupported() & (1 << KEY_DOWN)) {
        key = KEY_DOWN;
        key_handled = true;
      }
      break;

    case SDLK_PLUS:
      if (keysGetSupported() & (1 << KEY_PLUS)) {
        key = KEY_PLUS;
        key_handled = true;
      }
      break;

    case SDLK_MINUS:
      if (keysGetSupported() & (1 << KEY_MINUS)) {
        key = KEY_MINUS;
        key_handled = true;
      }
      break;

    case SDLK_m:
      if (keysGetSupported() & (1 << KEY_MENU)) {
        key = KEY_MENU;
        key_handled = true;
      } else if (keysGetSupported() & (1 << KEY_MODEL)) {
        key = KEY_MODEL;
        key_handled = true;     
      }
      break;

    case SDLK_s:
      if (keysGetSupported() & (1 << KEY_SYS)) {
        key = KEY_SYS;
        key_handled = true;
      }
      break;

    case SDLK_t:
      if (keysGetSupported() & (1 << KEY_TELE)) {
        key = KEY_TELE;
        key_handled = true;
      }
      break;

    default:
      key_handled = false;
      break;
    }

    if (key_handled) {
      simuSetKey(key, key_event.type == SDL_KEYDOWN ? true : false);
    }

    return key_handled;
  }

  return false;
}

SDL_Surface* LoadImage(const unsigned char* pixels, size_t len)
{
  // Read data
  int32_t w, h, bpp;

  void* data = stbi_load_from_memory(pixels, len, &w, &h, &bpp, 0);
  if (!data) return NULL;

  Uint32 format = (bpp == 3) ? SDL_PIXELFORMAT_RGB24 : SDL_PIXELFORMAT_RGBA32;

  SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, bpp * 8, format);
  if (surface) {
    SDL_LockSurface(surface);
    memcpy(surface->pixels, data, w * h * bpp);
    SDL_UnlockSurface(surface);
  }

  stbi_image_free(data);
  return surface;
}

SDL_Texture* LoadTexture(SDL_Renderer* renderer, const unsigned char* pixels, size_t len)
{
  SDL_Surface* surface = LoadImage(pixels, len);
  if (!surface) return NULL;

  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);

  return texture;
}

void SingleGimbal(const char* name, SDL_Texture* gimbal_frame,
                  SDL_Texture* stick_dot, ImVec2& stick_pos,
		  bool lock_y)
{
  auto frame_width = ImGui::GetContentRegionAvail().x;
  auto gimbal_width = frame_width < 91.0f ? 91.0f : frame_width > 200.0f ? 200.0f : frame_width;

  float ratio = gimbal_width / 182.0f;
  
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  ImVec2 pos = ImGui::GetCursorScreenPos();
  ImVec2 gimbal_p0(pos.x + (frame_width - gimbal_width) / 2.0f, pos.y);
  ImVec2 gimbal_p1(gimbal_p0.x + gimbal_width, gimbal_p0.y + gimbal_width);
  draw_list->AddImage(gimbal_frame, gimbal_p0, gimbal_p1);

  float dot_width = 38.0f * ratio;
  float dot_height = 40.0f * ratio;
  float dot_mid = 17.0f * ratio;
  float dot_range = gimbal_width - dot_mid * 2.0f;

  ImVec2 dot_p0(gimbal_p0.x + dot_range * stick_pos.x,
                gimbal_p0.y + dot_range * stick_pos.y);
  ImVec2 dot_p1(dot_p0.x + dot_width, dot_p0.y + dot_height);
  draw_list->AddImage(stick_dot, dot_p0, dot_p1);

  ImGui::InvisibleButton(name, ImVec2(frame_width, gimbal_width));
  if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {

    const auto& io = ImGui::GetIO();
    ImVec2 mouse_pos(io.MousePos.x - gimbal_p0.x,
                     io.MousePos.y - gimbal_p0.y);

    if (mouse_pos.x >= 0 && mouse_pos.x <= gimbal_width &&
        mouse_pos.y >= 0 && mouse_pos.y <= gimbal_width) {

      stick_pos.x = (mouse_pos.x - dot_mid) / dot_range;
      stick_pos.y = (mouse_pos.y - dot_mid) / dot_range;

      if (stick_pos.x < 0.0f) stick_pos.x = 0.0f;
      if (stick_pos.x > 1.0f) stick_pos.x = 1.0f;
      if (stick_pos.y < 0.0f) stick_pos.y = 0.0f;
      if (stick_pos.y > 1.0f) stick_pos.y = 1.0f;
      
      // ImGui::Text("X = %0.2f Y = %0.2f", stick_pos.x, stick_pos.y);
    }
  } else {
    if (stick_pos.x != 0.5f) {
      auto diff = 0.5 - stick_pos.x;
      if (std::abs(diff) > 0.01) {
        stick_pos.x += diff / 10;
      } else {
        stick_pos.x = 0.5;
      }
    }
    if (!lock_y && stick_pos.y != 0.5f) {
      auto diff = 0.5 - stick_pos.y;
      if (std::abs(diff) > 0.01) {
        stick_pos.y += diff / 10;
      } else {
        stick_pos.y = 0.5;
      }
    }
  }
}

void DisplayGimbals(SDL_Texture* gimbal_frame, SDL_Texture* stick_dot)
{
  // TODO: minimum width
  if (ImGui::BeginTable("gimbals", 2, 0)) {
    ImGui::TableNextRow();

    ImGui::TableSetColumnIndex(0);
    SingleGimbal("GimbalL", gimbal_frame, stick_dot, stick_l_pos, g_eeGeneral.stickMode == 1);

    ImGui::TableSetColumnIndex(1);
    SingleGimbal("GimbalR", gimbal_frame, stick_dot, stick_r_pos, g_eeGeneral.stickMode == 0);

    ImGui::EndTable();
  }
}

Uint32 get_bg_color()
{
  if (isBacklightEnabled()) {
    return IM_COL32(47, 123, 227, 255);
  } else {
    return IM_COL32(200, 200, 200, 255);
  }
}

void redraw()
{
  ImGuiStyle& style = ImGui::GetStyle();

  // poll audio
  audioQueue.wakeup();

  refreshDisplay(screen_frame_buffer);
    
  // Start the Dear ImGui frame
  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  static ImGuiWindowFlags flags =
    ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoTitleBar;

  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);

  bool show_win = true;
  if (ImGui::Begin("Main window", &show_win, flags)) {

    // show gimbals
    DisplayGimbals(gimbal_frame, stick_dot);
      
    ImVec2 screen_p0 = ImGui::GetCursorScreenPos();
    float aspect_ratio = float(LCD_H) / float(LCD_W);

    float width = viewport->WorkSize.x - 2 * style.WindowPadding.x;
    float scale_ratio = width / float(LCD_W);

    ImVec2 size(width, width * aspect_ratio);

    if (LCD_DEPTH == 1 || LCD_DEPTH == 4) {
      ImDrawList* draw_list = ImGui::GetWindowDrawList();
      ImVec2 p1 = { screen_p0.x + size.x, screen_p0.y + size.y };
      draw_list->AddRectFilled(screen_p0, p1, get_bg_color());
    }

    ImGui::Image(screen_frame_buffer, size);

    if (LCD_DEPTH == 1 || LCD_DEPTH == 4) {
      ImDrawList* draw_list = ImGui::GetWindowDrawList();
      float dx = size.x / float(LCD_W);
      float thickness = dx / 50.0;
      auto col = get_bg_color() & 0x80FFFFFF;
      for (int x = 1; x < LCD_W; x++) {
	ImVec2 p1 = { screen_p0.x + x * size.x / float(LCD_W), screen_p0.y };
	ImVec2 p2 = { p1.x, screen_p0.y + size.y - 1 };
	draw_list->AddLine(p1, p2, col, thickness);
      }
      for (int y = 1; y < LCD_H; y++) {
	ImVec2 p1 = { screen_p0.x, screen_p0.y + y * size.y / float(LCD_H) };
	ImVec2 p2 = { screen_p0.x + size.x - 1, p1.y };
	draw_list->AddLine(p1, p2, col, thickness);
      }
    }    
    
    handleTouchEvents(screen_p0, scale_ratio);

    ImGui::Text("tmr10ms: %u", g_tmr10ms);
    ImGui::Text("rtos time: %u", RTOS_GET_MS());
  }
  ImGui::End();

  // Rendering
  ImGui::Render();

  ImGuiIO& io = ImGui::GetIO();
  SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x,
                     io.DisplayFramebufferScale.y);

  SDL_SetRenderDrawColor(renderer, 114, 140, 153, 255);
  SDL_RenderClear(renderer);

  ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
  SDL_RenderPresent(renderer);
}

bool handle_events()
{
  SDL_Event event;
  while (SDL_PollEvent(&event)) {

    if(simuProcessEvents(event))
      continue;
      
    ImGui_ImplSDL2_ProcessEvent(&event);

    if (event.type == SDL_QUIT)
      return false;

    if (event.type == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_CLOSE &&
        event.window.windowID == SDL_GetWindowID(window))
      return false;
  }

  redraw();
  return true;
}

int main(int argc, char** argv)
{
  // Init simulation
  simuInit();
  simuStart(false, nullptr, nullptr);

  // Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
    printf("Error: %s\n", SDL_GetError());
    return -1;
  }

  simuAudioInit();
  
  // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
 
  // Create window with SDL_Renderer graphics context
  SDL_WindowFlags window_flags =
      (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  window = SDL_CreateWindow("EdgeTx Simu", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 600, 600, window_flags);

  renderer = SDL_CreateRenderer(
               window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    SDL_Log("Error creating SDL_Renderer!");
    return 0;
  }

  SDL_RendererInfo info;
  SDL_GetRendererInfo(renderer, &info);
  SDL_Log("Current SDL_Renderer: %s", info.name);

  SDL_Surface* icon = LoadImage(_icon_png, sizeof(_icon_png));
  if (window && icon) {
    SDL_SetWindowIcon(window, icon);
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  // Setup Dear ImGui style
  ImGui::StyleColorsLight();
  // ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer2_Init(renderer);

  // Create textures for radio screen
  screen_frame_buffer =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_STREAMING, LCD_W, LCD_H);

  if (!screen_frame_buffer) {
    SDL_Log("Could not create frame buffer textures");
    return 0;
  }

  // SDL_SetTextureScaleMode(screen_frame_buffer, SDL_ScaleModeBest);
  SDL_SetTextureBlendMode(screen_frame_buffer, SDL_BLENDMODE_BLEND);

  gimbal_frame = LoadTexture(renderer, _gimbal_frame_png, sizeof(_gimbal_frame_png));
  stick_dot = LoadTexture(renderer, _stick_dot_png, sizeof(_stick_dot_png));

  if (!gimbal_frame || !stick_dot) {
    SDL_Log("Could not load textures");
    return 0;
  }

  // 10ms timer
  SDL_TimerID timerID_10ms =
      SDL_AddTimer(TIMER_INTERVAL, timer_10ms_cb, const_cast<char*>("10ms"));
  if (!timerID_10ms) {
    SDL_Log("Error creating SDL_AddTimer!");
    return 0;
  }

  // race condition on YAML loaded...
  if (g_eeGeneral.stickMode == 1) {
    stick_l_pos.y = 1.0f;
  } else if (g_eeGeneral.stickMode == 0) {
    stick_r_pos.y = 1.0f;
  }
  
  // Main loop
  SDL_SetEventFilter([](void*, SDL_Event* event){
    if (event->type == SDL_WINDOWEVENT &&
        (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
         event->window.event == SDL_WINDOWEVENT_RESIZED)) {
      redraw();
      return 0;
    }
    return 1;
  }, NULL);

#if defined(__EMSCRIPTEN__)
  emscripten_set_main_loop([]() { handle_events(); }, 0, true);
#else
  do {
    Uint64 start_ts = SDL_GetPerformanceCounter();
    if (!handle_events()) break;

    Uint64 end_ts = SDL_GetPerformanceCounter();
    float elapsedMS =
      (end_ts - start_ts) / (float)SDL_GetPerformanceFrequency() * 1000.0f;

    // Cap to 60 FPS
    SDL_Delay(floor(16.666f - elapsedMS));

  } while(true);
#endif

  // Cleanup
  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyTexture(screen_frame_buffer);
  SDL_DestroyTexture(gimbal_frame);
  SDL_DestroyTexture(stick_dot);

  SDL_RemoveTimer(timerID_10ms);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_FreeSurface(icon);
  SDL_CloseAudio();
  SDL_Quit();
  
  return 0;
}

uint16_t simu_get_analog(uint8_t idx)
{
  auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
  if (idx < max_sticks) {
    switch(idx) {
    case 0: return stick_l_pos.x * 4096;
    case 1: return (1.0 - stick_l_pos.y) * 4096;
    case 2: return (1.0 - stick_r_pos.y) * 4096;
    case 3: return stick_r_pos.x * 4096;
    }
  }

  // idx -= max_sticks;

  // auto max_pots = adcGetMaxInputs(ADC_INPUT_POT);
  // if (idx < max_pots)
  //   return opentxSim->knobs[idx]->getValue();

  // idx -= max_pots;

  // auto max_axes = adcGetMaxInputs(ADC_INPUT_AXIS);
  // if (idx < max_axes) return 0;

  // probably RTC_BAT
  return 0;
}
