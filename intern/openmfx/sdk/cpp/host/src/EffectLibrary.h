/*
 * Copyright 2019-2022 Elie Michel
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "util/binary_util.h"
#include "ofxExtras.h"

#include <OpenMfx/Sdk/Cpp/Common>

#include <ofxCore.h>

#include <vector>

typedef void (*OfxSetBundleDirectoryFunc)(const char* path);
typedef int (*OfxGetNumberOfPluginsFunc)(void);
typedef OfxPlugin* (*OfxGetPluginFunc)(int nth);

namespace OpenMfx {

/**
 * The plug-in library holds all the data about the plug-ins made available by
 * a given ofx plug-in binary. A binary might contain many plug-ins.
 */
class EffectLibrary {
public:
    EffectLibrary() {}
    MOVE_ONLY(EffectLibrary)

    /**
     * Number of loaded plugins
     */
    int effectCount() const;

    /**
     * Assumes that the index is valid
     */
    const char *effectIdentifier(int effectIndex) const;

    /**
     * Assumes that the index is valid
     */
    unsigned int effectVersionMajor(int effectIndex) const;

    /**
     * Assumes that the index is valid
     */
    unsigned int effectVersionMinor(int effectIndex) const;

private: // reserved to EffectRegistry
    friend class EffectRegistryEntry;

    enum class Status {
        OK,
        NotLoaded,
        Error
    };

    /**
     * /pre registry has never been allocated
     * /post if true is returned, registry is allocated and filled with valid
     *       OfxPlugin pointers. Registry must be later released using
     *       free_registry()
     */
    bool load(const char *ofx_filepath);

    /**
     * /pre registry has been allocated
     * /post registry will never be used again
     */
    void unload();

    /**
     * Assumes that the index is valid
     */
    Status pluginStatus(int effectIndex) const;

    /**
     * Assumes that the index is valid
     */
    OfxPlugin* plugin(int effectIndex) const;

private:
    /**
     * Initialize in a plugin registry the attributes related to binary loading.
     */
    bool initBinary(const char* ofx_filepath);

    /**
     * Initialize a plugin registry provided that the procedure have been loaded
     * correctly.
     */
    void initPlugins();

private:
    /**
     * handle of the binary library, to be closed
     */
    BinaryHandle m_handle = nullptr;

    /**
     * Procedures loaded from the binary library
     */
    struct {
        OfxGetNumberOfPluginsFunc getNumberOfPlugins = nullptr;
        OfxGetPluginFunc getPlugin = nullptr;
        OfxSetBundleDirectoryFunc setBundleDirectory = nullptr;
    } m_procedures;

    /**
     * Plugins compatible with the Mesh Effect API
     */
    std::vector<std::pair<OfxPlugin*, Status>> m_plugins;
};

} // namespace OpenMfx
