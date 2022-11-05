#pragma once

#include "MfxHost.h"

/**
 * Provide basic access to host's suites, use for any class but for MfxEffect
 * (which must own the MfxHost, not just a reference it).
 */
class MfxBase {
protected:
	/**
	 * At construction the object need to know the host, which is ultimately owned
	 * by the current \ref MfxEffect.
	 */
	MfxBase(const MfxHost& host) : m_host(host) {}

	/**
	 * Subclasses usually use the host() implicitely when using the \ref MFX_CHECK
	 * and \ref MFX_ENSURE macros.
	 */
	const MfxHost& host() const { return m_host; }

private:
	const MfxHost& m_host;
};

