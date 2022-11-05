CppPluginSupport
================

*CppPluginSupport* is a small set of C++ classes reducing boilerplate when writing OpenMfx plugins. Its used is not mandatory but there is only little reason not to use it.

## Usage

**Configuration**

**Programmer Interface**

All classes defined in this library start with *Mfx*.

```C++
#include <PluginSupport/MfxEffect>

class MyEffect : public MfxEffect {
protected:
	OfxStatus Describe(OfxMeshEffectHandle descriptor) override {
		AddInput(kOfxMeshMainInput);
		AddInput(kOfxMeshMainOutput);

		AddParam("axis", 1)
		.Label("Axis")
		.Range(0, 2);

		AddParam("translation", { 0.0, 0.0 })
		.Label("Translation");

		return kOfxStatOK;
	}
};

class MyPlugin : public MfxPlugin {
protected:

};
```
