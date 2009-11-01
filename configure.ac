# License here
#

AC_PREREQ(2.59)
AC_INIT(evemu, 0.6-trunk, nospam@evemu.sf.net)
AM_CONFIG_HEADER(config.h)

if test x$prefix = xNONE; then
        prefix=/usr/local
fi

AC_SUBST(prefix)

AC_SUBST(INCLUDES)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

AM_INIT_AUTOMAKE
AM_PROG_CC_C_O
AC_PROG_MAKE_SET

# Templates
AH_TEMPLATE([USING_BIG_ENDIAN], Define this if you are on a big-endian system.)
AH_TEMPLATE([USING_LITTLE_ENDIAN], Define this if you are on a non-big-endian system.)
AH_TEMPLATE([X64], Define this if you are under an X64-based operating system.)
AH_TEMPLATE([X86], Define this if you are under an X86-based 32bit operating system.)
AH_TEMPLATE([TINYXML_USE_STL], Define this if you want tinyxml to use native STL.)
AH_TEMPLATE([EVEMU_REVISION], The revision of the latest checkout)

# Checking for a 64bit system
AC_MSG_CHECKING([uname -m for detecting host operating system architecture])
ARCH=`uname -m`
case "$ARCH" in
	amd64*)
		dnl yay 21st century hardware
		AC_MSG_RESULT(x64 feel teh power)
		AC_DEFINE(X64)
		AC_SUBST(ARCH,x64)
	;;
	x86_64*)
		dnl yay 21st century hardware
		AC_MSG_RESULT(x64 feel teh power)
		AC_DEFINE(X64)
		AC_SUBST(ARCH,x64)
	;;
	*)
		AC_MSG_RESULT(x86 oh noes i ran out of address space)
		AC_DEFINE(X86)
		AC_SUBST(ARCH,x86)
	;;
esac

# Check for BigEndian
AC_C_BIGENDIAN([AC_DEFINE(USING_BIG_ENDIAN)],[AC_DEFINE(USING_LITTLE_ENDIAN)],[])

# Custom part
# Collector
AC_ARG_ENABLE(collector, AC_HELP_STRING([--enable-collector],[enable the eve collector]),
[case "${enableval}" in
  yes) collector=true ;;
  no)  collector=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-collector) ;;
esac],[anvil=false])
AM_CONDITIONAL(COLLECTOR, test x$collector = xtrue)

# EVETool
AC_ARG_ENABLE(evetool, AC_HELP_STRING([--enable-evetool],[enable the eve tool]),
[case "${enableval}" in
  yes) evetool=true ;;
  no)  evetool=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-evetool) ;;
esac],[anvil=false])
AM_CONDITIONAL(EVETOOL, test x$evetool = xtrue)

# Checks for libraries.

# Mysql >5.0
AX_LIB_MYSQL([5.0])
AC_SUBST(MYSQL_CFLAGS)
AC_SUBST(MYSQL_LDFLAGS)

# Check for zlib
AC_CHECK_LIB(z, deflate,, AC_MSG_ERROR([can't find zlib]))

# Collector-specific libraries
if test x$collector = xtrue; then
	# Check for pcap
	AC_CHECK_LIB(pcap, pcap_open_live,, AC_MSG_ERROR([can't find libpcap]))
	AC_CHECK_HEADERS(pcap.h,, AC_MSG_ERROR([can't find pcap.h]))

	# nids
	AC_CHECK_LIB(nids, nids_init,, AC_MSG_ERROR([can't find libnids]))
	AC_CHECK_HEADERS(nids.h,, AC_MSG_ERROR([can't find nids.h]))

	# libnet
	AC_CHECK_LIB(net, main,, AC_MSG_ERROR([can't find libnet]))
	LIBNET_CFLAGS=`libnet-config --defines`
	AC_SUBST(LIBNET_CFLAGS)
fi

# Define compile/library flags.
AC_DEFINE(TINYXML_USE_STL)
AC_SUBST(AM_CXXFLAGS, "$AM_CXXFLAGS -pipe -fno-inline -Wall -ansi")
AC_SUBST(AM_LDFLAGS)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

# Checks for library functions.

# Extract the latest SVN revision
AC_CHECK_PROG([SVNVERSION], [svnversion], yes)
if test -z "$SVNVERSION"; then
	AC_MSG_ERROR(Subversion 'svnversion' not found)
else
	EVEMU_REVISION=`/usr/bin/env svnversion -n`
	if test -z "$EVEMU_REVISION"; then EVEMU_REVISION="n/a"; fi

	AC_MSG_NOTICE(Extracted SVN version: $EVEMU_REVISION)
	AC_DEFINE_UNQUOTED(EVEMU_REVISION,["\$Rev: $EVEMU_REVISION \$"])
	AC_SUBST(EVEMU_REVISION)
fi

AC_CONFIG_FILES([
	Makefile
	dep/Makefile
	dep/gangsta/Makefile
	dep/tinyxml/Makefile
	dep/utils/Makefile
	src/common/Makefile
	src/xmlpktgen/Makefile
	src/eve-common/Makefile
	src/eve-common/packets/Makefile
	src/eve-server/Makefile
	src/eve-tool/Makefile
	src/eve-collector/Makefile
])

AC_OUTPUT
