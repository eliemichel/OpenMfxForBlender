/*
 * Copyright 2019-2021 Elie Michel
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

 /** \file
  * \ingroup openmesheffect
  */

#pragma once

#include "ofxCore.h"
#include "ofxMeshEffect.h"
#include "ofxProperty.h"
#include "ofxParam.h"
#include "ofxMessage.h"

namespace OpenMfx {

/**
 * C++ extension of core OfxHost
 * To use this in you own program, it is advised to subclass it and implement BeforeMeshGet() and BeforeMeshRelease()
 */
class MfxHost {
public:
	/**
	 * Retrieve the MfxHost pointer stored in ofxHost's properties.
	 */
	static MfxHost* FromOfxHost(OfxHost* ofxHost);

public:
	MfxHost();
	~MfxHost();

  // Disable copy (rule of three)
	MfxHost(const MfxHost&) = delete;
	MfxHost& operator=(const MfxHost&) = delete;

public: // Utility wrappers around plugin->mainEntry

	// Load the plugin before using it, and unload it afterwards
	bool LoadPlugin(OfxPlugin* plugin);
	void UnloadPlugin(OfxPlugin* plugin);

	// The descriptor may be retrieved for different contexts of application
	bool GetDescriptor(OfxPlugin* plugin, OfxMeshEffectHandle & effectDescriptor);
	void ReleaseDescriptor(OfxMeshEffectHandle effectDescriptor);

	// An instance of an effect is basically a node of the scene's DAG/depsgraph
	// It is created from a descriptor
	bool CreateInstance(OfxMeshEffectHandle effectDescriptor, OfxMeshEffectHandle& effectInstance);
	void DestroyInstance(OfxMeshEffectHandle effectInstance);

	// Cooking and querying whether the instance has an effect produces different
	// results depending on the value that has been assigned to the effect's
	// parameters for this particular instance.
	// If isIdentity is turned true, there is no need to cook the effect, its
	// input inputToPassThrough must simply be considered as the output.
	bool IsIdentity(OfxMeshEffectHandle effectInstance, bool* isIdentity, char** inputToPassThrough);
	bool Cook(OfxMeshEffectHandle effectInstance);

protected:
	// Callback responsible for converting from and back to host's internal
	// representation. They are callbacks so that they are ran only if and when
	// the plugin needs so. To be overriden in subclasses.
	virtual OfxStatus BeforeMeshGet(OfxMeshHandle ofxMesh) {
		return kOfxStatReplyDefault;
	}
	virtual OfxStatus BeforeMeshRelease(OfxMeshHandle ofxMesh) {
		return kOfxStatReplyDefault;
	}
  virtual OfxStatus BeforeMeshAllocate(OfxMeshHandle ofxMesh) {
    return kOfxStatReplyDefault;
  }

	// Initialize inputs after creating an instance (mostly fills kOfxMeshPropInternalData)
	virtual void InitInput(OfxMeshInputStruct& input) {}

	// Give access to the raw OfxHost
	inline OfxHost* RawHost() { return &m_host; }

private:
	// Actual callback must be static, these versions only route to non static
	// methods whose name is the same.
	static OfxStatus BeforeMeshGetCb(OfxHost *ofxHost, OfxMeshHandle ofxMesh);
	static OfxStatus BeforeMeshReleaseCb(OfxHost* ofxHost, OfxMeshHandle ofxMesh);
  static OfxStatus BeforeMeshAllocateCb(OfxHost *ofxHost, OfxMeshHandle ofxMesh);

	static const void * FetchSuite(OfxPropertySetHandle host, const char* suiteName, int suiteVersion);

public:
	const OfxPropertySuiteV1* propertySuite;
	const OfxParameterSuiteV1* parameterSuite;
	const OfxMessageSuiteV2* messageSuite;
	const OfxMeshEffectSuiteV1* meshEffectSuite;

private:
	OfxHost m_host;
};

}  // namespace OpenMfx
