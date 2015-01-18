/**
 * basiclogger.h - template for basic logging functionality
 *
 * $Revision: 3544 $ $Author: heiko $
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

#ifndef NDEBUG
#define logDebug(msg)	{ (LOGGER_CLASS(LOGGER_CLASS::LOG_DEBUG))   << LOGGER_PREFIX << msg; }
#else
#define logDebug(msg)	{}
#endif
#define logger(msg)		{ (LOGGER_CLASS(LOGGER_CLASS::LOG_NONE))    << LOGGER_PREFIX << msg; }
#define logInfo(msg)	{ (LOGGER_CLASS(LOGGER_CLASS::LOG_INFO))    << LOGGER_PREFIX << msg; }
#define logWarning(msg)	{ (LOGGER_CLASS(LOGGER_CLASS::LOG_WARNING)) << LOGGER_PREFIX << msg; }
#define logError(msg)	{ (LOGGER_CLASS(LOGGER_CLASS::LOG_ERROR))   << LOGGER_PREFIX << msg; }
#define logFatal(msg)	{ (LOGGER_CLASS(LOGGER_CLASS::LOG_FATAL))   << LOGGER_PREFIX << msg; }

#ifndef BASICLOGGER_NO_PTHREADS
namespace {
pthread_mutex_t _basicLoggerLock = PTHREAD_MUTEX_INITIALIZER;
}
#endif

namespace Commons {

template<class> class IPostLogger;

template<class OIter>
class _EXPORT BasicLogger {
	DISALLOW_COPY_AND_ASSIGN(BasicLogger)
public:
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
		explicit _width(std::streamsize w) : m_width(w) {}
		std::streamsize m_width;
	} width;

	typedef struct _nonl {} nonl;

	typedef struct _PACKED _abool {
		_abool(bool ab) : b(ab) {}
		bool b;
	} abool;

	typedef struct _time {
		_time(const char *f = "%a, %d %b %Y %T %z", const tm *t = NULL) : fmt(f), time(t) {}
		const char *fmt;
		const tm *time;
	} time;

	static const std::ios_base::fmtflags hex;
	static const std::ios_base::fmtflags dec;

	static void setSilentMask(unsigned char mask = 0x00) {
		m_silentMask = mask;
	}

	static unsigned char getSilentMask() {
		return m_silentMask;
	}

	static size_t getLogCount(const LEVEL &level) _PURE;

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
	BasicLogger(const OIter &out, const LEVEL &level, const IPostLogger<OIter> *post = 0L);

	_INTERNAL LEVEL getLevel() const {
		return m_level;
	}

	_INTERNAL std::basic_ostringstream<typename OIter::char_type> &getMessageStream() {
		return m_msg;
	}

private:
	_INTERNAL void initMessageString();
	_INTERNAL const char *getLevelString(LEVEL lvl) const;
	_INTERNAL static const std::string demangle(const char *name);

private:
	std::basic_ostringstream<typename OIter::char_type> m_msg;

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

	static unsigned char m_silentMask;

	LEVEL m_level;

	bool m_noNewline;
	bool m_silent;

	const IPostLogger<OIter> *m_postLogger;
};

template<class OIter>
class IPostLogger {
	DISALLOW_COPY_AND_ASSIGN(IPostLogger)
public:
	inline virtual ~IPostLogger() {}

	virtual void postAction(const typename BasicLogger<OIter>::logString &ls) const throw() = 0;

protected:
	inline IPostLogger() {}
};

template<class OIter>
unsigned char BasicLogger<OIter>::m_silentMask = 0x00;

template<class OIter>
struct BasicLogger<OIter>::_logcount BasicLogger<OIter>::m_logCount = { 0, 0, 0, 0, 0, 0 };

template<class OIter>
const std::ios_base::fmtflags BasicLogger<OIter>::hex = std::ios_base::hex;

template<class OIter>
const std::ios_base::fmtflags BasicLogger<OIter>::dec = std::ios_base::dec;

template<class OIter>
BasicLogger<OIter>::BasicLogger(const OIter &out, const BasicLogger<OIter>::LEVEL &level,
								const IPostLogger<OIter> *post) : m_msg(), m_out(out),
	m_level(level), m_noNewline(false), m_silent(false),
	m_postLogger(post) {

#ifndef BASICLOGGER_NO_PTHREADS
	pthread_mutex_lock(&_basicLoggerLock);
#endif

	m_silent = !((m_silentMask & static_cast<unsigned char>(level)) == 0);

	std::new_handler hdl = std::set_new_handler(0L);

	m_msg.precision(0);

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

template<class OIter>
BasicLogger<OIter>::~BasicLogger() {

#ifndef BASICLOGGER_NO_PTHREADS
	pthread_mutex_lock(&_basicLoggerLock);
#endif

	if(!m_noNewline) m_msg << std::endl;

	logString s = m_msg.str();

	if(!m_silent) std::copy(s.begin(), s.end(), m_out);

	if(m_postLogger) m_postLogger->postAction(s);

#ifndef BASICLOGGER_NO_PTHREADS
	pthread_mutex_unlock(&_basicLoggerLock);
#endif
}

template<class OIter>
const char *BasicLogger<OIter>::getLevelString(LEVEL lvl) const {
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

template<class OIter>
void BasicLogger<OIter>::initMessageString() {

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

template<class OIter>
const std::string BasicLogger<OIter>::demangle(const char *name) {
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

template<class OIter>
size_t BasicLogger<OIter>::getLogCount(const LEVEL &level) {

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

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(const std::ios_base::fmtflags &f) {
	getMessageStream().flags(f);
	getMessageStream() << "0x";
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(const BasicLogger<OIter>::nonl &) {
	m_noNewline = true;
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(const BasicLogger<OIter>::width &w) {
	getMessageStream().width(w.m_width);
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(const BasicLogger<OIter>::time &t) {

	char outstr[200] = "";
	time_t ti = t.time ? 0 : std::time(NULL);
	// cppcheck-suppress nonreentrantFunctionslocaltime
	const tm *tmp = t.time ? t.time : std::localtime(&ti);

	if(tmp && std::strftime(outstr, sizeof(outstr), t.fmt, tmp)) {
		getMessageStream() << outstr;
	}

	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(const std::exception &e) {
	getMessageStream() << static_cast<const char *>(e.what());
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(const logString &msg) {
	getMessageStream() << msg;
	return *this;
}

#ifndef LOG_CHAR
template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(const std::string &msg) {
	getMessageStream() << logString(msg.begin(), msg.end());
	return *this;
}
#endif

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(const std::type_info &ti) {
	getMessageStream() << demangle(ti.name()).c_str();
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(const std::string *msg) {
	std::string s(msg ? (*msg) : std::string(LOGGER_NULL));
	getMessageStream() << logString(s.begin(), s.end());
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(const char *s) {
	std::string sc(s ? s : LOGGER_NULL);
	getMessageStream() << logString(sc.begin(), sc.end());
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(const wchar_t *s) {
	getMessageStream() << (s ? s : LLOGGER_NULL);
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(const abool &b) {
	getMessageStream() << std::boolalpha << static_cast<bool>(b.b);
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(float f) {

	float i;

	if(modff(f, &i) > 0) {
		getMessageStream().precision(4);
	}

	getMessageStream() << std::fixed << static_cast<float>(f);
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(short s) {
	getMessageStream() << static_cast<short>(s);
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(int i) {
	getMessageStream() << static_cast<int>(i);
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(long l) {
	getMessageStream() << static_cast<long>(l);
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(long long ll) {
	getMessageStream() << static_cast<long long>(ll);
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(unsigned short s) {
	getMessageStream() << static_cast<unsigned short>(s);
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(unsigned int i) {
	getMessageStream() << static_cast<unsigned int>(i);
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(unsigned long l) {
	getMessageStream() << static_cast<unsigned long>(l);
	return *this;
}

template<class OIter>
BasicLogger<OIter> &BasicLogger<OIter>::operator<<(unsigned long long ll) {
	getMessageStream() << static_cast<unsigned long long>(ll);
	return *this;
}

}

#endif // COMMONS_BASICLOGGER_H

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4; 
