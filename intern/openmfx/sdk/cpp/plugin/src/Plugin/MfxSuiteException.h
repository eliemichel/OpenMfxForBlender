#pragma once

#include "ofxCore.h"

#include <exception>
#include <string>

/**
 * Exception thrown by the \ref MFX_ENSURE macro.
 */
class MfxSuiteException : public std::exception
{
public:
	MfxSuiteException(OfxStatus status, const char *call);

	const char* what() const throw ();

	OfxStatus GetStatus() const { return m_status; }

private:
	OfxStatus m_status;
	const char *m_call;
	mutable std::string m_what; // cache result
};
