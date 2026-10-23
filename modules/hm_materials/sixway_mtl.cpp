/**************************************************************************/
/*  sixway_mtl.cpp                                                        */
/**************************************************************************/
/* License : Projekt Jenova Public Proprietary License (PJPPL v1.0)       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             CHADOT ENGINE                              */
/*                    https://chadotengine.github.io                      */
/**************************************************************************/
/* Copyright (c) 2024-present MemarDesign LLC. (Hamid.Memar)              */
/**************************************************************************/

#include "core/config/engine.h"
#include "servers/rendering/rendering_server.h"

// Imports
#include "sixway_mtl.h"

// Shader Storage
static Ref<Shader> shader_vl = nullptr;
static Ref<Shader> shader_fl = nullptr;
static Ref<Shader> _get_volume_shader() {
	if (shader_vl.is_null()) {
		shader_vl.instantiate();
		shader_vl->set_name("Six-Way Volumetric Shader");
		shader_vl->set_path("res://_6w_cache/volume.gdshader", true); // Mandatory for Documentation Parsing
		shader_vl->set_code(R"(
		// Developed By Hamid.Memar (MemarDesign LLC.)
		// License : PJPPL v1.0
		shader_type spatial;
		render_mode blend_mix, cull_disabled, depth_draw_always;
		/** Six Way Map RTB (Type: Texture2D): Right-Top-Back lighting texture (RGB channels contain lighting data for positive X, Y, and negative Z directions). */
		uniform sampler2D six_way_map_rtb : source_color;
		/** Six Way Map LBF (Type: Texture2D): Left-Bottom-Front lighting texture (RGB channels contain lighting data for negative X, negative Y, and positive Z directions). */
		uniform sampler2D six_way_map_lbf : source_color;
		/** Six Way Map TEA (Type: Texture2D): Transparency-Emission-AO texture (R channel for Alpha, G for Emission, B for Ambient Occlusion). */
		uniform sampler2D six_way_map_tea : source_color;
		/** Normal Map (Type: Texture2D): Normal map for simulating surface details. */
		uniform sampler2D normal_map : hint_normal;
		/** Emission Ramp (Type: Texture2D): Color ramp texture for emission effects. Blackbody can be used as well. */
		uniform sampler2D emission_ramp : source_color;
		/** Absorption (Type: Color): Light absorption color, Defines how much light is absorbed by the volume. */
		uniform vec3 absorption : source_color = vec3(1.0, 1.0, 1.0);
		/** Thickness (Type: float): Volume thickness multiplier (Affects light absorption). */
		uniform float thickness : hint_range(-10, 10) = 0.15;
		/** Density (Type: float): Density of the volume (Affects overall opacity). */
		uniform float density : hint_range(0, 1) = 1.0;
		/** Scattering (Type: float): Light scattering amount, Defines how much light diffuses through the volume. */
		uniform float scattering : hint_range(0, 1) = 0.0;
		/** Normal Power (Type: float): Strength of normal mapping effect. */
		uniform float normal_power : hint_range(0, 1) = 0.25;
		/** Normal Blend (Type: float): Blend factor between directional lighting and normal-mapped lighting. */
		uniform float normal_blend : hint_range(0.0, 1.0) = 0.25;
		/** AO Power (Type: float): Ambient occlusion strength from the TEA texture's B channel. */
		uniform float ao_power : hint_range(0.0, 1.0) = 0.5;
		/** emission_power (Type: float): Emission strength multiplier. */
		uniform float emission_power : hint_range(0.0, 10.0) = 1.0;
		/** Emission Color Shift (Type: float): Emission color shift. */
		uniform float emission_color_shift : hint_range(0.0, 360.0) = 1.0;
		/** Billboard Mode (Type: bool): Enables billboarding (face towards camera). */
		uniform bool billboard_mode = false;
		/** Billboard y-Axis Only (Type: bool): Billboard only around Y-axis (for trees/vertical objects). */
		uniform bool billboard_y_axis_only = false;
		/** billboard Keep Scale (Type: bool): Maintain object scale when billboarding. */
		uniform bool billboard_keep_scale = true;
		/** Animate (Type: bool): Enables flipbook animation. */
		uniform bool animate = false;
		/** Animate Speed (Type: float): Flipbook animation speed in frames per second. */
		uniform float animate_speed = 30.0;
		/** Animate Offset (Type: float): Time offset for animation start. */
		uniform float animate_offset = 0.0;
		/** Flipbook Dimensions (Type: Vector2): Grid dimensions for flipbook animation (columns, rows). */
		uniform ivec2 flipbook_dimensions = ivec2(2, 2);
		vec2 kAHX_(vec2 KsKI1,ivec2 DDVC6,float _6Zk3) { _6Zk3=floor(mod(_6Zk3,float(DDVC6.x*DDVC6.y)));vec2 tK7Uy=vec2(1.0)/vec2(DDVC6);vec2 FglILi=fract(KsKI1)*tK7Uy;FglILi.x+=(mod(_6Zk3,float(DDVC6.x)))*tK7Uy.x;FglILi.y+=float(int(_6Zk3)/DDVC6.x)*tK7Uy.y;return FglILi;} vec3 SFREYl(vec3 eH0Zqx,float HvJUDN) { float khM8u=dot(eH0Zqx,vec3(0.299,0.587,0.114));float vsBA4=dot(eH0Zqx,vec3(0.596,-0.274,-0.322));float EVxhz=dot(eH0Zqx,vec3(0.211,-0.523,0.312));float hbeto=radians(HvJUDN);float d2vpK=cos(hbeto);float JTDbQ=sin(hbeto);float x7a7W=vsBA4*d2vpK-EVxhz*JTDbQ;float cW1KQ=vsBA4*JTDbQ+EVxhz*d2vpK;return vec3( khM8u+0.956*x7a7W+0.621*cW1KQ, khM8u-0.272*x7a7W-0.647*cW1KQ, khM8u-1.106*x7a7W+1.703*cW1KQ );} void vertex() { if (billboard_mode&&billboard_y_axis_only) { MODELVIEW_MATRIX=VIEW_MATRIX*mat4( vec4(normalize(cross(vec3(0.0,1.0,0.0),MAIN_CAM_INV_VIEW_MATRIX[2].xyz)),0.0), vec4(0.0,1.0,0.0,0.0), vec4(normalize(cross(MAIN_CAM_INV_VIEW_MATRIX[0].xyz,vec3(0.0,1.0,0.0))),0.0), MODEL_MATRIX[3]);} if (billboard_mode&&!billboard_y_axis_only) { MODELVIEW_MATRIX=VIEW_MATRIX*mat4( MAIN_CAM_INV_VIEW_MATRIX[0], MAIN_CAM_INV_VIEW_MATRIX[1], MAIN_CAM_INV_VIEW_MATRIX[2], MODEL_MATRIX[3]);} if (billboard_mode&&billboard_keep_scale) { MODELVIEW_MATRIX=MODELVIEW_MATRIX*mat4( vec4(length(MODEL_MATRIX[0].xyz),0.0,0.0,0.0), vec4(0.0,length(MODEL_MATRIX[1].xyz),0.0,0.0), vec4(0.0,0.0,length(MODEL_MATRIX[2].xyz),0.0), vec4(0.0,0.0,0.0,1.0));MODELVIEW_NORMAL_MATRIX=mat3(MODELVIEW_MATRIX);} } void fragment() { vec2 KsKI1=vec2(1.0);if (animate) KsKI1=kAHX_(UV,flipbook_dimensions,TIME*animate_speed+animate_offset);else KsKI1=UV;vec3 UtUzP=mix(vec3(1.0),texture(six_way_map_tea,KsKI1).bbb,ao_power);ALBEDO=absorption*UtUzP;ALPHA=texture(six_way_map_tea,KsKI1).r*density;vec3 x2Xcz=texture(normal_map,KsKI1).rgb*2.0-1.0;vec3 nDzCjY=TANGENT*x2Xcz.x+BINORMAL*x2Xcz.y+NORMAL*x2Xcz.z;NORMAL_MAP=mix(vec3(.5,.5,1.0),nDzCjY,normal_power);float uXO1U=texture(six_way_map_tea,KsKI1).g;vec3 U9Ewr=texture(emission_ramp,vec2(uXO1U,0.0)).rgb*uXO1U*emission_power;EMISSION=SFREYl(U9Ewr,emission_color_shift);} void light() { vec2 KsKI1=vec2(1.0);if (animate) KsKI1=kAHX_(UV,flipbook_dimensions,TIME*animate_speed+animate_offset);else KsKI1=UV;vec3 qm1J9L=normalize(LIGHT);vec3 pZxJz=normalize(NORMAL);float Z0cjr=max(qm1J9L.x,0.0);float b8WM7=max(-qm1J9L.x,0.0);float WQUNL=max(qm1J9L.y,0.0);float lB4Xq=max(-qm1J9L.y,0.0);float lK8ZQ=max(qm1J9L.z,0.0);float cfLhX=max(-qm1J9L.z,0.0);float r1Uru=Z0cjr+b8WM7+WQUNL+lB4Xq+lK8ZQ+cfLhX;if (r1Uru>0.0){ Z0cjr/=r1Uru;b8WM7/=r1Uru;WQUNL/=r1Uru;lB4Xq/=r1Uru;lK8ZQ/=r1Uru;cfLhX/=r1Uru;} vec4 rtb=texture(six_way_map_rtb,KsKI1);vec4 M_Uve=texture(six_way_map_lbf,KsKI1);vec3 Rjt9Ak=Z0cjr*vec3(rtb.r)+b8WM7*vec3(M_Uve.r)+WQUNL*vec3(rtb.g)+lB4Xq*vec3(rtb.a)+lK8ZQ*vec3(M_Uve.b)+cfLhX*vec3(rtb.b);float A5gke=max(dot(pZxJz,qm1J9L),0.0);vec3 uZ4hp=mix(Rjt9Ak,Rjt9Ak*A5gke,normal_blend);vec3 Dvm86=uZ4hp*(1.0-(scattering/5.0))+(scattering/5.0)*vec3(1.0,1.0,1.0);vec3 UBUM1_=exp(-absorption*thickness);Dvm86=Dvm86*UBUM1_;DIFFUSE_LIGHT+=(Dvm86*LIGHT_COLOR*ATTENUATION/4.4);}
		)");
	}
	return shader_vl;
}
static Ref<Shader> _get_foliage_shader() {
	if (shader_fl.is_null()) {
		shader_fl.instantiate();
		shader_fl->set_name("Six-Way Foliage Shader");
		shader_fl->set_path("res://_6w_cache/foliage.gdshader", true); // Mandatory for Documentation Parsing
		shader_fl->set_code(R"(
		// Developed By Hamid.Memar (MemarDesign LLC.)
		// License : PJPPL v1.0
		shader_type spatial;
		render_mode blend_mix, cull_disabled, depth_draw_opaque, depth_prepass_alpha;
		uniform sampler2D SCREEN_TEXTURE : hint_screen_texture, filter_linear_mipmap;
		/** Six Way Map RTB (Type: Texture2D): Right-Top-Back lighting texture (RGB channels contain lighting data for positive X, Y, and negative Z directions). */
		uniform sampler2D six_way_map_RTB : source_color;
		/** Six Way Map LBF (Type: Texture2D): Left-Bottom-Front lighting texture (RGB channels contain lighting data for negative X, negative Y, and positive Z directions). */
		uniform sampler2D six_way_map_LBF : source_color;
		/** Six Way Map TEA (Type: Texture2D): Transparency-Emission-AO texture (R channel for Alpha, G for Emission, B for Ambient Occlusion). */
		uniform sampler2D six_way_map_TEA : source_color;
		/** Motion Vector Map (Type: Texture2D): Motion vector texture used for per-pixel animation. (Not Implemented Yet) */
		uniform sampler2D motion_vector_map : source_color;
		/** Normal Map (Type: Texture2D): Normal map for simulating surface details. */
		uniform sampler2D normal_map : hint_normal;
		/** Emission Ramp (Type: Texture2D): Color ramp texture for emission effects. Blackbody can be used as well. */
		uniform sampler2D emission_ramp : source_color;
		/** Albedo (Type: Texture2D): Base color texture for the foliage. */
		uniform sampler2D albedo : source_color;
		/** Thickness (Type: float): Volume thickness multiplier (Affects light absorption). */
		uniform float thickness : hint_range(-10, 10) = 0.15;
		/** Density (Type: float): Density of the volume (Affects overall opacity). */
		uniform float density : hint_range(0, 1) = 1.0;
		/** Scattering (Type: float): Light scattering amount, Defines how much light diffuses through the volume. */
		uniform float scattering : hint_range(0, 1) = 0.0;
		/** Normal Power (Type: float): Strength of normal mapping effect. */
		uniform float normal_power : hint_range(0, 1) = 0.25;
		/** Normal Blend (Type: float): Blend factor between directional lighting and normal-mapped lighting. */
		uniform float normal_blend : hint_range(0.0, 1.0) = 0.25;
		/** AO Power (Type: float): Ambient occlusion strength from the TEA texture's B channel. */
		uniform float ao_power : hint_range(0.0, 1.0) = 0.5;
		/** emission_power (Type: float): Emission strength multiplier. */
		uniform float emission_power : hint_range(0.0, 10.0) = 1.0;
		/** Emission Color Shift (Type: float): Emission color shift. */
		uniform float emission_color_shift : hint_range(0.0, 360.0) = 1.0;
		/** Alpha Scissor Threshold (Type: float): Alpha cutoff threshold for alpha-scissor transparency. */
		uniform float alpha_scissor_threshold : hint_range(0.0, 1.0) = 0.1;
		/** Billboard Mode (Type: bool): Enables billboarding (face towards camera). */
		uniform bool billboard_mode = false;
		/** billboard Keep Scale (Type: bool): Maintain object scale when billboarding. */
		uniform bool billboard_keep_scale = true;
		/** Animate (Type: bool): Enables flipbook animation. */
		uniform bool animate = false;
		/** Animate Speed (Type: float): Flipbook animation speed in frames per second. */
		uniform float animate_speed = 30.0;
		/** Animate Offset (Type: float): Time offset for animation start. */
		uniform float animate_offset = 0.0;
		/** Flipbook Dimensions (Type: Vector2): Grid dimensions for flipbook animation (columns, rows). */
		uniform ivec2 flipbook_dimensions = ivec2(2, 2);
		/** Depth Fade Distance (Type: float): Distance over which the foliage fades to avoid hard intersections with geometry. */
		uniform float depth_fade_distance : hint_range(0.0, 1.0) = 0.05;
		/** Depth Fade Strength (Type: float): Strength of the depth fade effect. */
		uniform float depth_fade_strength : hint_range(0.0, 1.0) = 0.0;
		/** Scale Variation (Type: float): Per-instance random scale variation for trees. */
		uniform float scale_variation : hint_range(0.0, 1.0) = 0.0;
		/** Hue Variation (Type: float): Per-instance random hue variation for trees. */
		uniform float hue_variation : hint_range(0.0, 1.0) = 0.0;
		/** Value Variation (Type: float): Per-instance random value (brightness) variation for trees. */
		uniform float value_variation : hint_range(0.0, 1.0) = 0.0;
		vec3 SFREYl(vec3 eH0Zqx,float HvJUDN) { float khM8u=dot(eH0Zqx,vec3(0.299,0.587,0.114));float vsBA4=dot(eH0Zqx,vec3(0.596,-0.274,-0.322));float EVxhz=dot(eH0Zqx,vec3(0.211,-0.523,0.312));float hbeto=radians(HvJUDN);float d2vpK=cos(hbeto);float JTDbQ=sin(hbeto);float x7a7W=vsBA4*d2vpK-EVxhz*JTDbQ;float cW1KQ=vsBA4*JTDbQ+EVxhz*d2vpK;return vec3( khM8u+0.956*x7a7W+0.621*cW1KQ, khM8u-0.272*x7a7W-0.647*cW1KQ, khM8u-1.106*x7a7W+1.703*cW1KQ );} float qz2tu(uint x){ x=(x ^ 61u) ^ (x>>16u);x=x+(x<<3u);x=x ^ (x>>4u);x=x*0x27d4eb2du;x=x ^ (x>>15u);return float(x)/float(0xffffffffu);} vec3 hW7wK(vec3 SUnP9VIj) { vec4 BGmuB=vec4(0.0,-1.0/3.0,2.0/3.0,-1.0);vec4 p=mix(vec4(SUnP9VIj.bg,BGmuB.wz),vec4(SUnP9VIj.gb,BGmuB.xy),step(SUnP9VIj.b,SUnP9VIj.g));vec4 Rvh1L=mix(vec4(p.xyw,SUnP9VIj.r),vec4(SUnP9VIj.r,p.yzx),step(p.x,SUnP9VIj.r));float EWsYo=Rvh1L.x-min(Rvh1L.w,Rvh1L.y);float QBrHr=1.0e-10;return vec3(abs(Rvh1L.z+(Rvh1L.w-Rvh1L.y)/(6.0*EWsYo+QBrHr)),EWsYo/(Rvh1L.x+QBrHr),Rvh1L.x);} vec3 UYQn4(vec3 SUnP9VIj) { vec4 BGmuB=vec4(1.0,2.0/3.0,1.0/3.0,3.0);vec3 p=abs(fract(SUnP9VIj.xxx+BGmuB.xyz)*6.0-BGmuB.www);return SUnP9VIj.z*mix(BGmuB.xxx,clamp(p-BGmuB.xxx,0.0,1.0),SUnP9VIj.y);}   vec2 kAHX_(vec2 KsKI1,ivec2 DDVC6,float _6Zk3) { _6Zk3=floor(mod(_6Zk3,float(DDVC6.x*DDVC6.y)));vec2 tK7Uy=vec2(1.0)/vec2(DDVC6);vec2 FglILi=fract(KsKI1)*tK7Uy;FglILi.x+=(mod(_6Zk3,float(DDVC6.x)))*tK7Uy.x;FglILi.y+=float(int(_6Zk3)/DDVC6.x)*tK7Uy.y;return FglILi;}   varying flat uint EOq9P;void vertex() { EOq9P=uint(INSTANCE_ID);if (billboard_mode) { MODELVIEW_MATRIX=VIEW_MATRIX*mat4( INV_VIEW_MATRIX[0], INV_VIEW_MATRIX[1], INV_VIEW_MATRIX[2], MODEL_MATRIX[3]);MODELVIEW_NORMAL_MATRIX=mat3(MODELVIEW_MATRIX);}   if (billboard_mode&&billboard_keep_scale) { MODELVIEW_MATRIX=MODELVIEW_MATRIX*mat4( vec4(length(MODEL_MATRIX[0].xyz),0.0,0.0,0.0), vec4(0.0,length(MODEL_MATRIX[1].xyz),0.0,0.0), vec4(0.0,0.0,length(MODEL_MATRIX[2].xyz),0.0), vec4(0.0,0.0,0.0,1.0));MODELVIEW_NORMAL_MATRIX=mat3(MODELVIEW_MATRIX);}   float bVNDYN=1.0+(qz2tu(uint(INSTANCE_ID))-0.5)*scale_variation*2.0;MODELVIEW_MATRIX=MODELVIEW_MATRIX*mat4( vec4(bVNDYN,0.0,0.0,0.0), vec4(0.0,bVNDYN,0.0,0.0), vec4(0.0,0.0,bVNDYN,0.0), vec4(0.0,0.0,0.0,1.0));MODELVIEW_NORMAL_MATRIX=mat3(MODELVIEW_MATRIX);}   void fragment() {  vec2 KsKI1=vec2(1.0);vec2 lxa7k=vec2(1.0);if (animate){ KsKI1=kAHX_(UV,flipbook_dimensions,TIME*animate_speed+animate_offset);lxa7k=kAHX_(UV,flipbook_dimensions,TIME*animate_speed+(animate_offset+1.0));} else KsKI1=UV;KsKI1.y+=0.0008;vec4 F28lyE=texture(six_way_map_TEA,KsKI1);if (animate){ float SmO6v=fract(TIME*animate_speed+animate_offset);vec4 pJdsh=texture(six_way_map_TEA,lxa7k);F28lyE=mix(F28lyE,pJdsh,SmO6v);}   vec3 UtUzP=mix(vec3(1.0),F28lyE.bbb,ao_power);vec3 eH0Zqx=texture(albedo,KsKI1).rgb;float VJe0HZrD=(qz2tu(uint(EOq9P+5000u))-0.5)*hue_variation*360.0;float JoTZz=1.0+(qz2tu(EOq9P+6000u)-0.5)*value_variation*2.0;vec3 Q9otR=hW7wK(eH0Zqx);Q9otR.x=fract(Q9otR.x+VJe0HZrD/360.0);Q9otR.z=clamp(Q9otR.z*JoTZz,0.0,1.0);eH0Zqx=UYQn4(Q9otR);ALBEDO=eH0Zqx*UtUzP;ALPHA=F28lyE.r*density;vec3 x2Xcz=texture(normal_map,KsKI1).rgb*2.0-1.0;vec3 nDzCjY=TANGENT*x2Xcz.x+BINORMAL*x2Xcz.y+NORMAL*x2Xcz.z;NORMAL_MAP=mix(vec3(.5,.5,1.0),nDzCjY,normal_power);vec2 n2fj3=SCREEN_UV;vec4 LSbjm=FRAGCOORD;float mkWkvW=textureLod(SCREEN_TEXTURE,n2fj3,0.0).r;float ndt20=LSbjm.z;float rF0HD=mkWkvW-ndt20;float NlCzv=clamp(rF0HD/depth_fade_distance,0.0,1.0);ALPHA*=mix(1.0,NlCzv,depth_fade_strength);float uXO1U=F28lyE.g;vec3 U9Ewr=texture(emission_ramp,vec2(uXO1U,0.0)).rgb*uXO1U*emission_power;EMISSION=SFREYl(U9Ewr,emission_color_shift);ALPHA_SCISSOR_THRESHOLD=alpha_scissor_threshold;}   void light() {  vec2 KsKI1=vec2(1.0);vec2 lxa7k=vec2(1.0);if (animate){ KsKI1=kAHX_(UV,flipbook_dimensions,TIME*animate_speed+animate_offset);lxa7k=kAHX_(UV,flipbook_dimensions,TIME*animate_speed+(animate_offset+1.0));} else KsKI1=UV;vec3 qm1J9L=normalize(LIGHT);vec3 pZxJz=normalize(NORMAL);float Z0cjr=max(qm1J9L.x,0.0);float b8WM7=max(-qm1J9L.x,0.0);float WQUNL=max(qm1J9L.y,0.0);float lB4Xq=max(-qm1J9L.y,0.0);float lK8ZQ=max(qm1J9L.z,0.0);float cfLhX=max(-qm1J9L.z,0.0);float r1Uru=Z0cjr+b8WM7+WQUNL+lB4Xq+lK8ZQ+cfLhX;if (r1Uru>0.0){ Z0cjr/=r1Uru;b8WM7/=r1Uru;WQUNL/=r1Uru;lB4Xq/=r1Uru;lK8ZQ/=r1Uru;cfLhX/=r1Uru;}   vec4 rtb=texture(six_way_map_RTB,KsKI1);vec4 M_Uve=texture(six_way_map_LBF,KsKI1);if (animate){ float SmO6v=fract(TIME*animate_speed+animate_offset);vec4 biH15=texture(six_way_map_RTB,lxa7k);rtb=mix(rtb,biH15,SmO6v);vec4 vm3Gr=texture(six_way_map_LBF,lxa7k);M_Uve=mix(M_Uve,vm3Gr,SmO6v);}   vec3 Rjt9Ak=Z0cjr*vec3(rtb.r)+b8WM7*vec3(M_Uve.r)+ WQUNL*vec3(rtb.g)+lB4Xq*vec3(rtb.a)+ lK8ZQ*vec3(M_Uve.b)+cfLhX*vec3(rtb.b);float A5gke=max(dot(pZxJz,qm1J9L),0.0);vec3 uZ4hp=mix(Rjt9Ak,Rjt9Ak*A5gke,normal_blend);vec3 Dvm86=uZ4hp*(1.0-(scattering/5.0))+(scattering/5.0)*vec3(1.0,1.0,1.0);vec3 eH0Zqx=texture(albedo,KsKI1).rgb;vec3 UBUM1_=exp(-eH0Zqx*thickness);Dvm86=Dvm86*UBUM1_;DIFFUSE_LIGHT+=(Dvm86*LIGHT_COLOR*ATTENUATION/4.4);}
		)");
	}
	return shader_fl;
}

// Six-Way Lighting Volume Material Implementation
SixWayVolumeMaterial::SixWayVolumeMaterial() {
	ShaderMaterial::set_shader(_get_volume_shader());
}

void SixWayVolumeMaterial::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name == "shader") {
		p_property.usage = PROPERTY_USAGE_NONE;
	}
}

void SixWayVolumeMaterial::set_shader(const Ref<Shader> &p_shader) {
	ShaderMaterial::set_shader(_get_volume_shader());
}

// Six-Way Lighting Foliage Material Implementation
SixWayFoliageMaterial::SixWayFoliageMaterial() {
	ShaderMaterial::set_shader(_get_foliage_shader());
}

void SixWayFoliageMaterial::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name == "shader") {
		p_property.usage = PROPERTY_USAGE_NONE;
	}
}

void SixWayFoliageMaterial::set_shader(const Ref<Shader> &p_shader) {
	ShaderMaterial::set_shader(_get_foliage_shader());
}

// Shared Methods Implementation
void ReleaseSixWayShaders() {
	if (shader_vl.is_valid()) {
		shader_vl.unref();
		shader_vl = nullptr;
	}
	if (shader_fl.is_valid()) {
		shader_fl.unref();
		shader_fl = nullptr;
	}
}
