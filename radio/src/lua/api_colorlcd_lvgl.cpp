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

#define LUA_LIB

#include "lua_api.h"
#include "lua_widget.h"
#include "edgetx.h"

#include "api_colorlcd.h"

class LvglWidgetParams
{
 public:
  LvglWidgetParams(lua_State *L, int index = 1)
  {
    luaL_checktype(L, index, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      const char *key = lua_tostring(L, -2);
      if (!strcmp(key, "type")) {
        if (lua_isinteger(L, -1)) {
          int n = lua_tointeger(L, -1);
          if (n > ETX_UNDEF && n < ETX_LAST) {
            type = (LuaLvglType)n;
          } else {
            luaL_error(L, "Invalid type '%d'", n);
            type = ETX_UNDEF;
          }
        } else {
          const char* s = luaL_checkstring(L, -1);
          type = getType(s);
          if (type == ETX_UNDEF)
            luaL_error(L, "Invalid type '%s'", s);
        }
      } else if (!strcmp(key, "name")) {
        name = luaL_checkstring(L, -1);
      } else if (!strcmp(key, "children")) {
        hasChildren = true;
      }
    }
  }

  LuaLvglType getType(const char* s)
  {
    if (strcasecmp(s, "label") == 0) return ETX_LABEL;
    if (strcasecmp(s, "rectangle") == 0) return ETX_RECTANGLE;
    if (strcasecmp(s, "circle") == 0) return ETX_CIRCLE;
    if (strcasecmp(s, "arc") == 0) return ETX_ARC;
    if (strcasecmp(s, "hline") == 0) return ETX_HLINE;
    if (strcasecmp(s, "vline") == 0) return ETX_VLINE;
    if (strcasecmp(s, "line") == 0) return ETX_LINE;
    if (strcasecmp(s, "triangle") == 0) return ETX_TRIANGLE;
    if (strcasecmp(s, "image") == 0) return ETX_IMAGE;
    if (strcasecmp(s, "qrcode") == 0) return ETX_QRCODE;
    if (strcasecmp(s, "box") == 0) return ETX_BOX;
    if (strcasecmp(s, "button") == 0) return ETX_BUTTON;
    if (strcasecmp(s, "momentaryButton") == 0) return ETX_MOMENTARY_BUTTON;
    if (strcasecmp(s, "toggle") == 0) return ETX_TOGGLE;
    if (strcasecmp(s, "textEdit") == 0) return ETX_TEXTEDIT;
    if (strcasecmp(s, "numberEdit") == 0) return ETX_NUMBEREDIT;
    if (strcasecmp(s, "choice") == 0) return ETX_CHOICE;
    if (strcasecmp(s, "slider") == 0) return ETX_SLIDER;
    if (strcasecmp(s, "verticalSlider") == 0) return ETX_VERTICAL_SLIDER;
    if (strcasecmp(s, "page") == 0) return ETX_PAGE;
    if (strcasecmp(s, "font") == 0) return ETX_FONT;
    if (strcasecmp(s, "align") == 0) return ETX_ALIGN;
    if (strcasecmp(s, "color") == 0) return ETX_COLOR;
    if (strcasecmp(s, "timer") == 0) return ETX_TIMER;
    if (strcasecmp(s, "switch") == 0) return ETX_SWITCH;
    if (strcasecmp(s, "source") == 0) return ETX_SOURCE;
    if (strcasecmp(s, "file") == 0) return ETX_FILE;
    if (strcasecmp(s, "setting") == 0) return ETX_SETTING;
    return ETX_UNDEF;
  }

  LuaLvglType type = ETX_UNDEF;
  const char *name = nullptr;
  bool hasChildren = false;
};

static int luaLvglPopup(lua_State *L, std::function<LvglWidgetObjectBase*()> create)
{
  auto obj = create();
  obj->create(L, 1);
  return 0;
}

static int luaDestroyLvglWidget(lua_State *L)
{
  auto p = LvglWidgetObjectBase::checkLvgl(L, 1);
  if (p) {
    p->clearRefs(L);
    delete p;
  }
  return 0;
}

/*luadoc
@function lvgl.set(parent, params)

Update the settings of an existing LVGL object. Can also be used to change settings that have no dedicated update function (e.g. rectangle thickness). Can also be called as `parent:set(params)`.

@param parent (object) LVGL object to update; unlike the constructor functions, this parameter is required, not optional

@param params (table) settings to update; use the same keys documented for the object's constructor function

@retval none

@status current Introduced in 2.11.0
*/
static int luaLvglSet(lua_State *L)
{
  auto p = LvglWidgetObjectBase::checkLvgl(L, 1, true);
  if (p) {
    p->update(L);
  }
  return 0;
}

/*luadoc
@function lvgl.clear([parent])

Delete LVGL objects.

@param parent (object) optional LVGL object whose children should be deleted; if omitted, the entire script UI is deleted

@retval none

@notice If `parent` is not set, the entire script UI is deleted -- use this when the UI changes dramatically (e.g. widget size changes).
@notice If `parent` is set, only its child LVGL objects are deleted -- use this to remove and replace a specific set of objects.

@status current Introduced in 2.11.0
*/
static int luaLvglClear(lua_State *L)
{
  if (luaScriptManager) {
    if (lua_gettop(L) == 1) {
      auto p = LvglWidgetObjectBase::checkLvgl(L, 1, true);
      if (p) {
        p->clear();
      }
    } else {
      luaScriptManager->clear();
    }
  }

  return 0;
}

/*luadoc
@function lvgl.show(parent)

Show an LVGL object.

@param parent (object) LVGL object to show

@retval none

@notice The `visible` setting function can also be used to dynamically show and hide objects.

@status current Introduced in 2.11.0
*/
static int luaLvglShow(lua_State *L)
{
  auto p = LvglWidgetObjectBase::checkLvgl(L, 1, true);
  if (p) {
    p->show();
  }
  return 0;
}

/*luadoc
@function lvgl.hide(parent)

Hide an LVGL object.

@param parent (object) LVGL object to hide

@retval none

@notice The `visible` setting function can also be used to dynamically show and hide objects.

@status current Introduced in 2.11.0
*/
static int luaLvglHide(lua_State *L)
{
  auto p = LvglWidgetObjectBase::checkLvgl(L, 1, true);
  if (p) {
    p->hide();
  }
  return 0;
}

/*luadoc
@function lvgl.enable(obj)

Enable interaction for an LVGL control object.

@param obj (object) LVGL object previously created by the `lvgl` library

@retval none

@status current Introduced in 2.11.0
*/
static int luaLvglEnable(lua_State *L)
{
  auto p = LvglWidgetObjectBase::checkLvgl(L, 1, true);
  if (p) {
    p->enable();
  }
  return 0;
}

/*luadoc
@function lvgl.disable(obj)

Disable interaction for an LVGL control object.

@param obj (object) LVGL object previously created by the `lvgl` library

@retval none

@status current Introduced in 2.11.0
*/
static int luaLvglDisable(lua_State *L)
{
  auto p = LvglWidgetObjectBase::checkLvgl(L, 1, true);
  if (p) {
    p->disable();
  }
  return 0;
}

/*luadoc
@function lvgl.close(obj)

Close an LVGL object that represents a closable container or dialog.

@param obj (object) LVGL object previously created by the `lvgl` library

@retval none

@status current Introduced in 2.11.0
*/
static int luaLvglClose(lua_State *L)
{
  auto p = LvglWidgetObjectBase::checkLvgl(L, 1, true);
  if (p) {
    p->close();
  }
  return 0;
}

static void buildLvgl(lua_State *L, int srcIndex, int refIndex)
{
  luaL_checktype(L, srcIndex, LUA_TTABLE);
  for (lua_pushnil(L); lua_next(L, srcIndex - 1); lua_pop(L, 1)) {
    auto t = lua_gettop(L);
    LvglWidgetParams p(L, -1);
    if (p.type == ETX_UNDEF)
      luaL_error(L, "Missing or bad type");
    if (p.type >= ETX_FIRST_CONTROL && !luaScriptManager->isFullscreen())
      continue;
    LvglWidgetObjectBase *obj = nullptr;
    switch (p.type) {
      case ETX_LABEL:
        obj = new LvglWidgetLabel();
        break;
      case ETX_RECTANGLE:
        obj = new LvglWidgetRectangle();
        break;
      case ETX_CIRCLE:
        obj = new LvglWidgetCircle();
        break;
      case ETX_ARC:
        obj = new LvglWidgetArc();
        break;
      case ETX_HLINE:
        obj = new LvglWidgetHLine();
        break;
      case ETX_VLINE:
        obj = new LvglWidgetVLine();
        break;
      case ETX_LINE:
        obj = new LvglWidgetLine();
        break;
      case ETX_TRIANGLE:
        obj = new LvglWidgetTriangle();
        break;
      case ETX_IMAGE:
        obj = new LvglWidgetImage();
        break;
      case ETX_QRCODE:
        obj = new LvglWidgetQRCode();
        break;
      case ETX_BOX:
        obj = new LvglWidgetBox();
        break;
      case ETX_BUTTON:
        obj = new LvglWidgetTextButton();
        break;
      case ETX_MOMENTARY_BUTTON:
        obj = new LvglWidgetMomentaryButton();
        break;
      case ETX_TOGGLE:
        obj = new LvglWidgetToggleSwitch();
        break;
      case ETX_TEXTEDIT:
        obj = new LvglWidgetTextEdit();
        break;
      case ETX_NUMBEREDIT:
        obj = new LvglWidgetNumberEdit();
        break;
      case ETX_CHOICE:
        obj = new LvglWidgetChoice();
        break;
      case ETX_SLIDER:
        obj = new LvglWidgetSlider();
        break;
      case ETX_VERTICAL_SLIDER:
        obj = new LvglWidgetVerticalSlider();
        break;
      case ETX_PAGE:
        obj = new LvglWidgetPage();
        break;
      case ETX_FONT:
        obj = new LvglWidgetFontPicker();
        break;
      case ETX_ALIGN:
        obj = new LvglWidgetAlignPicker();
        break;
      case ETX_COLOR:
        obj = new LvglWidgetColorPicker();
        break;
      case ETX_TIMER:
        obj = new LvglWidgetTimerPicker();
        break;
      case ETX_SWITCH:
        obj = new LvglWidgetSwitchPicker();
        break;
      case ETX_SOURCE:
        obj = new LvglWidgetSourcePicker();
        break;
      case ETX_FILE:
        obj = new LvglWidgetFilePicker();
        break;
      case ETX_SETTING:
        obj = new LvglWidgetSetting();
        break;
      default:
        continue;
    }
    if (obj) {
      obj->create(L, -1);
      auto ref = obj->getRef(L);
      if (p.name && refIndex != 0) {
        lua_pushstring(L, p.name);
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        lua_settable(L, refIndex - 4);
      }
      if (p.hasChildren && obj->getWindow()) {
        lua_getfield(L, -1, "children");
        auto prevParent = luaScriptManager->getTempParent();
        luaScriptManager->setTempParent(obj);
        buildLvgl(L, -1, (refIndex != 0) ? refIndex - 3 : LUA_REFNIL);
        lua_pop(L, 1);
        luaScriptManager->setTempParent(prevParent);
      }
    }
    lua_settop(L, t); // In case of errors in build functions
  }
}

static void addChildren(lua_State *L, LvglWidgetObjectBase* obj)
{
  if (obj->getWindow()) {
    lua_getfield(L, -1, "children");
    auto prevParent = luaScriptManager->getTempParent();
    luaScriptManager->setTempParent(obj);
    buildLvgl(L, -1, 0);
    lua_pop(L, 1);
    luaScriptManager->setTempParent(prevParent);
  }
}

static int luaLvglObj(lua_State *L, std::function<LvglWidgetObject*()> create, bool fullscreenOnly = false)
{
  if (luaScriptManager && (!fullscreenOnly || luaScriptManager->isFullscreen())) {
    LvglWidgetParams params(L, 1);

    auto obj = create();
    obj->create(L, 1);

    if (params.hasChildren) addChildren(L, obj);

    obj->push(L);
  } else {
    lua_pushnil(L);
  }

  return 1;
}

static int luaLvglObjEx(lua_State *L, std::function<LvglWidgetObjectBase*()> create, bool fullscreenOnly = false)
{
  if (luaScriptManager && (!fullscreenOnly || luaScriptManager->isFullscreen())) {
    LvglWidgetObjectBase* p = nullptr;
    LvglWidgetObjectBase* prevParent = nullptr;
    if (lua_gettop(L) == 2) {
      p = LvglWidgetObjectBase::checkLvgl(L, 1, true);
      if (p) {
        prevParent = luaScriptManager->getTempParent();
        luaScriptManager->setTempParent(p);
      }
    }

    LvglWidgetParams params(L, -1);

    auto obj = create();
    obj->create(L, -1);

    if (params.hasChildren) addChildren(L, obj);

    obj->push(L);

    if (p)
      luaScriptManager->setTempParent((prevParent));
  } else {
    lua_pushnil(L);
  }

  return 1;
}

/*luadoc
@function lvgl.build([parent, ]params)

Build a complex UI in a single operation from a table of object definitions.

@param parent (object) optional LVGL object to attach the built objects to; if omitted they attach to the top-level script window

@param params (table) a table of tables, each defining one LVGL object:
 * `type` (string or type constant) mandatory, selects the kind of object to create, e.g. `"rectangle"` or `lvgl.RECTANGLE`; named type constants (e.g. `lvgl.RECTANGLE`) added in 2.11.4, string names work in all versions
 * `name` (string) optional name used to look up this object in the returned table, defaults to an empty string
 * `children` (table) optional nested table of the same shape, to create child objects
 * any other key accepted by the constructor function for the chosen `type`

@retval table named LVGL objects (only entries whose definition included a `name` are included)

@notice Objects can be nested via `children`, another table of tables using the same shape as the top-level `params`.
@notice Very large or deeply nested tables may fail to compile to `.luac` -- if a script works from `.lua` but fails from `.luac`, split the call into multiple smaller `lvgl.build()` calls.

@status current Introduced in 2.11.0, named type constants added in 2.11.4
*/
static int luaLvglBuild(lua_State *L)
{
  if (luaScriptManager) {
    LvglWidgetObjectBase* p = nullptr;
    LvglWidgetObjectBase* prevParent = nullptr;
    if (lua_gettop(L) == 2) {
      p = LvglWidgetObjectBase::checkLvgl(L, 1, true);
      if (p) {
        prevParent = luaScriptManager->getTempParent();
        luaScriptManager->setTempParent(p);
      }
    }

    // Return array of lvgl object references
    lua_newtable(L);
    buildLvgl(L, -2, -1);

    if (p)
      luaScriptManager->setTempParent((prevParent));
  } else {
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function lvgl.isAppMode()

Test if the script is running in App Mode.

@retval boolean true if running in App Mode, false otherwise

@status current Introduced in 2.11.0
*/
static int luaLvglIsAppMode(lua_State *L)
{
  if (luaScriptManager) {
    lua_pushboolean(L, luaScriptManager->isAppMode());
  } else {
    lua_pushboolean(L, false);
  }
  return 1;
}

/*luadoc
@function lvgl.isFullScreen()

Test if the script is running in full screen mode.

@retval boolean true if running in full screen mode, false otherwise

@notice Always returns true for stand alone tool scripts.

@status current Introduced in 2.11.0
*/
static int luaLvglIsFullscreen(lua_State *L)
{
  if (luaScriptManager) {
    lua_pushboolean(L, luaScriptManager->isFullscreen());
  } else {
    lua_pushboolean(L, false);
  }
  return 1;
}

/*luadoc
@function lvgl.exitFullScreen()

If the script is running in full screen mode, return it to normal mode.

@retval none

@status current Introduced in 2.11.0
*/
static int luaLvglExitFullscreen(lua_State *L)
{
  if (luaScriptManager)
    luaScriptManager->exitFullscreen();
  return 0;
}

/*luadoc
@function lvgl.getContext()

For a widget script, returns the local instance table created by (and returned from) the script's `create()` function.

@retval table the widget instance's local data table, or nil for stand alone tool scripts

@status current Introduced in 2.11.0
*/
static int luaLvglGetContext(lua_State *L)
{
  if (luaScriptManager && luaScriptManager->getContext() != LUA_REFNIL) {
    // Push context tanle onto Lua stack (return object)
    lua_rawgeti(L, LUA_REGISTRYINDEX, luaScriptManager->getContext());
  } else {
    lua_pushnil(L);
  }
  return 1;
}

/*luadoc
@function lvgl.getScrollPos(parent)

Get the current scroll position of a container object.

@param parent (object) LVGL container object (e.g. `box`, `rectangle`, or `page`)

@retval number, number the X and Y scroll position of the container's top-left corner

@notice Only applies to container objects such as `box`, `rectangle`, and `page`.

@status current Introduced in 2.11.2
*/
static int luaLvglGetScrollPos(lua_State *L)
{
  auto p = LvglWidgetObjectBase::checkLvgl(L, 1, true);
  if (p) {
    lua_pushinteger(L, p->getScrollX());
    lua_pushinteger(L, p->getScrollY());
    return 2;
  }
  return 0;
}

extern "C" {
// lvgl functions
LROT_BEGIN(lvgllib, NULL, 0)
  LROT_FUNCENTRY(clear, luaLvglClear)
  LROT_FUNCENTRY(build, luaLvglBuild)
  LROT_FUNCENTRY(isAppMode, luaLvglIsAppMode)
  LROT_FUNCENTRY(isFullScreen, luaLvglIsFullscreen)
  LROT_FUNCENTRY(exitFullScreen, luaLvglExitFullscreen)
  LROT_FUNCENTRY(getContext, luaLvglGetContext)
  /*luadoc
  @common lvgl_object

   * `x` (number) position relative to the top-left of the parent
   * `y` (number) position relative to the top-left of the parent
   * `w` (number) width
   * `h` (number) height
   * `color` (color or function) primary color for the object
   * `pos` (function) dynamic position callback returning `x, y`
   * `size` (function) dynamic size callback returning `w, h`
   * `visible` (function) dynamic visibility callback
   * `floating` (boolean) keep the object fixed inside a scrollable container, added in 2.11.6, defaults to false
  */
  // Objects - widgets and standalone scripts
  /*luadoc
  @function lvgl.label([parent, ]params)

  Display a text label.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `text` (string or function) text to display, defaults to an empty string
   * `font` (font value or function) sets the font size, e.g. `MIDSIZE`, `DBLSIZE`; defaults to `STDSIZE`
   * `align` (text alignment value or function) sets the text justification, e.g. `RIGHT`, `VCENTER`; defaults to `LEFT`. `RIGHT`/`CENTER` require `w` to be set. `LEFT`/`RIGHT`/`CENTER` align text horizontally within the label's bounding box `(x,y,w,h)`; `VCENTER`/`VTOP`/`VBOTTOM` align the label's bounding box vertically within the parent object

  @retval table LVGL object

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(label, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetLabel(); }); })
  /*luadoc
  @function lvgl.rectangle([parent, ]params)

  Display a rectangle. Based on `lvgl.box`, so all `lvgl.box` settings also apply.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `thickness` (number) width of the border line, defaults to 1
   * `filled` (boolean or function) if true the rectangle is filled with `color`, defaults to false (function support added in 2.11.4)
   * `rounded` (number) if greater than 0, rounds the corners with this radius; must be >= `thickness` if set, defaults to 0
   * `opacity` (number or function) opacity from 0 (transparent) to 255 (opaque), defaults to 255

  @retval table LVGL object

  @notice When used in a stand alone tool script, the rectangle automatically adds scroll bars if any child objects are placed outside its boundaries. For widgets, child objects outside the bounds are clipped instead.

  @status current Introduced in 2.11.0, filled function support added in 2.11.4, floating added in 2.11.6
  */
  LROT_FUNCENTRY(rectangle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetRectangle(); }); })
  /*luadoc
  @function lvgl.hline([parent, ]params)

  Display a horizontal line.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  `w` sets the length of the line; `h` sets its thickness.

  @param params (table):
   @commonparams lvgl_object
   * `rounded` (boolean) if true the line's end caps are rounded, defaults to false
   * `opacity` (number or function) opacity from 0 (transparent) to 255 (opaque), defaults to 255
   * `dashGap` (number) gap size for dashed lines, defaults to 0
   * `dashWidth` (number) dash size for dashed lines, defaults to 0; both `dashGap` and `dashWidth` must be > 0 to draw dashed lines

  @retval table LVGL object

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(hline, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetHLine(); }); })
  /*luadoc
  @function lvgl.vline([parent, ]params)

  Display a vertical line.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  `h` sets the length of the line; `w` sets its thickness.

  @param params (table):
   @commonparams lvgl_object
   * `rounded` (boolean) if true the line's end caps are rounded, defaults to false
   * `opacity` (number or function) opacity from 0 (transparent) to 255 (opaque), defaults to 255
   * `dashGap` (number) gap size for dashed lines, defaults to 0
   * `dashWidth` (number) dash size for dashed lines, defaults to 0; both `dashGap` and `dashWidth` must be > 0 to draw dashed lines

  @retval table LVGL object

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(vline, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetVLine(); }); })
  /*luadoc
  @function lvgl.line([parent, ]params)

  Display one or more connected line segments.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  `x`, `y`, `w`, and `h` are not used.

  @param params (table):
   @commonparams lvgl_object
   * `rounded` (boolean) if true the line's end caps are rounded, defaults to false
   * `thickness` (number) thickness of each line segment, defaults to 1
   * `opacity` (number or function) opacity from 0 (transparent) to 255 (opaque), defaults to 255
   * `pts` (table or function) table of points, each a `{x, y}` pair; at least two points are required, defaults to nil

  @retval table LVGL object

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(line, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetLine(); }); })
  /*luadoc
  @function lvgl.triangle([parent, ]params)

  Display a filled triangle.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  `x`, `y`, `w`, and `h` are not used.

  @param params (table):
   @commonparams lvgl_object
   * `opacity` (number or function) opacity from 0 (transparent) to 255 (opaque), defaults to 255
   * `pts` (table or function) table of exactly three points, each a `{x, y}` pair, defaults to nil

  @retval table LVGL object

  @notice Unlike the line-drawing objects, LVGL has no built-in triangle primitive. The drawing method used is simple and does not anti-alias.

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(triangle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTriangle(); }); })
  /*luadoc
  @function lvgl.circle([parent, ]params)

  Display a solid or outlined circle.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  `w`, `h`, and `size` should not be used with `lvgl.circle` -- use `radius` instead.

  @param params (table):
   @commonparams lvgl_object
   * `thickness` (number) width of the border line, defaults to 1
   * `filled` (boolean) if true the circle is filled with `color`, defaults to false
   * `radius` (number or function) radius of the circle, defaults to 0
   * `opacity` (number or function) opacity from 0 (transparent) to 255 (opaque), defaults to 255

  @retval table LVGL object

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(circle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetCircle(); }); })
  /*luadoc
  @function lvgl.arc([parent, ]params)

  Display an arc.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  `w`, `h`, and `size` should not be used with `lvgl.arc` -- use `radius` instead.

  @param params (table):
   @commonparams lvgl_object
   * `thickness` (number) width of the arc's line, defaults to 1
   * `radius` (number or function) radius of the arc, defaults to 0
   * `startAngle` (number or function) starting angle in degrees (0-360), 0 is 3 o'clock, defaults to 0
   * `endAngle` (number or function) ending angle in degrees, defaults to 360
   * `opacity` (number or function) opacity from 0 (transparent) to 255 (opaque), defaults to 255
   * `rounded` (boolean) if true the ends of the arc are rounded, defaults to false
   * `bgColor` (color or function) color of the background arc, not used by default
   * `bgOpacity` (number or function) opacity of the background arc, defaults to 0 (not visible)
   * `bgStartAngle` (number or function) starting angle of the background arc in degrees, defaults to 0
   * `bgEndAngle` (number or function) ending angle of the background arc in degrees, defaults to 360

  @retval table LVGL object

  @notice Arc objects have two elements, a foreground arc and a background arc. By default the background arc is not shown -- to show it, set both `bgColor` and `bgOpacity`.
  @notice If `opacity` or `bgOpacity` is less than 255 and `rounded` is true, the ends of the arc will not draw correctly. This is an LVGL limitation.

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(arc, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetArc(); }); })
  /*luadoc
  @function lvgl.image([parent, ]params)

  Display an image, centered in the frame `(x, y, w, h)`. Images can be scaled to fit entirely within the frame or to fill it completely.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `file` (string) full path to the image file on the SD card, defaults to an empty string
   * `fill` (boolean) if true the image is scaled to completely fill the frame (may be cropped); if false it is scaled to fit entirely within the frame (may have empty borders), defaults to false

  @retval table LVGL object

  @notice `w` and `h` are required -- `lvgl.PERCENT_SIZE` does not work for images, unlike most other objects.

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(image, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetImage(); }); })
  /*luadoc
  @function lvgl.qrcode([parent, ]params)

  Display a QR code.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  `w` and `h` should be set to the same value for a QR code.

  @param params (table):
   @commonparams lvgl_object
   * `data` (string) URL or other content to encode, defaults to an empty string
   * `bgColor` (color) background color for the QR code image, defaults to `COLOR_THEME_SECONDARY3`

  @retval table LVGL object

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(qrcode, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetQRCode(); }); })
  // Objects - standalone scripts and full screen widgets only
  /*luadoc
  @function lvgl.button([parent, ]params)

  Add a text button using the EdgeTX style.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `text` (string or function) text displayed in the button, defaults to an empty string
   * `press` (function) called when the user taps the button, after releasing ENTER or the screen; may optionally return a non-zero integer to display the button in the checked state, defaults to nil
   * `longpress` (function) called on a long tap; may optionally return a non-zero integer to display the button in the checked state, defaults to nil
   * `active` (function) sets the enabled/disabled state; must return a boolean, true to enable, defaults to nil
   * `checked` (boolean) sets the initial checked state; can be updated with `lvgl.set()`, defaults to false
   * `color` (color or function) background color, defaults to the EdgeTX button style PRIMARY2 theme color
   * `textColor` (color or function) text color, defaults to the EdgeTX button style SECONDARY1 theme color
   * `cornerRadius` (number) radius for the button's corners, defaults to the EdgeTX button style radius
   * `font` (font value or function) sets the font size, e.g. `MIDSIZE`, `DBLSIZE`; defaults to `STDSIZE`

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(button, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTextButton(); }, true); })
  /*luadoc
  @function lvgl.momentaryButton([parent, ]params)

  Add a momentary text button using the EdgeTX style.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `text` (string or function) text displayed in the button, defaults to an empty string
   * `press` (function) called immediately when the user first taps the button or presses ENTER, defaults to nil
   * `release` (function) called when the user releases ENTER or stops touching the screen, defaults to nil
   * `active` (function) sets the enabled/disabled state; must return a boolean, true to enable, defaults to nil
   * `color` (color or function) background color, defaults to the EdgeTX button style PRIMARY2 theme color
   * `textColor` (color or function) text color, defaults to the EdgeTX button style SECONDARY1 theme color
   * `cornerRadius` (number) radius for the button's corners, defaults to the EdgeTX button style radius
   * `font` (font value or function) sets the font size, e.g. `MIDSIZE`, `DBLSIZE`; defaults to `STDSIZE`

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @notice Unlike `lvgl.button`, `press` fires immediately on tap/press rather than on release. The button shows the checked state until ENTER is released or the screen is no longer touched.

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(momentaryButton, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetMomentaryButton(); }, true); })
  /*luadoc
  @function lvgl.toggle([parent, ]params)

  Add a toggle switch using the EdgeTX style.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `get` (function) called to get the current state; must return a boolean or number (0/false = off, 1/true = on), defaults to nil
   * `set` (function) called when the user interacts with the switch; passed a single number, 0 = off, 1 = on, defaults to nil
   * `active` (function) sets the enabled/disabled state; must return a boolean, true to enable, defaults to nil

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(toggle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetToggleSwitch(); }, true); })
  /*luadoc
  @function lvgl.textEdit([parent, ]params)

  Add a text edit box using the EdgeTX style.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `value` (string or function) text to edit; if a function, the text can be changed by both script and user, defaults to an empty string
   * `length` (number) maximum editable text length, 1 to 128, defaults to 32
   * `set` (function) called when the user edits the text; passed the new string content, defaults to nil
   * `active` (function) sets the enabled/disabled state; must return a boolean, true to enable, defaults to nil

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(textEdit, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTextEdit(); }, true); })
  /*luadoc
  @function lvgl.numberEdit([parent, ]params)

  Add a number edit box using the EdgeTX style.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `get` (function) called to get the current value to display, defaults to nil
   * `set` (function) called for every change to the number while editing; passed the new value, defaults to nil
   * `edited` (function) called after the user finishes editing; passed the final value (added in 2.11.5), defaults to nil
   * `active` (function) sets the enabled/disabled state; must return a boolean, true to enable, defaults to nil
   * `min` (number) minimum allowed value, defaults to -1024
   * `max` (number) maximum allowed value, defaults to 1024
   * `display` (function) overrides how the value is displayed; passed the current value, must return a string, defaults to nil

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @notice Use `set` to react to every change during editing, or `edited` to only react to the final value once editing is complete.

  @status current Introduced in 2.11.0, edited added in 2.11.5, floating added in 2.11.6
  */
  LROT_FUNCENTRY(numberEdit, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetNumberEdit(); }, true); })
  /*luadoc
  @function lvgl.choice([parent, ]params)

  Display a button showing an option value. Tapping it opens a popup menu of options to choose from, using EdgeTX styling.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `title` (string) text displayed in the popup menu's header, defaults to an empty string
   * `values` (table) simple table of option strings shown in the popup; can be changed via `lvgl.set()` since 2.11.6, defaults to an empty list
   * `get` (function) called on every refresh cycle to keep the displayed value in sync, to get the index of the currently selected option (1..number of values), defaults to nil
   * `set` (function) called when the user taps a menu item; passed the selected item's index (1..number of values), defaults to nil
   * `active` (function) sets the enabled/disabled state; must return a boolean, true to enable, defaults to nil
   * `filter` (function) called for each option when the popup opens, passed the option's index; return true to show it, false to hide it, defaults to nil
   * `popupWidth` (number) width of the popup window, defaults to 0 (use default width)

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @notice The popup closes and `set` is called when the user selects an item.
  @notice If the user taps outside the popup or presses RTN, the popup closes without calling `set`.

  @status current Introduced in 2.11.0, values updatable via lvgl.set() added in 2.11.6, floating added in 2.11.6
  */
  LROT_FUNCENTRY(choice, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetChoice(); }, true); })
  /*luadoc
  @function lvgl.slider([parent, ]params)

  Add a horizontal slider using the EdgeTX style.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  `w` sets the size of the slider; `h` is not used.

  @param params (table):
   @commonparams lvgl_object
   * `min` (number) minimum value at the left end of the range, defaults to 0
   * `max` (number) maximum value at the right end of the range, defaults to 100
   * `get` (function) called to get the current knob value; should return a value between `min` and `max` inclusive, defaults to nil
   * `set` (function) called when the user moves the knob; passed the new value, defaults to nil
   * `active` (function) sets the enabled/disabled state; must return a boolean, true to enable, defaults to nil

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(slider, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetSlider(); }, true); })
  /*luadoc
  @function lvgl.verticalSlider([parent, ]params)

  Add a vertical slider using the EdgeTX style.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  `h` sets the size of the slider; `w` is not used.

  @param params (table):
   @commonparams lvgl_object
   * `min` (number) minimum value at the bottom end of the range, defaults to 0
   * `max` (number) maximum value at the top end of the range, defaults to 100
   * `get` (function) called to get the current knob value; should return a value between `min` and `max` inclusive, defaults to nil
   * `set` (function) called when the user moves the knob; passed the new value, defaults to nil
   * `active` (function) sets the enabled/disabled state; must return a boolean, true to enable, defaults to nil

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(verticalSlider, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetVerticalSlider(); }, true); })
  /*luadoc
  @function lvgl.font([parent, ]params)

  Display a button showing a font name. Tapping it opens a popup menu to choose a font from, using EdgeTX styling.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `get` (function) called on every refresh cycle to keep the displayed value in sync, to get the currently selected font, defaults to nil
   * `set` (function) called when the user taps a font in the popup; passed the selected font value, defaults to nil
   * `active` (function) sets the enabled/disabled state; must return a boolean, true to enable, defaults to nil

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @notice The popup closes and `set` is called when the user selects an item.
  @notice If the user taps outside the popup or presses RTN, the popup closes without calling `set`.

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(font, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetFontPicker(); }, true); })
  /*luadoc
  @function lvgl.align([parent, ]params)

  Display a button showing a text alignment name. Tapping it opens a popup menu to choose an alignment from, using EdgeTX styling.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `get` (function) called on every refresh cycle to keep the displayed value in sync, to get the currently selected alignment, defaults to nil
   * `set` (function) called when the user taps an alignment in the popup; passed the selected alignment value, defaults to nil

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @notice The popup closes and `set` is called when the user selects an item.
  @notice If the user taps outside the popup or presses RTN, the popup closes without calling `set`.

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(align, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetAlignPicker(); }, true); })
  /*luadoc
  @function lvgl.color([parent, ]params)

  Display a button showing a color swatch. Tapping it opens the color picker dialog, using EdgeTX styling.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `get` (function) called when the picker opens, to get the currently selected color, defaults to nil
   * `set` (function) called when the user picks a color; passed the selected color value, defaults to nil

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @notice The popup closes and `set` is called when the user selects a color.
  @notice If the user taps outside the popup or presses RTN, the popup closes without calling `set`.

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(color, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetColorPicker(); }, true); })
  /*luadoc
  @function lvgl.timer([parent, ]params)

  Display a button showing a timer name. Tapping it opens a popup menu to choose a timer from, using EdgeTX styling.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `get` (function) called on every refresh cycle to keep the displayed value in sync, to get the currently selected timer, defaults to nil
   * `set` (function) called when the user taps a timer in the popup; passed the selected timer value, defaults to nil
   * `active` (function) sets the enabled/disabled state; must return a boolean, true to enable, defaults to nil

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @notice The popup closes and `set` is called when the user selects an item.
  @notice If the user taps outside the popup or presses RTN, the popup closes without calling `set`.

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(timer, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTimerPicker(); }, true); })
  /*luadoc
  @function lvgl.switch([parent, ]params)

  Display a button showing a switch name. Tapping it opens the switch-select popup, using EdgeTX styling.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `get` (function) called on every refresh cycle to keep the displayed value in sync, to get the currently selected switch, defaults to nil
   * `set` (function) called when the user picks a switch; passed the selected switch value, defaults to nil
   * `active` (function) sets the enabled/disabled state; must return a boolean, true to enable, defaults to nil
   * `filter` (number) controls which switch types the user can pick; combine `lvgl.SW_xxx` constants to build a custom filter, defaults to `lvgl.SW_ALL`

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @notice The popup closes and `set` is called when the user selects an item.
  @notice If the user taps outside the popup or presses RTN, the popup closes without calling `set`.

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(switch, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetSwitchPicker(); }, true); })
  /*luadoc
  @function lvgl.source([parent, ]params)

  Display a button showing a source name. Tapping it opens the source-select popup, using EdgeTX styling.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `get` (function) called on every refresh cycle to keep the displayed value in sync, to get the currently selected source, defaults to nil
   * `set` (function) called when the user picks a source; passed the selected source value, defaults to nil
   * `filter` (number) controls which source types the user can pick; combine `lvgl.SRC_xxx` constants to build a custom filter, defaults to `lvgl.SRC_ALL`

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @notice The popup closes and `set` is called when the user selects an item.
  @notice If the user taps outside the popup or presses RTN, the popup closes without calling `set`.

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(source, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetSourcePicker(); }, true); })
  /*luadoc
  @function lvgl.file([parent, ]params)

  Display a button showing a filename. Tapping it opens a popup file picker, using EdgeTX styling.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `title` (string) text displayed in the popup's header, defaults to an empty string
   * `get` (function) called on every refresh cycle to keep the displayed value in sync, to get the currently selected filename; must return a string, defaults to nil
   * `set` (function) called when the user picks a file; passed the selected filename (without path), defaults to nil
   * `active` (function) sets the enabled/disabled state; must return a boolean, true to enable, defaults to nil
   * `folder` (string) SD card folder to browse for files, defaults to nil
   * `extension` (string) extension filter, e.g. `.png` or concatenated `.png.bmp` to match multiple, defaults to nil (no filter)
   * `hideExtension` (boolean) if true, the extension is stripped before `set` is called and hidden in the picker list, defaults to false
   * `maxLen` (number) maximum filename length (measured after `hideExtension` stripping, if set); files with longer names are not shown in the picker list, defaults to 255

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @notice The popup closes and `set` is called when the user picks a file.
  @notice If the user taps outside the popup or presses RTN, the popup closes without calling `set`.

  @status current Introduced in 2.11.0, floating added in 2.11.6
  */
  LROT_FUNCENTRY(file, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetFilePicker(); }, true); })
  // Containers
  /*luadoc
  @function lvgl.box([parent, ]params)

  Create a container for managing object layout.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `flexFlow` (lvgl.FLOW_COLUMN or lvgl.FLOW_ROW) enables flex layout for this box, not used by default
   * `flexPad` (number) padding between rows/columns when flex layout is used; recommend using `lvgl.PAD_xxx` values, defaults to `PAD_OUTLINE`
   * `scrollBar` (boolean) enables/disables scroll bars, defaults to true
   * `scrollDir` (lvgl.SCROLL_xx) allowed scroll directions when child objects extend past the box bounds (stand alone scripts only), defaults to `lvgl.SCROLL_ALL`
   * `scrolled` (function) called when the box content is scrolled; passed `x`, `y` of the current scroll position, defaults to nil
   * `scrollTo` (function) overrides the box scroll position; must return `x`, `y` to scroll to, defaults to nil
   * `align` (alignment type (LEFT, RIGHT, CENTER, VTOP, VBOTTOM, VCENTER)) alignment used with flex layouts (added in 2.11.4); avoid combining an alignment with a scrollbar on the same axis, defaults to `CENTER | VTOP`
   * `borderPad` (number or table) border padding around the container edges; a single number applies to all sides, or a table `{left=?, right=?, top=?, bottom=?}` (added in 2.11.5), defaults to `PAD_OUTLINE` if `flexFlow` is set, otherwise 0

  @retval table LVGL object

  @notice The box is a helper for managing screen layouts.
  @notice When adding controls such as button/toggle/textEdit, leave enough space around them for the focus outline.
  @notice In a stand alone tool script, the box automatically adds scroll bars if child objects fall outside its boundaries; for widgets, out-of-bounds children are clipped instead.
  @notice `scrollBar`, `scrollDir`, `scrolled`, and `scrollTo` were added in 2.11.2.

  @status current Introduced in 2.11.0, align added in 2.11.4, borderPad added in 2.11.5, floating added in 2.11.6, scrollBar/scrollDir/scrolled/scrollTo added in 2.11.2
  */
  LROT_FUNCENTRY(box, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetBox(); }); })
  /*luadoc
  @function lvgl.setting([parent, ]params)

  Create a container for a single row of a settings page, automatically padded for controls such as toggle/textEdit/button.

  @param parent (object) optional LVGL object to attach the new object to; if omitted the object is created in the top-level script window

  @param params (table):
   @commonparams lvgl_object
   * `title` (string or function) text displayed on the left; function support added in 2.11.6, defaults to an empty string

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @status current Introduced in 2.11.0, title function support added in 2.11.6, floating added in 2.11.6
  */
  LROT_FUNCENTRY(setting, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetSetting(); }, true); })
  /*luadoc
  @function lvgl.page(params)

  Create a page layout for a One-Time script using EdgeTX styling: a header with title/sub-title lines and a back button in the top-left corner. Should be created as the top-level LVGL object, with all other objects added as its children.

  @param params (table):
   * `title` (string or function) title text in the header; function support added in 2.11.4, defaults to an empty string
   * `subtitle` (string or function) sub-title text in the header; function support added in 2.11.4, defaults to an empty string
   * `icon` (string) full path to a 30x30 grey-scale mask image for the back button icon (white = transparent, black = opaque); defaults to the EdgeTX logo icon
   * `back` (function) called when the top-left header button is tapped or RTN is pressed; if `backButton` is not set this is the only header button, defaults to nil
   * `menu` (function) called when the top-left header button is tapped, but only if `backButton` is set to true -- in that case `back` moves to a second button on the top-right, defaults to nil
   * `prevButton` (table) adds a prev navigation button; table has a `press` function and optional `active` function (added in 2.11.4), defaults to nil
   * `nextButton` (table) adds a next navigation button; table has a `press` function and optional `active` function (added in 2.11.4), defaults to nil
   * `flexFlow` (lvgl.FLOW_COLUMN or lvgl.FLOW_ROW) enables flex layout for this page, not used by default
   * `flexPad` (number) padding between rows/columns when flex layout is used, defaults to `PAD_OUTLINE`
   * `scrollBar` (boolean) enables/disables scroll bars, defaults to true
   * `scrollDir` (lvgl.SCROLL_xx) allowed scroll directions when child objects extend past the page bounds (stand alone scripts only), defaults to `lvgl.SCROLL_ALL`
   * `scrolled` (function) called when the page content is scrolled; passed `x`, `y` of the current scroll position, defaults to nil
   * `scrollTo` (function) overrides the page scroll position; must return `x`, `y` to scroll to, defaults to nil
   * `align` (alignment type (LEFT, RIGHT, CENTER, VTOP, VBOTTOM, VCENTER)) alignment used with flex layouts (added in 2.11.4), defaults to `CENTER | VTOP`
   * `backButton` (boolean) if true, adds a second header button on the top-right calling `back`, and repurposes the top-left button to call `menu` instead (added in 2.11.4), defaults to false
   * `borderPad` (number or table) border padding around the container edges; a single number applies to all sides, or a table `{left=?, right=?, top=?, bottom=?}` (added in 2.11.5), defaults to `PAD_OUTLINE` if `flexFlow` is set, otherwise 0

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @notice The common object settings (x, y, w, h, color, etc.) are not used by `lvgl.page` -- only the settings listed above apply.
  @notice The page automatically adds scroll bars if child objects fall outside its boundaries.
  @notice `scrollBar`, `scrollDir`, `scrolled`, and `scrollTo` were added in 2.11.2.
  @notice The PAGE keys are not automatically mapped to `prevButton`/`nextButton` -- the script's `run` function must detect and handle them itself.

  @status current Introduced in 2.11.0, title/subtitle function support, menu, prevButton, nextButton, align, and backButton added in 2.11.4, borderPad added in 2.11.5, scrollBar/scrollDir/scrolled/scrollTo added in 2.11.2
  */
  LROT_FUNCENTRY(page, [](lua_State* L) { return luaLvglObj(L, []() { return new LvglWidgetPage(); }, true); })
  /*luadoc
  @function lvgl.dialog(params)

  Display a dialog box.

  @param params (table):
   * `title` (string) text displayed in the dialog's header, defaults to an empty string
   * `close` (function) called when the dialog is closed, defaults to nil
   * `flexFlow` (lvgl.FLOW_COLUMN or lvgl.FLOW_ROW) enables flex layout for this box, not used by default
   * `flexPad` (number) padding between rows/columns when flex layout is used, defaults to `PAD_OUTLINE`

  @retval table LVGL object, or nil if not running as a One-Time script or fullscreen widget

  @notice Only available for One-Time scripts and widgets running in full screen mode -- returns nil otherwise.
  @notice The common object settings (x, y, w, h, color, etc.) are not used by `lvgl.dialog` -- only the settings listed above apply.

  @status current Introduced in 2.11.0
  */
  LROT_FUNCENTRY(dialog, [](lua_State* L) { return luaLvglObj(L, []() { return new LvglWidgetDialog(); }, true); })
  // Dialogs
  /*luadoc
  @function lvgl.confirm(params)

  Display a Yes/No confirmation dialog box.

  @param params (table):
   * `title` (string) text displayed in the dialog's header, defaults to an empty string
   * `message` (string) text displayed in the dialog's body, defaults to an empty string
   * `confirm` (function) called when the user taps Yes, defaults to nil
   * `cancel` (function) called when the user taps No, defaults to nil

  @retval none

  @notice The common object settings (x, y, w, h, color, etc.) are not used by `lvgl.confirm` -- only the settings listed above apply.

  @status current Introduced in 2.11.0
  */
  LROT_FUNCENTRY(confirm, [](lua_State* L) { return luaLvglPopup(L, []() { return new LvglWidgetConfirmDialog(); }); })
  /*luadoc
  @function lvgl.message(params)

  Display a message dialog box.

  @param params (table):
   * `title` (string) text displayed in the dialog's header, defaults to an empty string
   * `message` (string) text displayed in the dialog's body, defaults to an empty string
   * `details` (string) additional text displayed in the dialog's body, defaults to an empty string

  @retval none

  @notice The common object settings (x, y, w, h, color, etc.) are not used by `lvgl.message` -- only the settings listed above apply.

  @status current Introduced in 2.11.0
  */
  LROT_FUNCENTRY(message, [](lua_State* L) { return luaLvglPopup(L, []() { return new LvglWidgetMessageDialog(); }); })
  /*luadoc
  @function lvgl.menu(params)

  Open a popup menu with multiple options to choose from, using EdgeTX styling.

  @param params (table):
   * `title` (string) text displayed in the popup's header, defaults to an empty string
   * `values` (table) simple table of option strings shown in the popup, defaults to an empty list
   * `get` (function) called when the popup is opened, to get the index of the currently selected option (1..number of values), defaults to nil
   * `set` (function) called when the user taps a menu item; passed the selected item's index (1..number of values), defaults to nil

  @retval none

  @notice The common object settings (x, y, w, h, color, etc.) are not used by `lvgl.menu` -- only the settings listed above apply.
  @notice The popup closes and `set` is called when the user selects an item.
  @notice If the user taps outside the popup or presses RTN, the popup closes without calling `set`.

  @status current Introduced in 2.11.1
  */
  LROT_FUNCENTRY(menu, [](lua_State* L) { return luaLvglPopup(L, []() { return new LvglWidgetMenu(); }); })
  // Object manipulation functions
  LROT_FUNCENTRY(set, luaLvglSet)
  LROT_FUNCENTRY(show, luaLvglShow)
  LROT_FUNCENTRY(hide, luaLvglHide)
  LROT_FUNCENTRY(enable, luaLvglEnable)
  LROT_FUNCENTRY(disable, luaLvglDisable)
  LROT_FUNCENTRY(close, luaLvglClose)
  LROT_FUNCENTRY(getScrollPos, luaLvglGetScrollPos)
  LROT_NUMENTRY(FLOW_ROW, LV_FLEX_FLOW_ROW)
  LROT_NUMENTRY(FLOW_COLUMN, LV_FLEX_FLOW_COLUMN)
  LROT_NUMENTRY(PAD_TINY, PAD_TINY)
  LROT_NUMENTRY(PAD_SMALL, PAD_SMALL)
  LROT_NUMENTRY(PAD_MEDIUM, PAD_MEDIUM)
  LROT_NUMENTRY(PAD_LARGE, PAD_LARGE)
  LROT_NUMENTRY(PAD_OUTLINE, PAD_OUTLINE)
  LROT_NUMENTRY(PAD_BORDER, PAD_BORDER)
  LROT_NUMENTRY(SRC_ALL, 0xFFFFFFFF)
  LROT_NUMENTRY(SRC_INPUT, SRC_INPUT)
  LROT_NUMENTRY(SRC_LUA, SRC_LUA)
  LROT_NUMENTRY(SRC_STICK, SRC_STICK|SRC_TILT|SRC_LIGHT|SRC_SPACEMOUSE)
  LROT_NUMENTRY(SRC_POT, SRC_POT)
  LROT_NUMENTRY(SRC_OTHER, SRC_MINMAX|SRC_TX|SRC_TIMER)
  LROT_NUMENTRY(SRC_HELI, SRC_HELI)
  LROT_NUMENTRY(SRC_TRIM, SRC_TRIM)
  LROT_NUMENTRY(SRC_SWITCH, SRC_SWITCH|SRC_FUNC_SWITCH)
  LROT_NUMENTRY(SRC_LOGICAL_SWITCH, SRC_LOGICAL_SWITCH)
  LROT_NUMENTRY(SRC_TRAINER, SRC_TRAINER)
  LROT_NUMENTRY(SRC_CHANNEL, SRC_CHANNEL)
  LROT_NUMENTRY(SRC_GVAR, SRC_GVAR)
  LROT_NUMENTRY(SRC_TELEM, SRC_TELEM)
  LROT_NUMENTRY(SRC_CLEAR, SRC_NONE)
  LROT_NUMENTRY(SRC_INVERT, SRC_INVERT)
  LROT_NUMENTRY(SW_ALL, 0xFFFFFFFF)
  LROT_NUMENTRY(SW_SWITCH, SW_SWITCH)
  LROT_NUMENTRY(SW_TRIM, SW_TRIM)
  LROT_NUMENTRY(SW_LOGICAL_SWITCH, SW_LOGICAL_SWITCH)
  LROT_NUMENTRY(SW_TELEM, SW_TELEM)
  LROT_NUMENTRY(SW_OTHER, SW_OTHER)
  LROT_NUMENTRY(SW_CLEAR, SW_NONE)
  LROT_NUMENTRY(SCROLL_OFF, LV_DIR_NONE)
  LROT_NUMENTRY(SCROLL_HOR, LV_DIR_HOR)
  LROT_NUMENTRY(SCROLL_VER, LV_DIR_VER)
  LROT_NUMENTRY(SCROLL_ALL, LV_DIR_ALL)
  LROT_NUMENTRY(PERCENT_SIZE, LV_PCT(0))
  LROT_NUMENTRY(PAGE_BODY_HEIGHT, LCD_H - EdgeTxStyles::MENU_HEADER_HEIGHT)
  LROT_NUMENTRY(UI_ELEMENT_HEIGHT, EdgeTxStyles::UI_ELEMENT_HEIGHT)
  LROT_FLOATENTRY(LCD_SCALE, LUA_LCD_SCALE)
  LROT_NUMENTRY(LABEL, ETX_LABEL)
  LROT_NUMENTRY(RECTANGLE, ETX_RECTANGLE)
  LROT_NUMENTRY(CIRCLE, ETX_CIRCLE)
  LROT_NUMENTRY(ARC, ETX_ARC)
  LROT_NUMENTRY(HLINE, ETX_HLINE)
  LROT_NUMENTRY(VLINE, ETX_VLINE)
  LROT_NUMENTRY(LINE, ETX_LINE)
  LROT_NUMENTRY(TRIANGLE, ETX_TRIANGLE)
  LROT_NUMENTRY(IMAGE, ETX_IMAGE)
  LROT_NUMENTRY(QRCODE, ETX_QRCODE)
  LROT_NUMENTRY(BOX, ETX_BOX)
  LROT_NUMENTRY(BUTTON, ETX_BUTTON)
  LROT_NUMENTRY(MOMENTARY_BUTTON, ETX_MOMENTARY_BUTTON)
  LROT_NUMENTRY(TOGGLE, ETX_TOGGLE)
  LROT_NUMENTRY(TEXT_EDIT, ETX_TEXTEDIT)
  LROT_NUMENTRY(NUMBER_EDIT, ETX_NUMBEREDIT)
  LROT_NUMENTRY(CHOICE, ETX_CHOICE)
  LROT_NUMENTRY(SLIDER, ETX_SLIDER)
  LROT_NUMENTRY(VERTICAL_SLIDER, ETX_VERTICAL_SLIDER)
  LROT_NUMENTRY(PAGE, ETX_PAGE)
  LROT_NUMENTRY(FONT, ETX_FONT)
  LROT_NUMENTRY(ALIGN, ETX_ALIGN)
  LROT_NUMENTRY(COLOR, ETX_COLOR)
  LROT_NUMENTRY(TIMER, ETX_TIMER)
  LROT_NUMENTRY(SWITCH, ETX_SWITCH)
  LROT_NUMENTRY(SOURCE, ETX_SOURCE)
  LROT_NUMENTRY(FILE, ETX_FILE)
  LROT_NUMENTRY(SETTING, ETX_SETTING)
LROT_END(lvgllib, NULL, 0)

// Metatable for simple objects (line, arc, label)
LROT_BEGIN(lvgl_base_mt, NULL, LROT_MASK_GC_INDEX)
  LROT_FUNCENTRY(__gc, luaDestroyLvglWidget)
  LROT_TABENTRY(__index, lvgl_base_mt)
  // Object manipulation functions
  LROT_FUNCENTRY(set, luaLvglSet)
  LROT_FUNCENTRY(show, luaLvglShow)
  LROT_FUNCENTRY(hide, luaLvglHide)
LROT_END(lvgl_base_mt, NULL, LROT_MASK_GC_INDEX)

// Metatable for complex objects
LROT_BEGIN(lvgl_mt, NULL, LROT_MASK_GC_INDEX)
  LROT_FUNCENTRY(__gc, luaDestroyLvglWidget)
  LROT_TABENTRY(__index, lvgl_mt)
  LROT_FUNCENTRY(clear, luaLvglClear)
  LROT_FUNCENTRY(build, luaLvglBuild)
  // Objects - widgets and standalone scripts
  LROT_FUNCENTRY(label, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetLabel(); }); })
  LROT_FUNCENTRY(rectangle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetRectangle(); }); })
  LROT_FUNCENTRY(hline, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetHLine(); }); })
  LROT_FUNCENTRY(vline, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetVLine(); }); })
  LROT_FUNCENTRY(line, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetLine(); }); })
  LROT_FUNCENTRY(triangle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTriangle(); }); })
  LROT_FUNCENTRY(circle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetCircle(); }); })
  LROT_FUNCENTRY(arc, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetArc(); }); })
  LROT_FUNCENTRY(image, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetImage(); }); })
  LROT_FUNCENTRY(qrcode, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetQRCode(); }); })
  // Objects - standalone scripts and full screen widgets only
  LROT_FUNCENTRY(button, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTextButton(); }, true); })
  LROT_FUNCENTRY(momentaryButton, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetMomentaryButton(); }, true); })
  LROT_FUNCENTRY(toggle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetToggleSwitch(); }, true); })
  LROT_FUNCENTRY(textEdit, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTextEdit(); }, true); })
  LROT_FUNCENTRY(numberEdit, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetNumberEdit(); }, true); })
  LROT_FUNCENTRY(choice, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetChoice(); }, true); })
  LROT_FUNCENTRY(slider, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetSlider(); }, true); })
  LROT_FUNCENTRY(verticalSlider, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetVerticalSlider(); }, true); })
  LROT_FUNCENTRY(font, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetFontPicker(); }, true); })
  LROT_FUNCENTRY(align, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetAlignPicker(); }, true); })
  LROT_FUNCENTRY(color, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetColorPicker(); }, true); })
  LROT_FUNCENTRY(timer, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTimerPicker(); }, true); })
  LROT_FUNCENTRY(switch, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetSwitchPicker(); }, true); })
  LROT_FUNCENTRY(source, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetSourcePicker(); }, true); })
  LROT_FUNCENTRY(file, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetFilePicker(); }, true); })
  // Containers
  LROT_FUNCENTRY(box, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetBox(); }); })
  LROT_FUNCENTRY(setting, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetSetting(); }, true); })
  // Object manipulation functions
  LROT_FUNCENTRY(set, luaLvglSet)
  LROT_FUNCENTRY(show, luaLvglShow)
  LROT_FUNCENTRY(hide, luaLvglHide)
  LROT_FUNCENTRY(enable, luaLvglEnable)
  LROT_FUNCENTRY(disable, luaLvglDisable)
  LROT_FUNCENTRY(close, luaLvglClose)
  LROT_FUNCENTRY(getScrollPos, luaLvglGetScrollPos)
LROT_END(lvgl_mt, NULL, LROT_MASK_GC_INDEX)

LUALIB_API int luaopen_lvgl(lua_State *L)
{
  luaL_rometatable(L, LVGL_SIMPLEMETATABLE, LROT_TABLEREF(lvgl_base_mt));
  luaL_rometatable(L, LVGL_METATABLE, LROT_TABLEREF(lvgl_mt));
  return 0;
}
}
