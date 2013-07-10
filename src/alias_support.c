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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "alias_support.h"

Alias *new_alias( Alias *aliases, char *lhs, char *rhs )
{
    Alias *result;

    result = malloc( sizeof(Alias) );
    if ( result != NULL ) {
        result->lhs = lhs;
        result->rhs = rhs;
        result->next = aliases;
    }

    return result;
}


/* Print the expansion of all known aliases. */

void print_aliases( Alias *a )
{
    while ( a != NULL ) {
        fprintf( stderr, "Alias: (%s) expands to: (%s)\n", a->lhs, a->rhs );
        a = a->next;
    }
}

/* If there is an alias which matches the command, return its expansion, otherwise NULL. */

char *lookup_alias( char *cmd, Alias *a )
{
    while ( (a != NULL) && (strcmp( cmd, a->lhs ) != 0) ) {
        a = a->next;
    }

    if ( a != NULL ) {
        return strdup( a->rhs );
    } else {
        return NULL;
    }
}
