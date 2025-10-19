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
 *
 * Based on
 *
  Elypson/qt-collapsible-section
  (c) 2016 Michael A. Voelkel - michael.alexander.voelkel@gmail.com

  This file is part of Elypson/qt-collapsible section.

  Elypson/qt-collapsible-section is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Elypson/qt-collapsible-section is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Elypson/qt-collapsible-section. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QFrame>
#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QToolButton>
#include <QWidget>

class AutoCollapsibleSection : public QWidget
{
  Q_OBJECT

  public:
    static const int DEFAULT_DURATION = 0;

    explicit AutoCollapsibleSection(QWidget * parent = nullptr);

    // setters
    void setAnimationDuration(const int duration);
    void setContentLayout(QLayout & contentLayout);
    void setTitle(const QString & title);

    // getters
    const int getAnimationDuration() { return animationDuration; }

    void updateHeights();

  signals:
    // inform parent that expand/collapse animation finished
    void resized();

  public slots:
    void toggle(bool collapsed);

  private:
    QGridLayout *mainLayout;
    QToolButton *toggleButton;
    QFrame *headerLine;
    QParallelAnimationGroup *toggleAnimation;
    QScrollArea *contentArea;
    int animationDuration = DEFAULT_DURATION;
    int collapsedHeight;
    bool isExpanded = false;
    QString title;
};
