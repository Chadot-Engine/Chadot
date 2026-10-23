/**************************************************************************/
/*  sixway_mtl.h                                                          */
/**************************************************************************/
/* License : Projekt Jenova Public Proprietary License (PJPPL v1.0)       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             CHADOT ENGINE                              */
/*                    https://chadotengine.github.io                      */
/**************************************************************************/
/* Copyright (c) 2024-present MemarDesign LLC. (Hamid.Memar)              */
/**************************************************************************/

#ifndef SIXWAY_MTL_H
#define SIXWAY_MTL_H

// Imports
#include "scene/resources/material.h"

// Six-Way Lighting Volume Material Definition
class SixWayVolumeMaterial : public ShaderMaterial {
	GDCLASS(SixWayVolumeMaterial, ShaderMaterial);

public:
	SixWayVolumeMaterial();

protected:
	static void _bind_methods() {}
	bool _get_flat_inspector_mode() const override { return true; }
	void _validate_property(PropertyInfo &p_property) const;
	void set_shader(const Ref<Shader> &p_shader);
};

// Six-Way Lighting Foliage Material Definition
class SixWayFoliageMaterial : public ShaderMaterial {
	GDCLASS(SixWayFoliageMaterial, ShaderMaterial);

public:
	SixWayFoliageMaterial();

protected:
	static void _bind_methods() {}
	bool _get_flat_inspector_mode() const override { return true; }
	void _validate_property(PropertyInfo &p_property) const;
	void set_shader(const Ref<Shader> &p_shader);
};

// Shared Methods
void ReleaseSixWayShaders();

#endif // SIXWAY_MTL_H
