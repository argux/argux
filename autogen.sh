libtoolize
aclocal -I m4
intltoolize --force
autoheader
autoconf --force
automake --add-missing --copy --force-missing --gnu
./configure --enable-maintainer-mode
