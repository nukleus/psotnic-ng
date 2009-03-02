#ifndef CONFIG_H
#define CONFIG_H 1

#ifndef HAVE_LITTLE_ENDIAN
#cmakedefine HAVE_LITTLE_ENDIAN
#endif

#ifdef HAVE_IPV6
#cmakedefine HAVE_IPV6
#endif

#ifndef HAVE_ADNS
#cmakedefine HAVE_ADNS
#endif

#ifndef HAVE_ADNS_PTHREAD
#cmakedefine HAVE_ADNS_PTHREAD
#endif

#ifndef HAVE_ADNS_FIREDNS
#cmakedefine HAVE_ADNS_FIREDNS
#endif

#ifndef HAVE_IRC_BACKTRACE
#cmakedefine HAVE_IRC_BACKTRACE
#endif

#ifndef HAVE_DEBUG
#cmakedefine HAVE_DEBUG
#endif

#ifndef HAVE_SSL
#cmakedefine HAVE_SSL
#endif

#ifndef SVN_REVISION
#	define SVN_REVISION	"UNKNOWN"
#endif

#ifndef HAVE_TCL
#cmakedefine HAVE_TCL
#endif

#endif /* CONFIG_H */
