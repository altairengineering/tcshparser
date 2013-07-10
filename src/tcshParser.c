/*  Take a tcsh alias table and a tcsh command, and print the command
    after alias substitution.

       tcshParser <alias-file> <cmd args ...>

    The alias file can be created from within tcsh by:

       alias > alias.txt

    Copyright (C) 2013 Ellexus (www.ellexus.com)

    This program is free software: you can redistribute it and/or modify
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
#include <error.h>
#include <errno.h>
#include <err.h>

#include "list_support.h"
#include "string_support.h"
#include "alias_support.h"

/* Any character in the white_space string will be taken to delimit
   words in an alias. */

char *white_space = " \f\n\r\t\v";

/* Read the contents of the alias file into memory. The alias file
   contains one line for each alias which has been defined. The first
   word on the line is the alias, and the rest of the line what the
   alias expands to. */

Alias *read_alias_table( char *alias_file )
{
    FILE *f;
    char *line = NULL;
    size_t buffer_size = 0;
    ssize_t n;
    Alias *result = NULL;
    List *words;
    char *lhs;
    char *rhs;

    f = fopen( alias_file, "r" );
    if ( f != NULL ) {
        while ( (n = getline( &line, &buffer_size, f )) > 0 ) {

            /* When reading a file using getline, we are given the '\n' characters, 
               but in this case we're not interested in them. */

            if ( (n > 0) && (line[n-1] == '\n') ) {
                line[n-1] = '\0';
            }

            /* Splitting the line between the first and second words, gives us the 
               alias and what it should expand to. */

            words = split_after_first_word( line );

            if ( list_length( words ) == 2 ) {
                lhs = get_nth_word( words, 0 );
                rhs = get_nth_word( words, 1 );

                /* What the alias expands to is often enclosed in
                   "()", but we don't need the brackets. Similary we
                   can strip out any unescaped quotes, i.e. '"'.*/

                rhs = get_string_in_brackets( rhs );
                rhs = remove_quotes( rhs );

                result = new_alias( result, lhs, rhs );
            } else {
                fprintf( stderr, "Ignoring unexpected entry in alias table: %s\n", line );
            }

            free_list( words );
        }

        free( line );
        fclose( f );
    } else {
        warn( "Unable to open file %s", alias_file );
    }

    return result;
}


/* A command is considered "empty" if it either of zero length or
   contains only white-space characters. */

int is_empty( char *command )
{
    while ( *command != '\0' && isspace( *command ) ) {
        command++;
    }

    if ( *command == '\0' ) {
        return 1;
    } else {
        return 0;
    }
}

/* Return a string representing words n through to m from the alias
   and its arguments. By analogy with conventional command line
   processing, argument zero is taken to mean the name of the alias,
   and arguments 1 to length are the actual arguments. */

char *arg_substring(int n, int m, char *alias, char *args)
{
    List *arg_array;
    int length;
    int first_arg;
    int i;
    char *result;

    /* Split the "args" character string into a list of words
       delimited by white space. */

    arg_array = split( args, white_space );
    length = list_length( arg_array );

    // fprintf( stderr, "arg_substring( %d, %d, %s, %s )\n", n, m, alias, args );

    if ( (n <= m) && (m <= length) ) {
        if ( n == 0 ) {
            result = strdup( alias );
            first_arg = 1;
        } else {
            result = strdup( "" );
            first_arg = n;
        }

        for ( i = first_arg; i <= m; i++ ) {
            result = append_dup_string( result, " " );
            result = append_dup_string( result, get_nth_word( arg_array, i-1 ) );
        }

        result = trim( result );
    } else {
        result = strdup( "" );
    }

    //fprintf( stderr, "arg_substring returns '%s'\n", result );

    return result;
}

/* Returns a copy of any sequence of digits from the start of a
   string. */

char *get_integer( char *string )
{
    int i = 0;
    int length = strlen( string );
    char *result;

    while ( (i < length) && isdigit( string[i] ) ) {
        i++;
    }

    if ( i > 0 ) {
        result = slice( string, 0, i );
    } else {
        result = NULL;
    }

    return result;
}

/* Before tcsh processes aliases, it stores the current command in the
   "history", so that an alias can use history processing commands on
   it. So, in the context of this program, "history" means the current
   command, rather than the commands that you typed before this
   one. */

char *replace_history( char *cmd, char *alias, char *args )
{
    int start_index = 0;
    int curr_index = 0;
    char *result = strdup( "" );
    List *arg_list;
    int num_args;
    int cmd_length = strlen( cmd );

    //fprintf( stderr, "replace_history( (%s),(%s),(%s) )\n", cmd, alias, args );

    char *m_string;
    char *n_string;

    //if we don't find a history pattern we just shove the args at the end.
    int found = 0;

    /* Split args into a list of words */

    arg_list = split( args, white_space );
    num_args = list_length( arg_list );

    /* We are looking for patterns which start with a '!'. The '!' is
       always followed by one or more characters, so there is no point
       in looking past the penultimate character. */

    while ( curr_index < (cmd_length - 1) ) {
        if ( cmd[curr_index] == '!' ) {
            int n = -1; //not set
            int m = -1; //not set
            curr_index++;

            int pattern_size = 0;
            int found_colon = 0;
            if ( cmd[curr_index] == ':' ) {
                curr_index++;
                found_colon = 1;
            }

            switch(cmd[curr_index]) {
            case '!':
            case '#':
                /* "!!" means the previous event while "!#" means the
                   current event, but for our purposes the two are
                   equivalent. */
                found = 1;
                n = 0;
                m = num_args;
                pattern_size = 1;
                break;

            case '*': /* "!*" means all the arguments, but returns nothing if there are no args. */
                found = 1;
                n = 1;
                m = num_args;
                pattern_size = 1;
                break;

            case '$': /* "!$" means the final argument */
                found = 1;
                n = num_args;
                m = num_args;
                pattern_size = 1;
                break;

            case '^': /* "!^" means the first argument */
                found = 1;
                n = 1;
                /* If the next character is '-', then we have a range 
                   starting with the first arg. */

                if (cmd[curr_index + 1] == '-') {
                    m_string = get_integer( &(cmd[curr_index+2]) );
                    if (m_string != NULL) { //"!:^-m
                        pattern_size = strlen( m_string ) + 2;
                        m = atoi( m_string );
                        free( m_string );
                    } else if ( cmd[curr_index+2] == '$' ) {
                        pattern_size = 3;
                        m = num_args;
                    } else { 
                        /* A "!:^-" without a following number implies all 
                           args but last, equivalent to "!:1-" */
                        pattern_size = 2;
                        m = num_args-1;
                    }
                } else {
                    pattern_size = 1;
                    m = 1;
                }
                break;

            case '-':
                /* If a "range" starts with a '-', then the range
                   implicitly starts at zero. */
                found = 1;
                n = 0;
                m_string = get_integer( &(cmd[curr_index+1]) );
                if (m_string != NULL) {
                    pattern_size = strlen(m_string) + 1;
                    m = atoi(m_string);
                    free( m_string );
                } else if (cmd[curr_index + 1] == '$') {
                    //"!:-$" implies all
                    pattern_size = 2;
                    m = num_args;
                } else {
                    //"!:-" implies all but last, equivalent to "!:0-"
                    pattern_size = 1;
                    m = num_args-1;
                }
                break;

            default:
                if ( found_colon ) {
                    found = 1;
                    n_string = get_integer( &(cmd[curr_index]) );
                    if (n_string != NULL) {
                        //found "!:n... "
                        pattern_size = strlen( n_string );
                        n = atoi( n_string );
                        free( n_string );

                        switch(cmd[curr_index + pattern_size]) {
                        case '*': /* Equivalent to !:n-$ */
                            m = num_args;
                            pattern_size++;
                            break;

                        case '-':
                            pattern_size++;
                            m_string = get_integer( &(cmd[curr_index+pattern_size]) );
                            if (m_string != NULL) {
                                //"!:n-m"
                                pattern_size += strlen( m_string );
                                m = atoi( m_string );
                                free( m_string );
                            } else if ( cmd[curr_index+pattern_size] == '$' ) {
                                m = num_args;
                                pattern_size++;
                            } else {
                                //"!:n-" implies all but last
                                m = num_args-1;
                            }
                            break;

                        default:
                            //just "!:n"
                            m = n;
                            break;
                        }
                    } else {
                        //"!:" is the same as "!#"
                        n = 0;
                        m = num_args;
                        pattern_size = 1;
                    }
                }
                break;
            }

            if ( pattern_size != 0 ) {
                int length;
                if ( found_colon ) {
                    length = curr_index - start_index - 2;
                } else {
                    length = curr_index - start_index - 1;
                }
                result = append_dup_string( result,
                                            strndup( &(cmd[start_index]), length ) );

                result = append_dup_string( result, arg_substring(n, m, alias, args) );
                curr_index += pattern_size;
                start_index = curr_index;
            }

        } else { // not '!'
            curr_index++;
        }
    }

    if ( start_index < cmd_length ) {
        result = append_dup_string( result, &(cmd[start_index]) );
    }

    /* If we didn't find any history substitutions in the whole
       string, then simply append the args. */
    if ( !found ) {
        result = append_dup_string( result, " " );
        result = append_dup_string( result, args );
    }

    result = trim( result );

    //fprintf( stderr, "replace_history returns (%s)\n", result );
    return result;
}

/* Break up a string into a list of simple commands */

List *split_into_simple_commands( char *cmd )
{
    List *command_list = NULL;

    int escaped = 0;
    int in_single_quote = 0;
    int in_double_quote = 0;
    int in_backwards_quote = 0;

    int start_index = 0;
    int current_index = 0;
    int cmd_length = strlen( cmd );
    char c;
    char prev_c;
    char next_c;

    //fprintf( stderr, "split_into_simple_commands(%s)\n", cmd );

    while ( current_index < cmd_length ) {
        /* It is sometimes easiest if we can look one character to the
           right or to the left of the current position, whilst not
           going off either end of the string. */

        c = cmd[ current_index ];

        if ( current_index > 0 ) {
            prev_c = cmd[ current_index - 1 ];
        } else {
            prev_c = '\0';
        }

        if ( (current_index + 1) < cmd_length ) {
            next_c = cmd[ current_index + 1 ];
        } else {
            next_c = '\0';
        }

        switch(c) {
        case '\\' :
            escaped = 1;
            current_index++;
            break;

        case '\'' :
            if ( escaped ) {
                escaped = 0;
                current_index++;
            } else {
                in_single_quote = !in_single_quote;
                current_index++;
            }
            break;

        case '\"' :
            if ( escaped ) {
                escaped = 0;
                current_index++;
            } else {
                in_double_quote = !in_double_quote;
                current_index++;
            }
            break;

        case '`' :
            if ( escaped ) {
                escaped = 0;
                current_index++;
            } else {
                in_backwards_quote = !in_backwards_quote;
                current_index++;
            }
            break;

        case '|' :
        case '&' :
        case '(' :
        case ')' :
        case ';' :
            escaped = 0;
            if (in_single_quote || in_double_quote || in_backwards_quote) {
                current_index++;
            } else {
                if ( ((c == '&') && (next_c == '&')) || 
                     ((c == '|') && (next_c == '|')) ||
                     ((c == '|') && (next_c == '&')) ) {
                    command_list = append_to_list( command_list, 
                                                   strndup( &(cmd[start_index]), current_index-start_index ) );
                    command_list = append_to_list( command_list, 
                                                   strndup( &(cmd[current_index]), 2 ) );
                    current_index += 2;
                    start_index = current_index;
                } else if ( (prev_c == '>') && (c == '&') ) { // ignore >& redirect                    
                    current_index++;
                } else {
                    command_list = append_to_list( command_list, 
                                                   strndup( &(cmd[start_index]), current_index-start_index ) );
                    command_list = append_to_list( command_list, 
                                                   strndup( &(cmd[current_index]), 1 ) );
                    current_index++;
                    start_index = current_index;
                }
            }
            break;

        case ' ' :
        case '\t' :
            /* If we see one or more spaces or tabs at the start of a
               (simple) command, then keep moving start_index forward,
               so that if we eventually find a command, we've already
               stepped over the white space. */

            if (start_index == current_index) {
                current_index++;
                start_index = current_index;
            } else {
                current_index++;
            }
            escaped = 0;
            break;

        default :
            escaped = 0;
            current_index++;
        }
    }

    if (start_index != current_index) {
        command_list = append_to_list( command_list, 
                                       strndup( &(cmd[start_index]), current_index-start_index ) );
    }

    //fprintf( stderr, "split_into_simple_commands returns\n" );
    //print_list( command_list );

    return command_list;
}

/* Expand any aliases in a command, which should initially be a
   "simple" command, although of course the process of expanding any
   aliases may create sub-commands within it. */

char *expand_aliases( char *command, int depth, Alias *aliases )
{
    int ends_with_space;
    size_t length;

    char *result;
    char *cmd;
    char *args;

    int i;
    List *words;
    char *alias_value;
    char *aliased_command;
    char *prev_cmd;
    List *new_command_list;
    List *new_words;
    int new_command_list_length;

    // fprintf( stderr, "expand_aliases (%s),%d ... \n", command, depth );

    /* If tcsh detects a loop in the aliases then it prints "Alias
       loop", but we simply stop expanding aliases at a certain
       depth. This is unlikely to affect any real alias expansion. */

    if ( depth >= 20 ) {
        result = strdup( command );
    } else if ( is_empty( command ) ) {
        result = strdup( command );
    } else {

        /* If the command ends with a space, then we will eventually
           make sure that the final result also ends with a space. */
        length = strlen( command );
        if ( (length > 0) && (command[length-1] == ' ') ) {
            ends_with_space = 1;
        } else {
            ends_with_space = 0;
        }

        /* Split the string at the first white space. The first entry
           on the list is then the "command" and the second contains
           all the arguments. */

        words = split_after_first_word( command );
        if ( list_length( words ) > 0 ) {
            cmd = get_nth_word( words, 0 );
            args = get_nth_word( words, 1 );
            free_list( words );

            /* If the command matches a defined alias, get the text which is is 
               supposed to expand to. */

            alias_value = lookup_alias( cmd, aliases );
            if ( alias_value != NULL ) {
                prev_cmd = strdup( cmd );

                /* Replace refernces to the "history" with values from the 
                   original command and arguments. */

                aliased_command = replace_history( alias_value, cmd, args );

                free( alias_value );
                free( cmd );

                /* Expanding the alias may very well have generated a number of 
                   sub-commands, so we must again split into simple commands. */

                new_command_list = split_into_simple_commands( aliased_command );
                new_command_list_length = list_length( new_command_list );

                result = get_nth_word( new_command_list, 0 );

                /* If the process so far has changed the command, then
                   we need to see whether this new command is itself
                   an alias. The simplest way to do this is to
                   recursively call resolve_alias, but we include an
                   explicit "depth" parameter so that we can avoid an
                   infinite loop. */

                new_words = split_after_first_word( aliased_command );
                cmd = get_nth_word( new_words, 0 );
                if ( strcmp( cmd, prev_cmd ) != 0 ) {
                    result = expand_aliases( result, depth+1, aliases );
                }
                free_list( new_words );

                free( aliased_command );
                free( cmd );
                free( prev_cmd );

                /* Recursively expand any aliases in the rest of the simple commands. */

                for ( i = 1; i < new_command_list_length; i++ ) {
                    result = append_dup_string( result, " " );
                    cmd = expand_aliases( get_nth_word( new_command_list, i ), depth+1, aliases );
                    result = append_dup_string( result, cmd );
                    free( cmd );
                }

                free_list( new_command_list );

                /* If the original command ended with a space, make
                   sure that the result does as well. */

                if ( ends_with_space ) {
                    result = append_dup_string( result, " " );
                }
            } else {
                /* The first word wasn't actually an alias. */
                free( cmd );
                result = strdup( command );
            }
        } else {
            result = strdup( "" );
        }
    }

    free( command );

    // fprintf( stderr, "expand_aliases returns (%s)\n", result );

    return result;
}

/* Expand aliases in a command, which is not necessarily a "simple"
   command. */

char *dealias_command( char *command, Alias *aliases )
{
    List *commands;
    List *entry;
    char *aliased_cmd;
    char *result = strdup( "" );

    /* Split the command into a list of simple commands. Expand any
       aliases in each of these, and accumulate all of these into a
       single string.*/

    commands = split_into_simple_commands( command );

    for ( entry = commands; entry != NULL; entry = entry->next ) {
        aliased_cmd = expand_aliases( strdup( entry->contents ), 0, aliases );
        result = append_dup_string( result, aliased_cmd );
        free( aliased_cmd );
    }

    return result;
}

/* Any text within "back-ticks" is treated as a sub-command, and as such we need to 
   expand any aliases it may contain. */

char *process_back_ticks( char *command, Alias *aliases )
{
    List *words;
    int n_words;
    int i;
    char *dealiased;
    char *word;
    char *result = strdup( "" );

    /* Split the string into "words" using a back tick as the word
       delimiter. The resulting "words" are not really words at all,
       but represent sequences which are either "outside" or "inside"
       backticks. */

    words = split( command, "`" );
    n_words = list_length( words );

    for ( i = 0; i < n_words; i++ ) {
        word = get_nth_word( words, i );

        if ( (i % 2) == 0 ) {
            /* Even numbered words are outside the back-ticks, and don't 
               need any further processing. */
            result = append_dup_string( result, word );
        } else {
            /* Odd numbered "words" were within back-ticks. Expand any
               aliases within the sub-command and wrap the result up
               in back-ticks. */
            dealiased = dealias_command( word, aliases );

            result = append_dup_string( result, "`");
            result = append_dup_string( result, dealiased );
            result = append_dup_string( result, "`");

            free( dealiased );
        }

        free( word );
    }

    free_list( words );

    return result;
}


int main( int argc, char *argv[] )
{
    Alias *aliases;
    char *cmd;
    int i;
    char *result;

    if ( argc > 1 ) {

        /* The first argument on the command line is usually the name
           of the file containing the aliases, which we read into
           memory. If however it is the string "-noalias", then we
           operate without any alias definitions.  */

        if ( strcmp( argv[1], "-noalias" ) == 0 ) {
            aliases = NULL;
        } else {
            aliases = read_alias_table( argv[1] );
        }

        // print_aliases( aliases );

        /* Gather up all the rest of the args into a single string as
           they form our command */

        cmd = strdup( "" );
        for ( i = 2; i < argc; i++ ) {
            cmd = append_dup_string( cmd, argv[i] );
            cmd = append_dup_string( cmd, " " );
        }

        /* If the command is enclosed in double quotes then remove
           the double quote from both ends. */

        cmd = get_string_in_quotes( trim( cmd ) );

        // fprintf( stderr, "Command is: %s\n", cmd );

        result = dealias_command( cmd, aliases );

        /* Any sub commands (contained in back ticks) may themselves
           contain aliases which need to be expanded. */

        result = process_back_ticks( result, aliases );

        /* Remove any quotes (") from the string, unless they are escaped 
           with a backslash (\") */

        result = remove_quotes( result );

        /* Convert any occurence of "\!" into "!". */
        result = remove_backslash( result, '!' );

        printf( "%s\n", result );
        free( result );

        free( cmd );
    } else {
        fprintf( stderr, "\nTake a tcsh alias table and a tcsh command and print the command after\n" );
        fprintf( stderr, "alias substitution. The alias table can be created from within tcsh by:\n" );
        fprintf( stderr, "  alias > alias.txt\n\n" );

        fprintf( stderr, "usage: %s <alias-table> <cmd args ...>\n", argv[0] );
    }

    return 0;
}
