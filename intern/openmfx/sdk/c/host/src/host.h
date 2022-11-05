#ifndef _host_h_
#define _host_h_

/*****************************************************************************/
/* Master Host */

#include <ofxCore.h>

const void* fetchSuite(OfxPropertySetHandle host, const char *suiteName, int suiteVersion);

#endif // _host_h_