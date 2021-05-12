#!/bin/sh
# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

ORIGDIR=`pwd`
cd $srcdir

PROJECT=dbus-glib
TEST_TYPE=-f
FILE=dbus-glib-1.pc.in

DEFAULT_CONFIGURE_ARGS="--enable-maintainer-mode --enable-gtk-doc --enable-tests --enable-verbose-mode --enable-checks --enable-asserts"

# Not all echo versions allow -n, so we check what is possible. This test is
# based on the one in autoconf.
case `echo "testing\c"; echo 1,2,3`,`echo -n testing; echo 1,2,3` in
  *c*,-n*) ECHO_N= ;;
  *c*,*  ) ECHO_N=-n ;;
  *)       ECHO_N= ;;
esac


# some terminal codes ...
boldface="`tput bold 2>/dev/null`"
normal="`tput sgr0 2>/dev/null`"
printbold() {
    echo $ECHO_N "$boldface"
    echo "$@"
    echo $ECHO_N "$normal"
}    
printerr() {
    echo "$@" >&2
}

# Usage:
#     compare_versions MIN_VERSION ACTUAL_VERSION
# returns true if ACTUAL_VERSION >= MIN_VERSION
compare_versions() {
    ch_min_version=$1
    ch_actual_version=$2
    ch_status=0
    IFS="${IFS=         }"; ch_save_IFS="$IFS"; IFS="."
    set $ch_actual_version
    for ch_min in $ch_min_version; do
        ch_cur=`echo $1 | sed 's/[^0-9].*$//'`; shift # remove letter suffixes
        if [ -z "$ch_min" ]; then break; fi
        if [ -z "$ch_cur" ]; then ch_status=1; break; fi
        if [ $ch_cur -gt $ch_min ]; then break; fi
        if [ $ch_cur -lt $ch_min ]; then ch_status=1; break; fi
    done
    IFS="$ch_save_IFS"
    return $ch_status
}

# Usage:
#     version_check PACKAGE VARIABLE CHECKPROGS MIN_VERSION SOURCE
# checks to see if the package is available
version_check() {
    vc_package=$1
    vc_variable=$2
    vc_checkprogs=$3
    vc_min_version=$4
    vc_source=$5
    vc_status=1

    vc_checkprog=`eval echo "\\$$vc_variable"`
    if [ -n "$vc_checkprog" ]; then
	printbold "using $vc_checkprog for $vc_package"
	return 0
    fi

    printbold "checking for $vc_package >= $vc_min_version..."
    for vc_checkprog in $vc_checkprogs; do
	echo $ECHO_N "  testing $vc_checkprog... "
	if $vc_checkprog --version < /dev/null > /dev/null 2>&1; then
	    vc_actual_version=`$vc_checkprog --version | head -n 1 | \
                               sed 's/^.*[ 	]\([0-9.]*[a-z]*\).*$/\1/'`
	    if compare_versions $vc_min_version $vc_actual_version; then
		echo "found $vc_actual_version"
		# set variable
		eval "$vc_variable=$vc_checkprog"
		vc_status=0
		break
	    else
		echo "too old (found version $vc_actual_version)"
	    fi
	else
	    echo "not found."
	fi
    done
    if [ "$vc_status" != 0 ]; then
	printerr "***Error***: You must have $vc_package >= $vc_min_version installed"
	printerr "  to build $PROJECT.  Download the appropriate package for"
	printerr "  from your distribution or get the source tarball at"
        printerr "    $vc_source"
	printerr
    fi
    return $vc_status
}

#tell Mandrake autoconf wrapper we want autoconf 2.5x, not 2.13
WANT_AUTOCONF_2_5=1
export WANT_AUTOCONF_2_5
version_check autoreconf AUTORECONF 'autoreconf' 2.59 \
    "http://ftp.gnu.org/pub/gnu/autoconf/autoconf-2.59.tar.gz" || DIE=1

automake_progs="automake automake-1.9"

version_check automake AUTOMAKE "$automake_progs" 1.9 \
    "http://ftp.gnu.org/pub/gnu/automake/automake-1.9.tar.gz" || DIE=1
ACLOCAL=`echo $AUTOMAKE | sed s/automake/aclocal/`
ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

version_check gtkdoc GTKDOC 'gtkdoc-scan' 1.6 \
   "http://ftp.gnome.org/pub/GNOME/sources/gtk-doc/" || DIE=1

test $TEST_TYPE $FILE || {
	printbold "You must run this script in the top-level $PROJECT directory"
	exit 1
}

if test -z "$*"; then
        printbold
	printbold "I am going to run ./configure with default arguments:"
        printbold "$DEFAULT_CONFIGURE_ARGS"
        printbold "If you wish to pass any others to it, please specify them on the"
        printbold "$0 command line."
        printbold
fi


libtoolize --force || echo "libtoolize failed"
gtkdocize || echo "gtkdocize failed"

export AUTOMAKE ACLOCAL
autoreconf --install || exit 1

DIE=0
cd $ORIGDIR

if test -z "$NOCONFIGURE"; then
    run_configure=true
    for arg in $*; do
	case $arg in 
            --no-configure)
		run_configure=false
		;;
            *)
		;;
	esac
    done
else
    run_configure=false
fi

if $run_configure; then
    if test -z "$*"; then
        $srcdir/configure $DEFAULT_CONFIGURE_ARGS
    else
        $srcdir/configure --enable-maintainer-mode  "$@"
    fi
    echo 
    echo "Now type 'make' to compile $PROJECT."
else
    echo
    echo "Now run 'configure' and 'make' to compile $PROJECT."
fi

