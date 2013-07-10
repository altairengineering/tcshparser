/*  This file is part of tcshParser.

    Copyright (C) 2013 Ellexus (www.ellexus.com)

    tcshParser is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __ALIAS_SUPPORT_H__
#define __ALIAS_SUPPORT_H__

typedef struct alias {
    struct alias *next;
    char *lhs;
    char *rhs;
} Alias;

Alias *new_alias( Alias *aliases, char *lhs, char *rhs );

char *lookup_alias( char *cmd, Alias *a );

void print_aliases( Alias *a );


#endif /* __ALIAS_SUPPORT_H__ */

