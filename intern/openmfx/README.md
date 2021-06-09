OpenMfx
=======

https://openmesheffect.org/

OpenMfx is a plugin API for interoperable non-destructive mesh modeling effects. Here in Blender, it is used as a way to dynamically load custom C/C++ modifiers.

Folder layout
-------------

```
openmfx
 - openfx: Equivalent of include/ in openmfx repo, contains the official API headers
 - host: Copy of examples/host/ from openmfx repo, shared among several hosts, not depending from Blender (Apache 2)
 - plugins: Copy of examples/plugins/, just for the sake of example
 - util: Copy of examples/util/, Utility functions shared by plugins and used a bit by the host, but that eventually will become obsolete
 - test: tests, we should have more of them
 - blender: The part of the host that is specific to Blender (GPL)
```
