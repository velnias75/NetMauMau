/**
 * basiclogger.h - template for basic logging functionality
 *
 * $Revision: 4460 $ $Author: heiko $
 *
 * (c) 2012-2015 Heiko Schäfer <heiko@rangun.de>
 *
 * LICENSE is inherited by the project using this file
 *
 **/

#ifndef COMMONS_BASICLOGGER_H
#define COMMONS_BASICLOGGER_H

#if defined(HAVE_CONFIG_H) || defined(IN_KDEVELOP_PARSER)
#include "config.h"
#endif

#include <cmath>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <typeinfo>

#ifndef BASICLOGGER_NO_PTHREADS
#include <pthread.h>
#endif

#ifdef HAVE_GCC_ABI_DEMANGLE
#include <cstdlib>
#include <cxxabi.h>
#endif

#include "linkercontrol.h"

#ifndef LOGGER_NULL
#define LOGGER_NULL "(null)"
#endif

#ifndef LLOGGER_NULL
#define LLOGGER_NULL L"(null)"
#endif

#ifndef LOGGER_PREFIX
#define LOGGER_PREFIX ""
#endif

#ifndef LOGGER_CLASS
#define LOGGER_CLASS Commons::BasicLogger
#endif

#define logSwitchBuf(n) volatile const Commons::BasicLoggerBufferSwitcher \
	<LOGGER_CLASS::output_iterator, LOGGER_CLASS::BUFCNT> \
	__basicLogger__buffer__switcher__##n__(n); _UNUSED(__basicLogger__buffer__switcher__##n__)

#ifndef NDEBUG
#define logDebug(msg)		{ LOGGER_CLASS::setCurrentBuf(0u); \
		(LOGGER_CLASS(LOGGER_CLASS::LOG_DEBUG))   << LOGGER_PREFIX << msg; }
#define logDebugN(n,msg) 	{ logSwitchBuf(n); \
		(LOGGER_CLASS(LOGGER_CLASS::LOG_DEBUG))   << LOGGER_PREFIX << msg; }
#else
#define logDebug(msg)		{}
#define logDebugN(n,msg)	{}
#endif
#define logger(msg)			{ LOGGER_CLASS::setCurrentBuf(0u); \
		(LOGGER_CLASS(LOGGER_CLASS::LOG_NONE))    << LOGGER_PREFIX << msg; }
#define loggerN(n,msg)		{ logSwitchBuf(n); \
		(LOGGER_CLASS(LOGGER_CLASS::LOG_NONE))    << LOGGER_PREFIX << msg; }
#define logInfo(msg)		{ LOGGER_CLASS::setCurrentBuf(0u); \
		(LOGGER_CLASS(LOGGER_CLASS::LOG_INFO))    << LOGGER_PREFIX << msg; }
#define logInfoN(n,msg)		{ logSwitchBuf(n); \
		(LOGGER_CLASS(LOGGER_CLASS::LOG_INFO))    << LOGGER_PREFIX << msg; }
#define logWarning(msg)		{ LOGGER_CLASS::setCurrentBuf(0u); \
		(LOGGER_CLASS(LOGGER_CLASS::LOG_WARNING)) << LOGGER_PREFIX << msg; }
#define logWarningN(n,msg)	{ logSwitchBuf(n); \
		(LOGGER_CLASS(LOGGER_CLASS::LOG_WARNING)) << LOGGER_PREFIX << msg; }
#define logError(msg)		{ LOGGER_CLASS::setCurrentBuf(0u); \
		(LOGGER_CLASS(LOGGER_CLASS::LOG_ERROR))   << LOGGER_PREFIX << msg; }
#define logErrorN(n,msg)	{ logSwitchBuf(n); \
		(LOGGER_CLASS(LOGGER_CLASS::LOG_ERROR))   << LOGGER_PREFIX << msg; }
#define logFatal(msg)		{ LOGGER_CLASS::setCurrentBuf(0u); \
		(LOGGER_CLASS(LOGGER_CLASS::LOG_FATAL))   << LOGGER_PREFIX << msg; }
#define logFatalN(n,msg)	{ logSwitchBuf(n); \
		(LOGGER_CLASS(LOGGER_CLASS::LOG_FATAL))   << LOGGER_PREFIX << msg; }

#ifndef BASICLOGGER_NO_PTHREADS
namespace {
pthread_mutex_t _basicLoggerLock = PTHREAD_MUTEX_INITIALIZER;
}
#endif

namespace Commons {

template<class, std::size_t> class IPostLogger;

template<class OIter, std::size_t BUFFERS = 1>
class _EXPORT BasicLogger {
	DISALLOW_COPY_AND_ASSIGN(BasicLogger)
public:
	enum { BUFCNT = BUFFERS };

	typedef typename std::basic_ostringstream<typename OIter::char_type>::__string_type logString;
	typedef OIter output_iterator;

	typedef enum _PACKED {
		LOG_NONE		= 0x01,
		LOG_DEBUG		= 0x02,
		LOG_INFO		= 0x04,
		LOG_WARNING		= 0x08,
		LOG_ERROR		= 0x10,
		LOG_FATAL		= 0x20
	} LEVEL;

	virtual ~BasicLogger();

	typedef struct _PACKED _width {
		// cppcheck-suppress functionStatic
		explicit _width(std::streamsize w) : m_width(w) {}
		std::streamsize m_width;
	} width;

	typedef struct _nonl {} nonl;

	typedef struct _PACKED _abool {
		// cppcheck-suppress functionStatic
		explicit _abool(bool ab) : b(ab) {}
		bool b;
	} abool;

	typedef struct _time {
		_time(const char *f = "%a, %d %b %Y %T %z", const tm *t = NULL) : fmt(f), time(t) {}
		const char *fmt;
		const tm *time;
		static bool enabled;
	} time;

	static const std::ios_base::fmtflags hex;
	static const std::ios_base::fmtflags dec;

	static void setSilentMask(unsigned char mask = 0x00) {
		m_silentMask = mask;
	}

	static unsigned char getSilentMask() {
		return m_silentMask;
	}

	static std::size_t getLogCount(const LEVEL &level) _PURE;

	inline static std::size_t getCurrentBuf() {
		return m_bufNo;
	}

	inline static void setCurrentBuf(std::size_t n) {
		m_bufNo = (!(n && m_bufNo == n) ? n : (n >= (BUFFERS - 1u) ? 1u : n + 1u));
	}

	virtual BasicLogger &operator<<(const std::ios_base::fmtflags &f);
	virtual BasicLogger &operator<<(const BasicLogger::nonl &nonl);
	virtual BasicLogger &operator<<(const BasicLogger::width &w);
	virtual BasicLogger &operator<<(const BasicLogger::time &t);

	virtual BasicLogger &operator<<(const logString &msg);
#ifndef LOG_CHAR
	virtual BasicLogger &operator<<(const std::string &msg);
#endif
	virtual BasicLogger &operator<<(const std::exception &e);
	virtual BasicLogger &operator<<(const std::type_info &ti);

	virtual BasicLogger &operator<<(short s);
	virtual BasicLogger &operator<<(int i);
	virtual BasicLogger &operator<<(long l);
	virtual BasicLogger &operator<<(long long ll);

	virtual BasicLogger &operator<<(unsigned short s);
	virtual BasicLogger &operator<<(unsigned int i);
	virtual BasicLogger &operator<<(unsigned long l);
	virtual BasicLogger &operator<<(unsigned long long ll);

	virtual BasicLogger &operator<<(float f);
	virtual BasicLogger &operator<<(const std::string *msg);
	virtual BasicLogger &operator<<(const abool &b);
	virtual BasicLogger &operator<<(const char *s);
	virtual BasicLogger &operator<<(const wchar_t *s);

protected:
	BasicLogger(const OIter &out, const LEVEL &level,
				const IPostLogger<OIter, BUFFERS> *post = 0L);

	_INTERNAL LEVEL getLevel() const {
		return m_level;
	}

	_INTERNAL std::basic_ostringstream<typename OIter::char_type> &getMessageStream() {
		return m_buf[m_bufNo];
	}

private:
	_INTERNAL void init(const LEVEL &level);
	_INTERNAL void initMessageString();
	_INTERNAL const char *getLevelString(LEVEL lvl) const _CONST;
	_INTERNAL static const std::string demangle(const char *name);

private:
	static std::basic_ostringstream<typename OIter::char_type> m_buf[BUFFERS];
	static volatile std::size_t m_bufNo;

#if GCC_VERSION < 40300
	const OIter m_out;
#else
	const OIter &m_out;
#endif

	static struct _logcount {
		size_t none;
		size_t debug;
		size_t info;
		size_t warning;
		size_t error;
		size_t fatal;
	} m_logCount;

	// we need this in order to get the combination LTO & inetd to work
	_LTO_EXT_VISIBLE static unsigned char m_silentMask;

	LEVEL m_level;

	bool m_noNewline;
	bool m_silent;

	const IPostLogger<OIter, BUFFERS> *m_postLogger;
};

template<class OIter, std::size_t BUFFERS = 1>
class BasicLoggerBufferSwitcher {
	DISALLOW_COPY_AND_ASSIGN(BasicLoggerBufferSwitcher)
public:
	explicit BasicLoggerBufferSwitcher(std::size_t n) :
		m_curBuf(BasicLogger<OIter, BUFFERS>::getCurrentBuf()) {
		BasicLogger<OIter, BUFFERS>::setCurrentBuf(n);
	}

	~BasicLoggerBufferSwitcher() {
		BasicLogger<OIter, BUFFERS>::setCurrentBuf(m_curBuf);
	}

private:
	std::size_t m_curBuf;
};

template<class OIter, std::size_t BUFFERS>
class IPostLogger {
	DISALLOW_COPY_AND_ASSIGN(IPostLogger)
public:
	inline virtual ~IPostLogger() {}

	virtual void postAction(const typename BasicLogger < OIter,
							BUFFERS >::logString &ls) const throw() = 0;

protected:
	inline IPostLogger() {}
};

#if !defined(__clang__)
#pragma GCC diagnostic ignored "-Wunsafe-loop-optimizations"
#pragma GCC diagnostic push
#endif
template<class OIter, std::size_t BUFFERS>
std::basic_ostringstream<typename OIter::char_type> BasicLogger<OIter, BUFFERS>::m_buf[BUFFERS];
#if !defined(__clang__)
#pragma GCC diagnostic pop
#endif

template<class OIter, std::size_t BUFFERS>
volatile std::size_t BasicLogger<OIter, BUFFERS>::m_bufNo = 0u;

template<class OIter, std::size_t BUFFERS>
unsigned char BasicLogger<OIter, BUFFERS>::m_silentMask = 0x00;

template<class OIter, std::size_t BUFFERS>
bool BasicLogger<OIter, BUFFERS>::_time::enabled = true;

template<class OIter, std::size_t BUFFERS>
struct BasicLogger<OIter, BUFFERS>::_logcount
		BasicLogger<OIter, BUFFERS>::m_logCount = { 0, 0, 0, 0, 0, 0 };

template<class OIter, std::size_t BUFFERS>
const std::ios_base::fmtflags BasicLogger<OIter, BUFFERS>::hex = std::ios_base::hex;

template<class OIter, std::size_t BUFFERS>
const std::ios_base::fmtflags BasicLogger<OIter, BUFFERS>::dec = std::ios_base::dec;

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS>::BasicLogger(const OIter &out,
		const BasicLogger<OIter, BUFFERS>::LEVEL &level,
		const IPostLogger<OIter, BUFFERS> *post) : m_out(out), m_level(level), m_noNewline(false),
	m_silent(false), m_postLogger(post) {
	init(level);
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS>::~BasicLogger() {

#ifndef BASICLOGGER_NO_PTHREADS
	pthread_mutex_lock(&_basicLoggerLock);
#endif

	const logString ps(m_buf[m_bufNo].str());

	for(std::size_t i = 0u; i < BUFFERS; ++i) {

		if(!m_buf[i].str().empty()) {

			if(!m_noNewline) m_buf[i] << std::endl;

			const logString &s(m_buf[i].str());

			if(!m_silent) std::copy(s.begin(), s.end(), m_out);

			m_buf[i].str(logString());
		}
	}

	if(m_postLogger) m_postLogger->postAction(ps);

#ifndef BASICLOGGER_NO_PTHREADS
	pthread_mutex_unlock(&_basicLoggerLock);
#endif
}

template<class OIter, std::size_t BUFFERS>
void BasicLogger<OIter, BUFFERS>::init(const BasicLogger<OIter, BUFFERS>::LEVEL &level) {
#ifndef BASICLOGGER_NO_PTHREADS
	pthread_mutex_lock(&_basicLoggerLock);
#endif

	m_silent = !((m_silentMask & static_cast<unsigned char>(level)) == 0);

	std::new_handler hdl = std::set_new_handler(0L);

	m_buf[m_bufNo].precision(0);

#ifdef __EXCEPTIONS

	try {
#endif
		initMessageString();
#ifdef __EXCEPTIONS
	} catch(const std::bad_alloc &) {
		std::set_new_handler(hdl);
		throw "Error initialising logger!";
	}

#endif

	std::set_new_handler(hdl);

#ifndef BASICLOGGER_NO_PTHREADS
	pthread_mutex_unlock(&_basicLoggerLock);
#endif
}

template<class OIter, std::size_t BUFFERS>
const char *BasicLogger<OIter, BUFFERS>::getLevelString(LEVEL lvl) const {
	switch(lvl) {
	case LOG_INFO:
		return "INFO: ";

	case LOG_WARNING:
		return "WARNING: ";

	case LOG_ERROR:
		return "ERROR: ";

	case LOG_FATAL:
		return "FATAL: ";

	case LOG_DEBUG:
		return "DEBUG: ";

	case LOG_NONE:
	default:
		return "";
	}
}

template<class OIter, std::size_t BUFFERS>
void BasicLogger<OIter, BUFFERS>::initMessageString() {

	LEVEL lvl = getLevel();

	getMessageStream() << getLevelString(lvl);

	switch(lvl) {
	case LOG_INFO:
		++m_logCount.info;
		break;

	case LOG_WARNING:
		++m_logCount.warning;
		break;

	case LOG_ERROR:
		++m_logCount.error;
		break;

	case LOG_FATAL:
		++m_logCount.fatal;
		break;

	case LOG_DEBUG:
		++m_logCount.debug;
		break;

	case LOG_NONE:
	default:
		++m_logCount.none;
		break;
	}
}

template<class OIter, std::size_t BUFFERS>
const std::string BasicLogger<OIter, BUFFERS>::demangle(const char *name) {
#ifdef HAVE_GCC_ABI_DEMANGLE
	int status = -4;
	char *res = abi::__cxa_demangle(name, NULL, NULL, &status);
	const char *const demangled_name = (status == 0) ? res : name;
	std::string ret_val(demangled_name);
	free(res);
	return ret_val;
#else
	return name;
#endif
}

template<class OIter, std::size_t BUFFERS>
std::size_t BasicLogger<OIter, BUFFERS>::getLogCount(const LEVEL &level) {

	switch(level) {
	case LOG_INFO:
		return m_logCount.info;

	case LOG_WARNING:
		return m_logCount.warning;

	case LOG_ERROR:
		return m_logCount.error;

	case LOG_FATAL:
		return m_logCount.fatal;

	case LOG_DEBUG:
		return m_logCount.debug;

	case LOG_NONE:
	default:
		return m_logCount.none;
	}

	return 0;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &
BasicLogger<OIter, BUFFERS>::operator<<(const std::ios_base::fmtflags &f) {
	getMessageStream().flags(f);
	getMessageStream() << "0x";
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &
BasicLogger<OIter, BUFFERS>::operator<<(const BasicLogger<OIter, BUFFERS>::nonl &) {
	m_noNewline = true;
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &
BasicLogger<OIter, BUFFERS>::operator<<(const BasicLogger<OIter, BUFFERS>::width &w) {
	getMessageStream().width(w.m_width);
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &
BasicLogger<OIter, BUFFERS>::operator<<(const BasicLogger<OIter, BUFFERS>::time &t) {

	if(t.enabled) {

		char outstr[200] = "";
		time_t ti = t.time ? 0 : std::time(NULL);
		// cppcheck-suppress nonreentrantFunctionslocaltime
		const tm *tmp = t.time ? t.time : std::localtime(&ti);

		if(tmp && std::strftime(outstr, sizeof(outstr), t.fmt, tmp)) {
			getMessageStream() << outstr;
		}
	}

	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(const std::exception &e) {
	getMessageStream() << static_cast<const char *>(e.what());
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(const logString &msg) {
	getMessageStream() << msg;
	return *this;
}

#ifndef LOG_CHAR
template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(const std::string &msg) {
	getMessageStream() << logString(msg.begin(), msg.end());
	return *this;
}
#endif

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(const std::type_info &ti) {
	getMessageStream() << demangle(ti.name()).c_str();
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(const std::string *msg) {
	std::string s(msg ? (*msg) : std::string(LOGGER_NULL));
	getMessageStream() << logString(s.begin(), s.end());
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(const char *s) {
	std::string sc(s ? s : LOGGER_NULL);
	getMessageStream() << logString(sc.begin(), sc.end());
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(const wchar_t *s) {
	getMessageStream() << (s ? s : LLOGGER_NULL);
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(const abool &b) {
	getMessageStream() << std::boolalpha << static_cast<bool>(b.b);
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(float f) {

	float i;

	if(modff(f, &i) > 0) {
		getMessageStream().precision(4);
	}

	getMessageStream() << std::fixed << static_cast<float>(f);
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(short s) {
	getMessageStream() << static_cast<short>(s);
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(int i) {
	getMessageStream() << static_cast<int>(i);
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(long l) {
	getMessageStream() << static_cast<long>(l);
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(long long ll) {
	getMessageStream() << static_cast<long long>(ll);
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(unsigned short s) {
	getMessageStream() << static_cast<unsigned short>(s);
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(unsigned int i) {
	getMessageStream() << static_cast<unsigned int>(i);
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(unsigned long l) {
	getMessageStream() << static_cast<unsigned long>(l);
	return *this;
}

template<class OIter, std::size_t BUFFERS>
BasicLogger<OIter, BUFFERS> &BasicLogger<OIter, BUFFERS>::operator<<(unsigned long long ll) {
	getMessageStream() << static_cast<unsigned long long>(ll);
	return *this;
}

}

#endif // COMMONS_BASICLOGGER_H

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
