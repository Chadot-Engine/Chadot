/**************************************************************************/
/*  render_data_extension.h                                               */
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

#include "core/object/gdvirtual.gen.h"
#include "servers/rendering/storage/render_data.h"
#include "servers/rendering/storage/render_scene_buffers.h"
#include "servers/rendering/storage/render_scene_data.h"

class RenderSceneBuffersExtension : public RenderSceneBuffers {
	GDCLASS(RenderSceneBuffersExtension, RenderSceneBuffers);

protected:
	static void _bind_methods();

	GDVIRTUAL1(_configure, const RenderSceneBuffersConfiguration *)
	GDVIRTUAL1(_set_fsr_sharpness, float)
#ifdef DLSS_FEATURE_ENABLED
	GDVIRTUAL1(_set_dlss_quality, int)
	GDVIRTUAL1(_set_dlss_preset, int)
	GDVIRTUAL1(_set_dlss_sharpness, float)
	GDVIRTUAL1(_set_dlss_hdr, bool)
	GDVIRTUAL1(_set_dlss_auto_exposure, bool)
	GDVIRTUAL1(_set_dlss_alpha_upscaling, bool)
	GDVIRTUAL1(_set_dlss_depth_inverted, bool)
	GDVIRTUAL1(_set_dlss_mv_low_res, bool)
#endif
	GDVIRTUAL1(_set_texture_mipmap_bias, float)
	GDVIRTUAL1(_set_anisotropic_filtering_level, int)
	GDVIRTUAL1(_set_use_debanding, bool)

public:
	virtual ~RenderSceneBuffersExtension() {}

	virtual void configure(const RenderSceneBuffersConfiguration *p_config) override;

	virtual void set_fsr_sharpness(float p_fsr_sharpness) override;
#ifdef DLSS_FEATURE_ENABLED
	virtual void set_dlss_quality(int p_quality) override;
	virtual void set_dlss_preset(int p_preset) override;
	virtual void set_dlss_sharpness(float p_sharpness) override;
	virtual void set_dlss_hdr(bool p_enabled) override;
	virtual void set_dlss_auto_exposure(bool p_enabled) override;
	virtual void set_dlss_alpha_upscaling(bool p_enabled) override;
	virtual void set_dlss_depth_inverted(bool p_enabled) override;
	virtual void set_dlss_mv_low_res(bool p_enabled) override;
#endif
	virtual void set_texture_mipmap_bias(float p_texture_mipmap_bias) override;
	virtual void set_anisotropic_filtering_level(RSE::ViewportAnisotropicFiltering p_anisotropic_filtering_level) override;
	virtual void set_use_debanding(bool p_use_debanding) override;
};

class RenderSceneDataExtension : public RenderSceneData {
	GDCLASS(RenderSceneDataExtension, RenderSceneData);

protected:
	static void _bind_methods();

public:
	virtual Transform3D get_cam_transform() const override;
	virtual Projection get_cam_projection() const override;

	virtual uint32_t get_view_count() const override;
	virtual Vector3 get_view_eye_offset(uint32_t p_view) const override;
	virtual Projection get_view_projection(uint32_t p_view) const override;

	virtual RID get_uniform_buffer() const override;

	GDVIRTUAL0RC(Transform3D, _get_cam_transform)
	GDVIRTUAL0RC(Projection, _get_cam_projection)

	GDVIRTUAL0RC(uint32_t, _get_view_count)
	GDVIRTUAL1RC(Vector3, _get_view_eye_offset, uint32_t)
	GDVIRTUAL1RC(Projection, _get_view_projection, uint32_t)

	GDVIRTUAL0RC(RID, _get_uniform_buffer)
};

class RenderDataExtension : public RenderData {
	GDCLASS(RenderDataExtension, RenderData);

protected:
	static void _bind_methods();

	virtual Ref<RenderSceneBuffers> get_render_scene_buffers() const override;
	virtual RenderSceneData *get_render_scene_data() const override;

	virtual RID get_environment() const override;
	virtual RID get_camera_attributes() const override;

	GDVIRTUAL0RC(Ref<RenderSceneBuffers>, _get_render_scene_buffers)
	GDVIRTUAL0RC(RenderSceneData *, _get_render_scene_data)
	GDVIRTUAL0RC(RID, _get_environment)
	GDVIRTUAL0RC(RID, _get_camera_attributes)
};
