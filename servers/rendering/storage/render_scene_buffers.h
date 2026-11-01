/**************************************************************************/
/*  render_scene_buffers.h                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/object/ref_counted.h"
#include "servers/rendering/rendering_server_enums.h"

class RenderSceneBuffersConfiguration : public RefCounted {
	GDCLASS(RenderSceneBuffersConfiguration, RefCounted);

private:
	RID render_target;

	Size2i internal_size;
	Size2i target_size;
	uint32_t view_count = 1;

	RSE::ViewportScaling3DMode scaling_3d_mode = RSE::VIEWPORT_SCALING_3D_MODE_OFF;
	RSE::ViewportMSAA msaa_3d = RSE::VIEWPORT_MSAA_DISABLED;
	RSE::ViewportScreenSpaceAA screen_space_aa = RSE::VIEWPORT_SCREEN_SPACE_AA_DISABLED;
	RSE::ViewportAnisotropicFiltering anisotropic_filtering_level = RSE::VIEWPORT_ANISOTROPY_4X;

	float fsr_sharpness = 0.0;
#ifdef DLSS_FEATURE_ENABLED
	int dlss_quality = 1;
	int dlss_preset = 0;
	float dlss_sharpness = 0.0f;
	bool dlss_hdr = true;
	bool dlss_auto_exposure = true;
	bool dlss_alpha_upscaling = false;
	bool dlss_depth_inverted = true;
	bool dlss_mv_low_res = true;
#endif
	float texture_mipmap_bias = 0.0;
	bool use_taa = false;
	bool use_debanding = false;

protected:
	static void _bind_methods();

public:
	RID get_render_target() const { return render_target; }
	void set_render_target(RID p_render_target) { render_target = p_render_target; }

	Size2i get_internal_size() const { return internal_size; }
	void set_internal_size(Size2i p_internal_size) { internal_size = p_internal_size; }

	Size2i get_target_size() const { return target_size; }
	void set_target_size(Size2i p_target_size) { target_size = p_target_size; }

	uint32_t get_view_count() const { return view_count; }
	void set_view_count(uint32_t p_view_count) { view_count = p_view_count; }

	RSE::ViewportScaling3DMode get_scaling_3d_mode() const { return scaling_3d_mode; }
	void set_scaling_3d_mode(RSE::ViewportScaling3DMode p_scaling_3d_mode) { scaling_3d_mode = p_scaling_3d_mode; }

	RSE::ViewportMSAA get_msaa_3d() const { return msaa_3d; }
	void set_msaa_3d(RSE::ViewportMSAA p_msaa_3d) { msaa_3d = p_msaa_3d; }

	RSE::ViewportScreenSpaceAA get_screen_space_aa() const { return screen_space_aa; }
	void set_screen_space_aa(RSE::ViewportScreenSpaceAA p_screen_space_aa) { screen_space_aa = p_screen_space_aa; }

	float get_fsr_sharpness() const { return fsr_sharpness; }
	void set_fsr_sharpness(float p_fsr_sharpness) { fsr_sharpness = p_fsr_sharpness; }

#ifdef DLSS_FEATURE_ENABLED
	int get_dlss_quality() const { return dlss_quality; }
	void set_dlss_quality(int p_quality) { dlss_quality = p_quality; }

	int get_dlss_preset() const { return dlss_preset; }
	void set_dlss_preset(int p_preset) { dlss_preset = p_preset; }

	float get_dlss_sharpness() const { return dlss_sharpness; }
	void set_dlss_sharpness(float p_sharpness) { dlss_sharpness = p_sharpness; }

	bool get_dlss_hdr() const { return dlss_hdr; }
	void set_dlss_hdr(bool p_enabled) { dlss_hdr = p_enabled; }

	bool get_dlss_auto_exposure() const { return dlss_auto_exposure; }
	void set_dlss_auto_exposure(bool p_enabled) { dlss_auto_exposure = p_enabled; }

	bool get_dlss_alpha_upscaling() const { return dlss_alpha_upscaling; }
	void set_dlss_alpha_upscaling(bool p_enabled) { dlss_alpha_upscaling = p_enabled; }

	bool get_dlss_depth_inverted() const { return dlss_depth_inverted; }
	void set_dlss_depth_inverted(bool p_enabled) { dlss_depth_inverted = p_enabled; }

	bool get_dlss_mv_low_res() const { return dlss_mv_low_res; }
	void set_dlss_mv_low_res(bool p_enabled) { dlss_mv_low_res = p_enabled; }
#endif

	float get_texture_mipmap_bias() const { return texture_mipmap_bias; }
	void set_texture_mipmap_bias(float p_texture_mipmap_bias) { texture_mipmap_bias = p_texture_mipmap_bias; }

	RSE::ViewportAnisotropicFiltering get_anisotropic_filtering_level() const { return anisotropic_filtering_level; }
	void set_anisotropic_filtering_level(RSE::ViewportAnisotropicFiltering p_anisotropic_filtering_level) { anisotropic_filtering_level = p_anisotropic_filtering_level; }

	bool get_use_taa() const { return use_taa; }
	void set_use_taa(bool p_use_taa) { use_taa = p_use_taa; }

	bool get_use_debanding() const { return use_debanding; }
	void set_use_debanding(bool p_use_debanding) { use_debanding = p_use_debanding; }

	RenderSceneBuffersConfiguration() {}
	virtual ~RenderSceneBuffersConfiguration() {}
};

class RenderSceneBuffers : public RefCounted {
	GDCLASS(RenderSceneBuffers, RefCounted);

protected:
	static void _bind_methods();

public:
	RenderSceneBuffers() {}
	virtual ~RenderSceneBuffers() {}

	virtual void configure(const RenderSceneBuffersConfiguration *p_config) = 0;

	// for those settings that are unlikely to require buffers to be recreated, we'll add setters
	virtual void set_fsr_sharpness(float p_fsr_sharpness) = 0;
#ifdef DLSS_FEATURE_ENABLED
	virtual void set_dlss_quality(int p_quality) = 0;
	virtual void set_dlss_preset(int p_preset) = 0;
	virtual void set_dlss_sharpness(float p_sharpness) = 0;
	virtual void set_dlss_hdr(bool p_enabled) = 0;
	virtual void set_dlss_auto_exposure(bool p_enabled) = 0;
	virtual void set_dlss_alpha_upscaling(bool p_enabled) = 0;
	virtual void set_dlss_depth_inverted(bool p_enabled) = 0;
	virtual void set_dlss_mv_low_res(bool p_enabled) = 0;
#endif
	virtual void set_texture_mipmap_bias(float p_texture_mipmap_bias) = 0;
	virtual void set_anisotropic_filtering_level(RSE::ViewportAnisotropicFiltering p_anisotropic_filtering_level) = 0;
	virtual void set_use_debanding(bool p_use_debanding) = 0;
};
