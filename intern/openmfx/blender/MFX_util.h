/**
 * Utility functions using Blender functions
 */

#pragma once

/**
 * Get absolute path (ui file browser returns relative path for saved files)
 */
void MFX_normalize_plugin_path(char* dest_path, const char* src_path);
