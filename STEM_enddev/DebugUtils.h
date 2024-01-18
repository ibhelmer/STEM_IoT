/*
DebugUtils.h - Simple debugging utilities.
Copyright (C) 2011 Fabio Varesano <fabio at varesano dot net>

Ideas taken from:
http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1271517197

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

For use in code insert:
#define DEBUG
#include "DebugUtils.h"
*/

#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#ifdef DEBUG
/* Long version */
/* #define DEBUG_PRINT(str)    \
    Serial.print(millis());     \
    Serial.print(": ");    \
    Serial.print(__PRETTY_FUNCTION__); \
    Serial.print(' ');      \
    Serial.print(__FILE__);     \
    Serial.print(':');      \
    Serial.print(__LINE__);     \
    Serial.print(' ');      \
    Serial.println(str);
#else */
/* Short version */
/*#define DBPRINT(str)    \
    Serial.print(__PRETTY_FUNCTION__); \
    Serial.print(" -> ");      \
    Serial.print(str);
#define DBPRINTLN(str)   \
    Serial.print(__PRETTY_FUNCTION__); \
    Serial.print(" -> ");      \
    Serial.println(str);
*/

/* Minimum version */
#define DBPRINT(str)    \
    Serial.print(str);
#define DBPRINTLN(str)   \
    Serial.println(str);
#else
  #define DBPRINT(str)
  #define DBPRINGLN(str)
#endif

#endif