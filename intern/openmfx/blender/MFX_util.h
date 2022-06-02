/**
 * Utility functions using Blender functions
 */

#ifndef __MFX_UTILS_H__
#define __MFX_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "DNA_node_types.h"

#include "mfxPluginRegistry.h"

/**
 * Get absolute path (ui file browser returns relative path for saved files)
 */
void MFX_normalize_plugin_path(char *dest_path, const char *src_path);

const PluginRegistry *MFX_get_plugin_registry(bNode *node);

#ifdef __cplusplus
}
#endif

#endif // __MFX_UTILS_H__
