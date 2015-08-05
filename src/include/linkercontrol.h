/*
 * linkercontrol.h - a common header for controlling visibility of functions and methods
 *
 * $Revision: 4377 $ $Author: heiko $
 *
 * (c) 2012-2015 Heiko Schäfer <heiko@hgl.rangun.de>
 *
 * LICENCE is inherited by project using this file
 *
 */

#ifndef COMMONS_LINKERCONTROL_H
#define COMMONS_LINKERCONTROL_H

namespace Commons {

// Generic helper definitions for shared library support
#if (defined _WIN32 || defined __CYGWIN__)
#define _IMPORT __declspec(dllimport)
#define _EXPORT __declspec(dllexport)
#define _LOCAL
#define _INTERNAL
#else
#if __GNUC__ >= 4
#define _IMPORT   __attribute__ ((visibility ("default")))
#define _EXPORT   __attribute__ ((visibility ("default")))
#define _LOCAL    __attribute__ ((visibility ("hidden")))
#define _INTERNAL __attribute__ ((visibility ("internal")))
#else
#define _IMPORT
#define _EXPORT
#define _LOCAL
#define _INTERNAL
#endif
#endif

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
	_INTERNAL explicit TypeName(const TypeName&); \
	_INTERNAL TypeName& operator=(const TypeName&);

#define FINALBASE(P) \
	template<class C> \
	class _EXPORT __final##P { \
		_INTERNAL __final##P(const __final##P&); \
		_INTERNAL __final##P& operator=(const __final##P&); \
		_INTERNAL __final##P() {} \
		_INTERNAL virtual ~__final##P() {} \
		friend class P; \
	};

#define FINALCC(P) __final##P<P>()

#define FINAL(P) public virtual __final##P<P>

#define GCC_VERSION (__GNUC__ * 10000 \
					 + __GNUC_MINOR__ * 100 \
					 + __GNUC_PATCHLEVEL__)

#ifndef _PACKED
#define _PACKED __attribute__ ((__packed__))
#endif

#ifndef _WEAK
#define _WEAK __attribute__ ((__weak__))
#endif

#ifndef _NORETURN
#define _NORETURN __attribute__ ((__noreturn__))
#endif

#ifndef _NOTHROW
#define _NOTHROW __attribute__ ((__nothrow__))
#endif

#ifndef _NOUNUSED
#define _NOUNUSED __attribute__ ((warn_unused_result))
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wvariadic-macros"
#endif
#ifndef _NONNULL
#define _NONNULL(...) __attribute__ ((nonnull (__VA_ARGS__)))
#endif
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef __clang__
#define _LTO_EXT_VISIBLE
#else
#define _LTO_EXT_VISIBLE __attribute__ ((externally_visible))
#endif

#ifndef _NONNULL_RETURN
#define _NONNULL_RETURN __attribute__ ((returns_nonnull))
#endif

#ifndef _NONNULL_ALL
#define _NONNULL_ALL __attribute__ ((nonnull))
#endif

#ifndef _PURE
#define _PURE __attribute__ ((pure))
#endif

#ifndef _WIN32
#define _NWPURE _PURE
#else
#define _NWPURE
#endif

#ifndef _WIN32
#define _WPURE
#else
#define _WPURE _PURE
#endif

#ifndef _CONST
#define _CONST __attribute__ ((const))
#endif

#ifndef _INIT_PRIO
#define _INIT_PRIO(n) __attribute__ ((init_priority (n)))
#endif

#ifndef _WIN32
#define _NWCONST _CONST
#else
#define _NWCONST
#endif

#ifndef _WIN32
#define _WCONST
#else
#define _WCONST _CONST
#endif

#ifndef _HOT
#if GCC_VERSION < 40300
#define _HOT
#else
#define _HOT __attribute__ ((hot))
#endif
#endif

#ifndef _COLD
#if GCC_VERSION < 40300
#define _COLD
#else
#define _COLD __attribute__ ((cold))
#endif
#endif

#ifndef _DEPRECATED
#define _DEPRECATED __attribute__((deprecated))
#endif

#if GCC_VERSION >= 40800 || (defined(__x86_64__) && GCC_VERSION >= 40700)
#define likely(x)   (x)
#define unlikely(x) (x)
#else
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#ifndef _UNUSED
#define _UNUSED(x) (void)(x)
#endif

}

#endif /* COMMONS_LINKERCONTROL_H */

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
