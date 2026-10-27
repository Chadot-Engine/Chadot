# Chadot Engine : Godot for Chads

<img width="1840" height="1000" alt="Chadot Screenshot" src="https://github.com/user-attachments/assets/3258d011-fc4b-436f-8343-d71655c75fc8" />

## What's Chadot?

Chadot is a fork of the Godot Engine focused on adding critical missing features and giving developers more control over how the engine behaves.
It provides a fully customizable rendering pipeline with support for systems such as PHLMS, ReLight, JenovaFX etc. allowing developers to build and tailor their rendering workflow to their needs.

> **Note:** The project is primarily focused on **Microsoft Windows**.   
> While most features are expected to work on Linux, Linux support is not a primary development target.

## Current Features
- Compatible with [Jenova Framework](https://github.com/Jenova-Framework)
- Per-Light Hybrid Light Mapping and Shadow-Masking (PHLMS)
- MeshInstanceEx3D (aka RayCompute3D) Integration
- GameRecorder3D Integration for Cinematic/Replay
- Full GBuffer Access in Post-Processing Shaders
- Custom Lightmap Support with ReLight Integration
- Native Lottie Vector Animation Support
- Per-Viewport Global Material Override (Environment)
- Six-Way Lighting Volumetric/Foliage Materials Integration
- Voxel-Friendly Triangle-Based Lightmap Unwrapper/Packer
- Custom Panorama Reflections for Reflection Probes

### Work in Progress
- New API and SDK for engine internals
- DLSS 5 upscaling and neural rendering integration
- ReLight lightmapper integration
  
## License

**Chadot uses a mixed licensing model.**

The original Godot Engine codebase remains licensed under the terms of the MIT License. Any code inherited directly from Godot continues to follow its original license.

Additional features, modifications and components introduced by Chadot may be released under different licenses. Unless explicitly stated otherwise, changes and additions are considered to be licensed under MIT.

Some features may be licensed under the **PJPPL v1.0** or other licenses specified by the author. When a feature is added, license information is included in commit messages, source files or accompanying documentation. If no alternative license is explicitly specified, the code is considered to be licensed under the MIT License by default.

### Projekt Jenova Public Proprietary License (PJPPL v1.0)
Code licensed under PJPPL v1.0 is publicly visible for transparency and integration with the Chadot source tree. However, visibility of the source code does not grant permission to copy, redistribute, modify, reuse or incorporate that code into other projects unless explicitly permitted by its license.

Features distributed under PJPPL v1.0 may only be used through official binaries provided by the author. Any use of the licensed code outside of those official distributions is prohibited unless prior written permission has been granted.

Engine binaries can be verified by their SHA-256 hash and checked against [the official registry](https://chadot-engine.github.io/authorized-builds/). **As long as the engine binaries remain unmodified and match the hashes provided by the registry, they comply with the license and are allowed to be used and redistributed.**

The terms of PJPPL v1.0 and other Chadot specific licenses may be revised in future versions.

## AI Policy
Chadot is developed by a real developer who understands the engine, its architecture and the problems it aims to solve.

While AI tools can be useful for research, prototyping and exploring ideas, all development and design decisions are made by a human. I do not rely on AI generated code for the project's implementation, as I believe complex engine development still requires a level of understanding, judgment, and long term maintenance that current AI tools cannot reliably provide.

All code in Chadot is written and reviewed manually. **Pull requests and Issues containing AI generated code/content will not be accepted.**
