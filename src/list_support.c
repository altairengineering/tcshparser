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

/* Functions to represent a list of strings. */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "list_support.h"

/* Returns the number of items in the list. */

int list_length( List *list )
{
    int result = 0;

    while ( list != NULL ) {
        list = list->next;
        result++;
    }

    return result;
}


char *get_nth_word( List *list, int n )
{
    char *word;

    while ( (n > 0) && (list != NULL) ) {
        n--;
        list = list->next;
    }

    if ( list != NULL ) {
        word = list->contents;
    } else {
        word = "";
    }

    return strdup( word );
}

/* Create an entry for "value" and append it to an existing "list". */

List *append_to_list( List *list, char *value )
{
    List *entry;
    List *result;

    entry = malloc( sizeof( List ) );
    if ( entry != NULL ) {
        entry->next = NULL;
        entry->contents = value;

        if ( list == NULL ) {
            result = entry;
        } else {
            result = list;
            while ( list->next != NULL ) {
                list = list->next;
            }

            list->next = entry;
        }
    }

    return result;
}

/* Free all the memory allocated to a list of strings */

void free_list( List *list )
{
    List *entry;

    while ( list != NULL ) {
        if ( list->contents != NULL ) {
            free( list->contents );
        }

        entry = list;
        list = list->next;

        free( entry );
    }
}

/* Print a list of strings. Only used for debugging. */

void print_list( List *list )
{
    int i;

    for ( i = 0; list != NULL; i++ ) {
        fprintf( stderr, "Entry %d: %s\n", i, list->contents );
        list = list->next;
    }
}

