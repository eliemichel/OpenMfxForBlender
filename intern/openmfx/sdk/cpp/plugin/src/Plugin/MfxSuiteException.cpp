#include "MfxSuiteException.h"

#include <sstream>

static const char* getOfxStateName(OfxStatus status) {
    static const char* names[] = {
      "kOfxStatOK",
      "kOfxStatFailed",
      "kOfxStatErrFatal",
      "kOfxStatErrUnknown",
      "kOfxStatErrMissingHostFeature",
      "kOfxStatErrUnsupported",
      "kOfxStatErrExists",
      "kOfxStatErrFormat",
      "kOfxStatErrMemory",
      "kOfxStatErrBadHandle",
      "kOfxStatErrBadIndex",
      "kOfxStatErrValue",
      "kOfxStatReplyYes",
      "kOfxStatReplyNo",
      "kOfxStatReplyDefault",
      "Invalid status!"
    };
    if (status < 0 || status > 15) {
        return names[15];
    }
    return names[(size_t)status];
}

MfxSuiteException::MfxSuiteException(OfxStatus status, const char *call)
	: m_status(status)
	, m_call(call)
{}

const char* MfxSuiteException::what() const throw ()
{
	std::ostringstream ss;
	ss
		<< "MfxSuiteException: Suite method call '" << m_call
		<< "' returned status " << m_status
		<< " (" << getOfxStateName(m_status) << ")";
	m_what = ss.str();
	return m_what.c_str();
}
