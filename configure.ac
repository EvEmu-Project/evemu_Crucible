# License here
#

#########################
# Autoconf requirements #
#########################
AC_PREREQ(2.59)
AC_INIT([evemu],[svn-trunk],[nospam@evemu.sf.net])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

###########################
# Autoheader requirements #
###########################
AH_TEMPLATE([DEBUG], Define this if you are running debug build.)
AH_TEMPLATE([NDEBUG], Define this if you are running non-debug build.)
AH_TEMPLATE([USING_BIG_ENDIAN], Define this if you are on a big-endian system.)
AH_TEMPLATE([USING_LITTLE_ENDIAN], Define this if you are on a non-big-endian system.)
AH_TEMPLATE([X64], Define this if you are under an X64-based operating system.)
AH_TEMPLATE([X86], Define this if you are under an X86-based 32bit operating system.)
AH_TEMPLATE([TINYXML_USE_STL], Define this if you want tinyxml to use native STL.)
AH_TEMPLATE([EVEMU_REVISION], The revision of the latest checkout)

######################
# User configuration #
######################

# Define our default CXXFLAGS: prevents AC_PROG_CXX
# from defining its own default
if test -z "$CXXFLAGS"; then
        CXXFLAGS=" ";
fi

# Debugging symbols, default to false
AC_ARG_ENABLE(debug, AC_HELP_STRING([--enable-debug],[enable debugging symbols]),
[case "${enableval}" in
  yes) debug=true ;;
  no)  debug=false ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
esac],[debug=false])

# Optimization, default to true
AC_ARG_ENABLE(optimization, AC_HELP_STRING([--enable-optimization],[enable code optimization]),
[case "${enableval}" in
  yes) optimization=true ;;
  no)  optimization=false ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for --enable-optimization) ;;
esac],[optimization=true])

# EVECollector, default to false
AC_ARG_ENABLE(evecollector, AC_HELP_STRING([--enable-evecollector],[enable the eve collector]),
[case "${enableval}" in
  yes) evecollector=true ;;
  no)  evecollector=false ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for --enable-evecollector) ;;
esac],[evecollector=false])

# EVETool, default to false
AC_ARG_ENABLE(evetool, AC_HELP_STRING([--enable-evetool],[enable the eve tool]),
[case "${enableval}" in
  yes) evetool=true ;;
  no)  evetool=false ;;
  *)   AC_MSG_ERROR(bad value ${enableval} for --enable-evetool) ;;
esac],[evetool=false])

#######################
# Checks for programs #
#######################
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

AC_CHECK_PROG([SVNVERSION], [svnversion], yes)

# Checking for a 64bit system
AC_MSG_CHECKING([uname -m for detecting host operating system architecture])
case `uname -m` in
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

########################
# Checks for libraries #
########################

# Mysql >5.0
AX_LIB_MYSQL([5.0])

# Check for zlib
AC_CHECK_LIB(z, deflate,, AC_MSG_ERROR([can't find zlib]))

# EVECollector-specific libraries
if test x$evecollector = xtrue; then
	# Check for pcap
	AC_CHECK_LIB(pcap, pcap_open_live,, AC_MSG_ERROR([can't find libpcap]))

	# nids
	AC_CHECK_LIB(nids, nids_init,, AC_MSG_ERROR([can't find libnids]))

	# libnet
	AC_CHECK_LIB(net, main,, AC_MSG_ERROR([can't find libnet]))
fi

###########################
# Checks for header files #
###########################
AC_HEADER_STDBOOL

# EVECollector-specific includes
if test x$evecollector = xtrue; then
	# Check for pcap
	AC_CHECK_HEADERS(pcap.h,, AC_MSG_ERROR([can't find pcap.h]))

	# nids
	AC_CHECK_HEADERS(nids.h,, AC_MSG_ERROR([can't find nids.h]))
fi

####################
# Checks for types #
####################

#########################
# Checks for structures #
#########################

#######################################
# Checks for compiler characteristics #
#######################################
AC_C_CONST
AC_C_INLINE

################################
# Checks for library functions #
################################

#########################
# Package configuration #
#########################

# set default prefix
AC_PREFIX_DEFAULT(/usr/local)
AC_SUBST(prefix)

# Define compile/library flags.
AC_DEFINE(TINYXML_USE_STL)

EVEMU_CXXFLAGS="-pipe -fno-inline -Wall -ansi"

if test x$debug = xtrue; then
        AC_DEFINE(DEBUG)
        EVEMU_CXXFLAGS="$EVEMU_CXXFLAGS -g"
else
        AC_DEFINE(NDEBUG)
fi

if test x$optimization = xtrue; then
        EVEMU_CXXFLAGS="$EVEMU_CXXFLAGS -O2"
fi

if test x$evecollector = xtrue; then
	LIBNET_CXXFLAGS=`libnet-config --defines`
fi

AC_SUBST(EVEMU_CXXFLAGS)
AC_SUBST(LIBNET_CXXFLAGS)
AC_SUBST(AM_CXXFLAGS)

AC_SUBST(AM_LDFLAGS)

# Extract the latest SVN revision
if test -z "$SVNVERSION"; then
	AC_MSG_WARN(Subversion 'svnversion' not found)
else
	EVEMU_REVISION=`/usr/bin/env svnversion -n`
fi

if test -z "$EVEMU_REVISION"; then
        EVEMU_REVISION="N/A";
fi

AC_MSG_NOTICE(Extracted SVN version: $EVEMU_REVISION)
AC_DEFINE_UNQUOTED(EVEMU_REVISION,["\$Rev: $EVEMU_REVISION \$"])
AC_SUBST(EVEMU_REVISION)

# Define automake conditionals
AM_CONDITIONAL(EVECOLLECTOR, test x$evecollector = xtrue)
AM_CONDITIONAL(EVETOOL, test x$evetool = xtrue)

########################
# Makefiles and output #
########################
AC_CONFIG_FILES([
	Makefile
	dep/gangsta/Makefile
	dep/tinyxml/Makefile
	dep/utf8cpp/Makefile
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
