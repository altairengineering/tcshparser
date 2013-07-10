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

#ifndef __LIST_SUPPORT_H__
#define __LIST_SUPPORT_H__

/* A lot of the code involves working with lists of strings. The
   following few functions provide support a simple linked list
   representation of such lists. */

typedef struct list {
    struct list *next;
    char *contents;
} List;

int list_length( List *list );

char *get_nth_word( List *list, int n );

List *append_to_list( List *list, char *value );

void free_list( List *list );

void print_list( List *list );

#endif /* __LIST_SUPPORT_H__ */
