## include/

This directory contains the Mesh Effect API itself in `ofxMeshEffect.h`, as well as the subset of OpenFX required for it to work. Note that it does not requires `ofxImageEffect` for instance.

To implement the API, add this directory to your project's include directories, or if you use CMake simply link to the target `OpenMfx`.
