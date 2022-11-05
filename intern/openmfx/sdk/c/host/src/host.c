#include "host.h"
#include "types.h"
#include "meshEffectSuite.h"
#include "propertySuite.h"
#include "parameterSuite.h"

#include <stdio.h>
#include <string.h>

const void* fetchSuite(OfxPropertySetHandle host, const char *suiteName, int suiteVersion) {
  printf("[host] fetchSuite(host, %s, %d)\n", suiteName, suiteVersion);
  if (0 == strcmp(suiteName, kOfxMeshEffectSuite)) {
    switch (suiteVersion) {
      case 1:
        return (void*)&meshEffectSuiteV1;
    }
  }
  if (0 == strcmp(suiteName, kOfxPropertySuite)) {
    switch (suiteVersion) {
      case 1:
        return (void*)&propertySuiteV1;
    }
  }
  if (0 == strcmp(suiteName, kOfxParameterSuite)) {
    switch (suiteVersion) {
      case 1:
        return (void*)&parameterSuiteV1;
    }
  }
  return NULL;
}
