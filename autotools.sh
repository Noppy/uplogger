#!/bin/sh
touch INSTALL NEWS README LICENSE AUTHORS ChangeLog
echo aclocal
aclocal
echo autoheader
autoheader
echo automake
automake --add-missing --copy
echo autoconf
autoconf

