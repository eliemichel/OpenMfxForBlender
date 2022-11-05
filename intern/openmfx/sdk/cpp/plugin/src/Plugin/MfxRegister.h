#pragma once

#include "MfxEffect.h"

#include <array>
#include <string>
#include <tuple>

/**
 * Registration must be performed at the very end of the main file. Example:
 * ```{.cpp}
 * MfxRegister(
 *     MyFirstEffect,
 *     AnotherEffect,
 *     AThirdEffect
 * );
 * ```
 * There can be an arbitrary number of arguments, and all of them are expected
 * to be subclasses of \ref MfxEffect.
 * 
 * **Dev note:** It is a bit unfortunate for readability that all this has to
 * be a macro, but it is important to have `gEffects` defined before `setHost<>`
 * and `mainEntry<>` so I could not figure out any other way.
 */
#define MfxRegister(...) std::tuple<__VA_ARGS__> gEffects; \
constexpr size_t Count = std::tuple_size<std::tuple<__VA_ARGS__>>::value;\
 \
/*-----------------------------------------------------------------------------*/ \
/* Automatic generation of setHost* and mainEntry* function pointers */ \
 \
template<size_t X> \
void setHost(OfxHost* host) { \
    std::get<X>(gEffects).SetHost(host); \
} \
 \
template<size_t X> \
OfxStatus mainEntry(const char* action, const void* handle, OfxPropertySetHandle inArgs, OfxPropertySetHandle outArgs) { \
    return std::get<X>(gEffects).MainEntry(action, handle, inArgs, outArgs); \
} \
 \
template <size_t X> \
constexpr OfxPlugin make_plugin() { \
    return { \
    /* pluginApi */          kOfxMeshEffectPluginApi, \
    /* apiVersion */         kOfxMeshEffectPluginApiVersion, \
    /* pluginIdentifier */   std::get<X>(gEffects).GetName(), \
    /* pluginVersionMajor */ 1, \
    /* pluginVersionMinor */ 0, \
    /* setHost */            setHost<X>, \
    /* mainEntry */          mainEntry<X> \
    }; \
} \
 \
/* Static foreach iterating over COUNT plugins and calling setupPlugin for each */ \
 \
template <size_t ... Is> \
constexpr auto make_plugins_array(std::index_sequence<Is...>) { \
    return std::array<OfxPlugin,Count> { make_plugin<Is>()... }; \
} \
 \
auto gPlugins = make_plugins_array(std::make_index_sequence<Count>{}); \
 \
/*----------------------------------------------------------------------------- */ \
/* Exported standard symbols */ \
 \
OfxExport int OfxGetNumberOfPlugins(void) { \
    return static_cast<int>(gPlugins.size()); \
} \
 \
OfxExport OfxPlugin* OfxGetPlugin(int nth) { \
    return &gPlugins[nth]; \
}

