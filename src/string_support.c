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

#include "string_support.h"
#include "list_support.h"

/* Append a string to a string which is stored in memory allocated by
   malloc. (Often a string returned by strdup or strndup.) */

char *append_dup_string( char *s, char *suffix )
{
    s = realloc( s, (strlen(s) + strlen(suffix) + 1) );
    strcat( s, suffix );

    return s;
}

/* Create a copy of a part of a string. The beginning and end of the
   substring can be specified relative to the start or end of the
   string. 
   
   If relative_begin is zero then the substring begins at the start of
   the string. If relative_end is zero then the substring ends at the
   end of the string.

   If either relative_begin or relative_end is positive then it
   defines the position relative to the start of the string, and if
   negative relative to the end of the string. So a value of +1 means
   one character from the start of the string while -1 means one
   character from the end of the string.

   Any implied position which is before the start of the string is
   replaced by the start of the string. Similary any implied position
   which is beyond the end of the string is replaced by the end of the
   string. */

char *slice( char *s, int relative_begin, int relative_end )
{
    int length = strlen( s );
    int absolute_begin;
    int absolute_end;
    char *result;

    if ( relative_begin >= 0 ) {
        absolute_begin = relative_begin;
    } else {
        absolute_begin = length + relative_begin;
    }

    if ( absolute_begin < 0 ) {
        absolute_begin = 0;
    } else if ( absolute_begin > length ) {
        absolute_begin = length;
    }

    if ( relative_end > 0 ) {
        absolute_end = relative_end;
    } else {
        absolute_end = length + relative_end;
    }

    if ( absolute_end < 0 ) {
        absolute_end = 0;
    } else if ( absolute_end > length ) {
        absolute_end = length;
    }

    if ( absolute_begin < absolute_end ) {
        result = strndup( &(s[absolute_begin]), absolute_end - absolute_begin );
    } else {
        result = strdup( "" );
    }

    return result;
}

/* Returns a two element list, the first element containing the first
   word and the second element containing any other words. Words are
   delimited by white space. */

List *split_after_first_word( char *text )
{
    int i = 0;
    List *result = NULL;

    while ( (text[i] != '\0') &&  !isspace( text[i] ) ) {
        i++;
    }

    result = append_to_list( result, slice( text, 0, i ) );

    while ( (text[i] != '\0') && isspace( text[i] ) ) {
        i++;
    }

    result = append_to_list( result, slice( text, i, 0 ) );

    return result;
}

/* Split a string into a list of "words".

Words are delimited by any character found within the delimiters
string, except that words will never be split within double quotes.

The delimiters parameter contains all characters we might want to
count as white-space. */


List *split( char *text, char *delimiters )
{
    int length = strlen(text);
    int in_quote = 0;
    int i;
    List *result = NULL;
    char *buffer;
    char *p;

    /* Allocate a buffer which is long enough to hold a single
       word. The word can't be longer than longer than the string that
       we are splitting. */

    buffer = malloc( length + 1 );
    p = buffer;

    for ( i = 0; i < length; i++ ) {
        if ( text[i] == '"' ) {
            /* If we see a ", then toggle our mode between inside 
               and outside of quotes. */
            in_quote = !in_quote;
        } else if ( in_quote || (strchr(delimiters, text[i]) == NULL) ) {
            /* We're either inside a quote, or have verified that
               we're not looking at a delimiter, so copy the
               character into the buffer for the next word. */
            *p++ = text[i];
        } else if ( p != buffer ) {
            /* We're not in a quote, and we're looking at a delimeter,
               so if there is a word stored in the buffer, append it
               to the list. */
            *p = '\0';
            result = append_to_list( result, strdup( buffer ) );
            p = buffer;
        }
    }

    /* When we reach the end of the string, append any word that was
       in the buffer to the list. */
    if ( p != buffer ) {
        *p = '\0';
        result = append_to_list( result, strdup( buffer ) );
    }

    free( buffer );
    return result;
}

/* Given a string in memory allocated by malloc, returns a new string,
   also in memory allocated by malloc, with any leading or trailing
   white space removed. 

   The original string is freed, and a new string allocated. */

char *trim( char *s )
{
    int begin = 0;
    int end = strlen(s) - 1;
    char *result;

    while ( (begin < end) && isspace(s[begin]) ) {
        begin++;
    }

    while ( (end > begin) && isspace(s[end]) ) {
        end--;
    }

    result = slice( s, begin, (end+1) );

    free( s );
    return ( result );
}

/* If a string begins with the specified begin and end characters,
   return a new string corresponding to the string between these
   characters. If the string doesn't start and end with these
   characters, return the unmodified string. */

static char *get_enclosed_string( char *s, char left, char right )
{
    size_t length;
    char *result;

    length = strlen( s );

    if ( (s[0] == left) && (s[length-1] == right) ) {
        result = slice( s, 1, -1 );
        free( s );
    } else {
        result = s;
    }

    return result;
}

/* If the string starts with '(' and ends with ')', then create a new
   string which contains the characters between the '(' and the
   ')'. In this case the original string is freed. If however the
   string isn't enclosed in "()" then return the original string. */

char *get_string_in_brackets( char *string )
{
    return get_enclosed_string( string, '(', ')' );
}

/* If the string starts with '"' and ends with '"', then create a new
   string which contains the characters between the quotes. In this
   case the original string is freed. If however the string isn't
   enclosed in quotes then return the original string. */

char *get_string_in_quotes( char *string )
{
    return get_enclosed_string( string, '"', '"' );
}

/* Remove quotes (") from a string, unless they are escaped with a
   backslash. */

char *remove_quotes( char *s )
{
    size_t length = strlen(s);
    char *result = malloc( length + 1 );
    int i = 0;
    int j = 0;

    while ( i < length ) {
        if ( s[i] == '\\' ) {
            result[j++] = s[i++];
            result[j++] = s[i++];
        } else if ( s[i] == '"' ) {
            i++;
        } else {
            result[j++] = s[i++];
        }
    }

    result[j] = '\0';

    free( s );
    return result;
}

/* Return a new copy of a string with any \<character> converted to
   <character>. The original string is freed. */

char *remove_backslash( char *s, char character )
{
    size_t length = strlen(s);
    char *result = malloc( length + 1 );
    int i = 0;
    int j = 0;

    while ( i < length ) {
        if ( s[i] == character ) {
            if ( (i > 0) && (j > 0) && (s[i-1] == '\\') ) {
                result[j-1] = s[i++];
            } else {
                result[j++] = s[i++];
            }
        } else {
            result[j++] = s[i++];
        }
    }

    result[j] = '\0';

    free( s );
    return result;
}
