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

#include "list_support.h"

#ifndef __STRING_SUPPORT_H__
#define __STRING_SUPPORT_H__

char *append_dup_string( char *s, char *suffix );

char *slice( char *s, int relative_begin, int relative_end );

List *split( char *text, char *delimiters );

List *split_after_first_word( char *text );

char *trim( char *s );

char *get_string_in_brackets( char *string );

char *get_string_in_quotes( char *string );


char *remove_quotes( char *s );

char *remove_backslash( char *s, char character );


#endif /* __STRING_SUPPORT_H__ */
