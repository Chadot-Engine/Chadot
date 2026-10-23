/**************************************************************************/
/*  register_types.h                                                      */
/**************************************************************************/
/* License : Projekt Jenova Public Proprietary License (PJPPL v1.0)       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             CHADOT ENGINE                              */
/*                    https://chadotengine.github.io                      */
/**************************************************************************/
/* Copyright (c) 2024-present MemarDesign LLC. (Hamid.Memar)              */
/**************************************************************************/

#ifndef HM_MATERIALS_REGISTER_TYPES_H
#define HM_MATERIALS_REGISTER_TYPES_H

#include "modules/register_module_types.h"
#include "sixway_mtl.h"

void initialize_hm_materials_module(ModuleInitializationLevel p_level);
void uninitialize_hm_materials_module(ModuleInitializationLevel p_level);

#endif // HM_MATERIALS_REGISTER_TYPES_H
