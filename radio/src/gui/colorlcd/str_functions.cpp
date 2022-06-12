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
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <list>
#include "str_functions.h"

static const char splitChars[] = { " -\t" };

void removeAllWhiteSpace(char *str)
{
  int len = strlen(str);
  int origLen = len;
  while(len) {
    if (iswspace(str[len]))
      memmove(str + len, str + len + 1, origLen - len);
    len--;
  }
}

char *trim(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}

std::string trim_start(std::string str)
{
  uint32_t i;
  for (i = 0; i < str.length(); i++)
    if (!iswspace(str[i]))
      break;
  
  return str.substr(i);
}

std::vector<std::string> Explode(std::string str, const char *splitChars)
{
    std::vector<std::string> parts;
    int startIndex = 0;
    while (true)
    {
        size_t index = str.find_first_of(splitChars, startIndex);

        if (index == std::string::npos)
        {
            parts.emplace_back(str.substr(startIndex));
            return parts;
        }

        std::string word = str.substr(startIndex, index - startIndex);
        char nextChar = str.substr(index, 1)[0];

        // Dashes and the likes should stick to the word occuring before it. Whitespace doesn't have to.
        if (iswspace(nextChar))
        {
            parts.emplace_back(word);
            parts.emplace_back(std::string(&nextChar, 1));
        }
        else
        {
            parts.emplace_back(word + nextChar);
        }

        startIndex = index + 1;
    }
}

std::string wrap(std::string str, uint32_t width)
{
    std::vector<std::string> words = Explode(str, splitChars);

    uint32_t curLineLength = 0;
    std::ostringstream strBuilder;

    for(uint32_t i = 0; i < words.size(); i++)
    {
        std::string word = words[i];
        // If adding the new word to the current line would be too long,
        // then put it on a new line (and split it up if it's too long).
        if (curLineLength + word.length() > width)
        {
            // Only move down to a new line if we have text on the current line.
            // Avoids situation where wrapped whitespace causes emptylines in text.
            if (curLineLength > 0)
            {
                strBuilder << "\n";
                curLineLength = 0;
            }

            // If the current word is too long to fit on a line even on it's own then
            // split the word up.
            while (word.length() > width)
            {
                strBuilder << word.substr(0, width - 1) + "-"; 
                word = word.substr(width - 1);

                strBuilder << "\n";
            }

            // Remove leading whitespace from the word so the new line starts flush to the left.
            word = trim_start(word);
        }

        strBuilder << word;
        curLineLength += word.length();
    }

    return strBuilder.str();
}
