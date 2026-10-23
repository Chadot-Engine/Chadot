/**************************************************************************/
/*  register_types.cpp                                                    */
/**************************************************************************/
/* License : Projekt Jenova Public Proprietary License (PJPPL v1.0)       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             CHADOT ENGINE                              */
/*                    https://chadotengine.github.io                      */
/**************************************************************************/
/* Copyright (c) 2024-present MemarDesign LLC. (Hamid.Memar)              */
/**************************************************************************/

#include "register_types.h"
#include "core/object/class_db.h"

// Module Routines
void initialize_hm_materials_module(ModuleInitializationLevel p_level) {
	// Check for Initialization Level
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;

	// Register Materials
	ClassDB::register_class<SixWayVolumeMaterial>();
	ClassDB::register_class<SixWayFoliageMaterial>();
}
void uninitialize_hm_materials_module(ModuleInitializationLevel p_level) {
	// Check for Initialization Level
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
		return;

	// Release Materials
	ReleaseSixWayShaders();
}
