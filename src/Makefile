# This file is part of tcshParser.
# Copyright (C) 2013 Ellexus (www.ellexus.com)
#
# tcshParser is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

CFLAGS = -std=c99 -g -Wall

tcshParser:	tcshParser.o list_support.o string_support.o alias_support.o

tcshParser.o:	tcshParser.c list_support.h string_support.h alias_support.h

list_support.o:	list_support.c list_support.h string_support.h

string_support.o:	string_support.c string_support.h

alias_support.o:	alias_support.c alias_support.h


clean:
	rm -f *.o tcshParser


