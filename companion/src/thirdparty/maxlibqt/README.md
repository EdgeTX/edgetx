# maxLibQt - C++ and QML code library for the *Qt*&trade; framework.

This is a (growing) collection of somewhat random C++ classes and QtQuick QML modules for use with the [Qt](http://qt.io)&trade; framework. 

They are free to use in other open source projects under the terms of the GNU Public License (GPL).  For use in
commercial or other closed-source software, you need to contact me for a license agreement.  See the LICENSE.txt file.

The original target Qt library version was 5.2+. Newer components, and some that have been updated more recently,
may require a later version, at least 5.9 (if you run into something that could be made more backwards-compatible, 
let me know). Most (if not all) of the C++ code requires C++11 at minimum.

Project home: https://github.com/mpaperno/maxLibQt

## C++ Components:

* Core
  * `AppDebugMessageHandler` - Custom debug/message handler class to work in conjunction with *qDebug()* family of functions.
* Item Models
  * `GroupedItemsProxyModel` - A proxy model (`QIdentityProxyModel` subclass) which allows a grouped tree-based item presentation of a flat table data model. Typically used for visually grouping items by some shared criteria, like a category or subject. Useful in a `QTreeView` or the `TreeComboBox` from this collection.
* Layouts
  * `OverlayStackLayout` - A QStackedLayout with additional features to allow stacks with "floating" overlay widgets, such as toolbars, buttons, 
  messages, etc., while still allowing interaction with exposed areas of the widget(s) underneath. Includes a functional image viewer example application.
* Widgets
  * `ActionPushButton` - A QPushButton which takes a default QAction, just like a QToolButton can.
  * `BuddyLabel` - A QLabel with enhanced "buddy" capabilities like click redirect and tooltip inheritance.
  * `CollapsingToolBar` - A QToolBar which toggles between _ToolButtonIconOnly_ and _ToolButtonTextBesideIcon_ styles based on available width.
  * `ExportableTableView` - A QTableView with features to export data as plain text or HTML.
  * `RoundedMessageBox` - A frameless QMessageBox implementation, fully stylable via CSS or QPalette.
  * `ScrollableMessageBox` - A simple message box with a large scrollable area (a QTextEdit) for detailed text (including HTML formatting).
  * `TimerEdit` - A time value line editor which accepts negative and large times (> 23:59:59), suitable for a timer, etc.
  * `TreeComboBox` - A QComboBox control which works with a tree-based data model & view, allowing drill-down selection of items.

## QML Components:

* Controls
  * [`MLDoubleSpinBox`](maxLibQt::controls::MLDoubleSpinBox) - A SpinBox control which handles float/double number types to the desired precision, as well as large integers (within ECMAScript limits). Avoids the `int` size type limitation of the current QtQuick Controls (v2.0 - 2.4) SpinBox but still using the current theme styling (Fusion/Material/etc).
  * [`MLHexSpinBox`](maxLibQt::controls::MLHexSpinBox) - A SpinBox control which allows editing integers in hexadeciaml format. Allows a wide range of numbers, including unsigned integers. Based on `MLDoubleSpinBox`.

### Documentation:

Some Doxygen-style documentation is embedded in the code and can be generated as either part of the CMake build or manually with the
included Doxyfile (run `doxygen doc/Doxyfile` in root of this project). To generate docs for *QML code* you'd need to use install [doxyqml](https://github.com/agateau/doxyqml) (the Doxyfile is already configured to use it if you have it). Some of the source code is further documented inline (but never enough).

This documentation is also published at https://mpaperno.github.io/maxLibQt/

### Building (C++):

Both *CMake* and *qmake* project files are provided. They're set up to be built as individual libraries split by base folder (core, item models, etc). With CMake you can also build one library file which contains all the others (haven't figured out how to do that with qmake yet).

You can also simply include whichever C++ module(s) you need into your own project. They're standalone unless specifically mentioned otherwise in their docs.  There are also `.pri` *qmake* files for each source folder which could be used to quickly include all modules in that folder (and add to the `INCLUDEPATH`).

### Using (QML)

Simplest way to use the QtQuick modules would be to copy them into your code tree. 

To use them as external ("library") imports in your code, put the `maxLibQt` folder (from inside the `/src/quick/` folder of this project) into the QML search path for your app. Then, for example to use `MLDoubleSpinBox`, you'd specify `import maxLibQt.controls 1.0` in your QML.

-------------
### Author

Maxim Paperno   
https://github.com/mpaperno/   
http://www.WorldDesign.com/contact  

Please inquire about custom C++/Qt development work.

### Copyright, License, and Disclaimer

Copyright (c) Maxim Paperno. All rights reserved.

See LICENSE.txt file for license details.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 

*Qt* is a trademark of *The Qt Company* with which neither I nor this project have any affiliation.
