#include "MFX_util.h"

#include "BKE_main.h" // BKE_main_blendfile_path_from_global

#include "DNA_node_types.h"

#include "BLI_path_util.h"
#include "BLI_string.h"

#include "MFX_node_runtime.h"

void MFX_normalize_plugin_path(char* dest_path, const char* src_path)
{
    BLI_strncpy(dest_path, src_path, FILE_MAX);
    const char* base_path =
        BKE_main_blendfile_path_from_global();  // TODO: How to get a bMain object here to avoid
                                                // "from_global()"?
    if (NULL != base_path) {
        BLI_path_abs(dest_path, base_path);
    }
}

const PluginRegistry *MFX_get_plugin_registry(bNode *node)
{
  NodeGeometryOpenMfxRuntimeHandle *runtime = ((NodeGeometryOpenMfx *)(node->storage))->runtime;
  return &runtime->registry();
}
