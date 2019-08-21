
#ifndef GPCLIENT_EXPORT_H
#define GPCLIENT_EXPORT_H

#ifdef GPCLIENT_STATIC_DEFINE
#  define GPCLIENT_EXPORT
#  define GPCLIENT_NO_EXPORT
#else
#  ifndef GPCLIENT_EXPORT
#    ifdef gpclient_EXPORTS
        /* We are building this library */
#      define GPCLIENT_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define GPCLIENT_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef GPCLIENT_NO_EXPORT
#    define GPCLIENT_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef GPCLIENT_DEPRECATED
#  define GPCLIENT_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef GPCLIENT_DEPRECATED_EXPORT
#  define GPCLIENT_DEPRECATED_EXPORT GPCLIENT_EXPORT GPCLIENT_DEPRECATED
#endif

#ifndef GPCLIENT_DEPRECATED_NO_EXPORT
#  define GPCLIENT_DEPRECATED_NO_EXPORT GPCLIENT_NO_EXPORT GPCLIENT_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef GPCLIENT_NO_DEPRECATED
#    define GPCLIENT_NO_DEPRECATED
#  endif
#endif

#endif
