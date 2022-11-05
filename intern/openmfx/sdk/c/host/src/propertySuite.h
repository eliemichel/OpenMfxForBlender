#ifndef _propertySuite_h_
#define _propertySuite_h_

/*****************************************************************************/
/* Property Suite */

#include <ofxCore.h>
#include <ofxProperty.h>

OfxStatus propSetPointer(OfxPropertySetHandle properties,
                         const char *property,
                         int index,
                         void *value);

OfxStatus propGetPointer(OfxPropertySetHandle properties,
                         const char *property,
                         int index,
                         void **value);

OfxStatus propSetString(OfxPropertySetHandle properties,
                        const char *property,
                        int index,
                        const char *value);

OfxStatus propGetString(OfxPropertySetHandle properties,
                        const char *property,
                        int index,
                        char **value);

OfxStatus propSetInt(OfxPropertySetHandle properties,
                     const char *property,
                     int index,
                     int value);

OfxStatus propGetInt(OfxPropertySetHandle properties,
                     const char *property,
                     int index,
                     int *value);

extern const OfxPropertySuiteV1 propertySuiteV1;

#endif // _propertySuite_h_
