/**************************************************************************/
/*  register_types.cpp                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             CHADOT ENGINE                              */
/*                    https://chadotengine.github.io                      */
/**************************************************************************/
/* Copyright (c) 2024-present MemarDesign LLC. (Hamid.Memar)              */
/* Copyright (c) 2025 Godot Lottie Contributors.                          */
/**************************************************************************/

#include "register_types.h"

#include "lottie_animation.h"
#include "lottie_frame_cache.h"
#include "lottie_state_machine.h"

#include "core/object/class_db.h"

#include <thorvg.h>

#ifdef THREADS_ENABLED
#define TVG_THREADS 1
#else
#define TVG_THREADS 0
#endif

static bool lottie_initialized = false;

void initialize_lottie_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	if (tvg::Initializer::init(TVG_THREADS) != tvg::Result::Success) {
		return;
	}

	lottie_initialized = true;

	GDREGISTER_CLASS(LottieAnimation);
	GDREGISTER_CLASS(LottieAnimationState);
	GDREGISTER_CLASS(LottieStateTransition);
	GDREGISTER_CLASS(LottieStateMachine);
}

void uninitialize_lottie_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	if (!lottie_initialized) {
		return;
	}

	LottieFrameCache::clear_singleton();
	tvg::Initializer::term();
	lottie_initialized = false;
}
