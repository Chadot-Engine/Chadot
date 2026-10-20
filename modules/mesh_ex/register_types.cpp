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

#include "core/config/engine.h"
#include "core/object/class_db.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/node_3d.h"
#include "scene/resources/image_texture.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh.h"

// Configuration
const int BRUTE_FORCE_THRESHOLD = 2000;

// MeshInstanceEx3D Implementation
class MeshInstanceEx3D : public MeshInstance3D {
	GDCLASS(MeshInstanceEx3D, MeshInstance3D);

private:
	// Properties
	Ref<Texture2D> id_map = nullptr;
	Vector3i cache_division = Vector3i(10, 10, 10);

private:
	// Cache Data Structures
	struct TriangleData {
		Vector3 v0, v1, v2;
		Vector2 uv0, uv1, uv2;
		int surface_index = 0;
	};
	struct SpatialGrid {
		Vector<int> cells;
		Vector<int> cell_offsets;
		Vector3i dimensions;
		Vector3 cell_size;
		AABB bounds;
		int total_cells = 0;

		void build(const Vector<TriangleData> &triangles, const AABB &p_bounds, const Vector3i &cache_division) {
			bounds = p_bounds;
			Vector3 size = bounds.get_size();
			dimensions = cache_division;
			cell_size = Vector3(size.x / dimensions.x, size.y / dimensions.y, size.z / dimensions.z);
			total_cells = dimensions.x * dimensions.y * dimensions.z;

			// First Pass
			Vector<int> cell_counts;
			cell_counts.resize(total_cells);
			cell_counts.fill(0);

			for (int t = 0; t < triangles.size(); t++) {
				const TriangleData &tri = triangles[t];
				AABB tri_bounds(tri.v0, Vector3(0, 0, 0));
				tri_bounds.expand_to(tri.v1);
				tri_bounds.expand_to(tri.v2);

				Vector3i min_cell = get_cell_index(tri_bounds.position);
				Vector3i max_cell = get_cell_index(tri_bounds.position + tri_bounds.size);

				for (int x = min_cell.x; x <= max_cell.x; x++) {
					for (int y = min_cell.y; y <= max_cell.y; y++) {
						for (int z = min_cell.z; z <= max_cell.z; z++) {
							if (x >= 0 && x < dimensions.x && y >= 0 && y < dimensions.y && z >= 0 && z < dimensions.z) {
								int idx = x + y * dimensions.x + z * dimensions.x * dimensions.y;
								int current = cell_counts[idx];
								cell_counts.set(idx, current + 1);
							}
						}
					}
				}
			}

			// Build Offset Table
			cell_offsets.resize(total_cells + 1);
			cell_offsets.set(0, 0);
			for (int i = 0; i < total_cells; i++) {
				cell_offsets.set(i + 1, cell_offsets[i] + cell_counts[i]);
			}

			// Alloctae Flat Array
			cells.resize(cell_offsets[total_cells]);
			Vector<int> cell_writers = cell_offsets;

			// Second Pass
			for (int t = 0; t < triangles.size(); t++) {
				const TriangleData &tri = triangles[t];
				AABB tri_bounds(tri.v0, Vector3(0, 0, 0));
				tri_bounds.expand_to(tri.v1);
				tri_bounds.expand_to(tri.v2);

				Vector3i min_cell = get_cell_index(tri_bounds.position);
				Vector3i max_cell = get_cell_index(tri_bounds.position + tri_bounds.size);

				for (int x = min_cell.x; x <= max_cell.x; x++) {
					for (int y = min_cell.y; y <= max_cell.y; y++) {
						for (int z = min_cell.z; z <= max_cell.z; z++) {
							if (x >= 0 && x < dimensions.x &&
									y >= 0 && y < dimensions.y &&
									z >= 0 && z < dimensions.z) {
								int cell_idx = x + y * dimensions.x + z * dimensions.x * dimensions.y;
								int write_idx = cell_writers[cell_idx];
								cells.set(write_idx, t);
								cell_writers.set(cell_idx, write_idx + 1);
							}
						}
					}
				}
			}
		}
		Vector3i get_cell_index(const Vector3 &point) const {
			Vector3 local = point - bounds.position;
			return Vector3i(
					CLAMP(int(local.x / cell_size.x), 0, dimensions.x - 1),
					CLAMP(int(local.y / cell_size.y), 0, dimensions.y - 1),
					CLAMP(int(local.z / cell_size.z), 0, dimensions.z - 1));
		}
		int get_cell_linear_index(const Vector3i &cell) const {
			if (cell.x < 0 || cell.x >= dimensions.x || cell.y < 0 || cell.y >= dimensions.y || cell.z < 0 || cell.z >= dimensions.z) {
				return -1;
			}
			return cell.x + cell.y * dimensions.x + cell.z * dimensions.x * dimensions.y;
		}
		void get_cell_triangles(int cell_idx, Vector<int> &out_triangles) const {
			out_triangles.clear();
			if (cell_idx < 0 || cell_idx >= total_cells) {
				return;
			}

			int start = cell_offsets[cell_idx];
			int end = cell_offsets[cell_idx + 1];

			for (int i = start; i < end; i++) {
				out_triangles.push_back(cells[i]);
			}
		}
	};

	// Internal Data
	mutable Vector<TriangleData> cached_triangles;
	mutable SpatialGrid spatial_grid;
	mutable AABB cached_bounds;
	mutable bool cache_dirty = true;

protected:
	// Bindings
	static void _bind_methods() {
		// Methods
		ClassDB::bind_method(D_METHOD("get_uv_at_position", "world_point"), &MeshInstanceEx3D::get_uv_at_position);
		ClassDB::bind_method(D_METHOD("get_color_from_uv", "hit_uv"), &MeshInstanceEx3D::get_color_from_uv);
		ClassDB::bind_method(D_METHOD("invalidate_cache"), &MeshInstanceEx3D::invalidate_cache);

		// Properties
		ClassDB::bind_method(D_METHOD("set_cache_division", "division"), &MeshInstanceEx3D::set_cache_division);
		ClassDB::bind_method(D_METHOD("get_cache_division"), &MeshInstanceEx3D::get_cache_division);
		ADD_PROPERTY(PropertyInfo(Variant::VECTOR3I, "cache_division"), "set_cache_division", "get_cache_division");

		ClassDB::bind_method(D_METHOD("set_id_map", "texture"), &MeshInstanceEx3D::set_id_map);
		ClassDB::bind_method(D_METHOD("get_id_map"), &MeshInstanceEx3D::get_id_map);
		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "id_map", PropertyHint::PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_id_map", "get_id_map");
	}

private:
	// Setters/Getters
	void set_cache_division(const Vector3i &p_division) {
		Vector3i safe_division = p_division;

		if (safe_division.x == 0) {
			safe_division.x = 2;
		}
		if (safe_division.y == 0) {
			safe_division.y = 2;
		}
		if (safe_division.z == 0) {
			safe_division.z = 2;
		}

		cache_division = safe_division;
	}
	Vector3i get_cache_division() const { return cache_division; }
	void set_id_map(const Ref<Texture2D> &p_texture) { id_map = p_texture; }
	Ref<Texture2D> get_id_map() const { return id_map; }

private:
	// Internal Functions
	void build_cache() const {
		MeshInstanceEx3D *mutable_this = const_cast<MeshInstanceEx3D *>(this);
		mutable_this->cached_triangles.clear();

		Ref<Mesh> _mesh = get_mesh();
		if (_mesh.is_null()) {
			return;
		}

		Ref<ArrayMesh> array_mesh = _mesh;
		if (array_mesh.is_null()) {
			return;
		}

		int surface_count = array_mesh->get_surface_count();
		AABB total_bounds;
		bool first_bounds = true;

		for (int s = 0; s < surface_count; s++) {
			Array arrays = array_mesh->surface_get_arrays(s);
			if (arrays.is_empty()) {
				continue;
			}

			PackedVector3Array vertices = arrays[Mesh::ARRAY_VERTEX];
			PackedVector2Array uvs = arrays[Mesh::ARRAY_TEX_UV];
			PackedInt32Array indices = arrays[Mesh::ARRAY_INDEX];

			if (vertices.size() == 0 || uvs.size() == 0) {
				continue;
			}

			if (indices.size() == 0) {
				indices.resize(vertices.size());
				for (int i = 0; i < vertices.size(); i++) {
					indices.set(i, i);
				}
			}

			for (int i = 0; i < indices.size() - 2; i += 3) {
				int idx0 = indices[i];
				int idx1 = indices[i + 1];
				int idx2 = indices[i + 2];

				if (idx0 >= vertices.size() || idx1 >= vertices.size() || idx2 >= vertices.size()) {
					continue;
				}
				if (idx0 >= uvs.size() || idx1 >= uvs.size() || idx2 >= uvs.size()) {
					continue;
				}

				TriangleData tri;
				tri.v0 = vertices[idx0];
				tri.v1 = vertices[idx1];
				tri.v2 = vertices[idx2];
				tri.uv0 = uvs[idx0];
				tri.uv1 = uvs[idx1];
				tri.uv2 = uvs[idx2];
				tri.surface_index = s;

				mutable_this->cached_triangles.push_back(tri);

				if (first_bounds) {
					total_bounds = AABB(tri.v0, Vector3(0, 0, 0));
					first_bounds = false;
				}
				total_bounds.expand_to(tri.v0);
				total_bounds.expand_to(tri.v1);
				total_bounds.expand_to(tri.v2);
			}
		}

		mutable_this->cached_bounds = total_bounds;

		// Build Spatial Grid Cache
		if (cached_triangles.size() > BRUTE_FORCE_THRESHOLD) {
			mutable_this->spatial_grid.build(cached_triangles, cached_bounds, cache_division);
		}

		mutable_this->cache_dirty = false;
	}
	Vector3 closest_point_on_triangle(const Vector3 &p_point, const Vector3 &p_v0, const Vector3 &p_v1, const Vector3 &p_v2) const {
		Vector3 edge0 = p_v1 - p_v0;
		Vector3 edge1 = p_v2 - p_v0;
		Vector3 v0_to_point = p_point - p_v0;

		float dot00 = edge0.dot(edge0);
		float dot01 = edge0.dot(edge1);
		float dot11 = edge1.dot(edge1);
		float dot0p = edge0.dot(v0_to_point);
		float dot1p = edge1.dot(v0_to_point);

		float denom = dot00 * dot11 - dot01 * dot01;
		if (Math::abs(denom) < 0.0001f) {
			return (p_v0 + p_v1 + p_v2) / 3.0f;
		}

		float u = (dot11 * dot0p - dot01 * dot1p) / denom;
		float v = (dot00 * dot1p - dot01 * dot0p) / denom;

		if (u >= 0.0f && v >= 0.0f && (u + v) <= 1.0f) {
			return p_v0 + edge0 * u + edge1 * v;
		}

		Vector3 closest_on_ab = closest_point_on_segment(p_point, p_v0, p_v1);
		Vector3 closest_on_bc = closest_point_on_segment(p_point, p_v1, p_v2);
		Vector3 closest_on_ca = closest_point_on_segment(p_point, p_v2, p_v0);

		float dist_ab = p_point.distance_squared_to(closest_on_ab);
		float dist_bc = p_point.distance_squared_to(closest_on_bc);
		float dist_ca = p_point.distance_squared_to(closest_on_ca);

		if (dist_ab <= dist_bc && dist_ab <= dist_ca) {
			return closest_on_ab;
		}
		if (dist_bc <= dist_ab && dist_bc <= dist_ca) {
			return closest_on_bc;
		}
		return closest_on_ca;
	}
	Vector3 closest_point_on_segment(const Vector3 &p_point, const Vector3 &p_a, const Vector3 &p_b) const {
		Vector3 ab = p_b - p_a;
		float t = (p_point - p_a).dot(ab) / ab.dot(ab);
		t = CLAMP(t, 0.0f, 1.0f);
		return p_a + ab * t;
	}
	Vector3 barycentric_coords(const Vector3 &p_point, const Vector3 &p_v0, const Vector3 &p_v1, const Vector3 &p_v2) const {
		Vector3 v0v1 = p_v1 - p_v0;
		Vector3 v0v2 = p_v2 - p_v0;
		Vector3 v0p = p_point - p_v0;

		float d00 = v0v1.dot(v0v1);
		float d01 = v0v1.dot(v0v2);
		float d11 = v0v2.dot(v0v2);
		float d20 = v0p.dot(v0v1);
		float d21 = v0p.dot(v0v2);

		float denom = d00 * d11 - d01 * d01;
		if (Math::abs(denom) < 0.0001f) {
			return Vector3(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f);
		}

		float v = (d11 * d20 - d01 * d21) / denom;
		float w = (d00 * d21 - d01 * d20) / denom;
		float u = 1.0f - v - w;

		if (u >= 0.0f && v >= 0.0f && w >= 0.0f) {
			return Vector3(u, v, w);
		}

		Vector3 closest;
		float closest_dist = FLT_MAX;

		Vector3 edges[3] = { p_v1 - p_v0, p_v2 - p_v1, p_v0 - p_v2 };
		Vector3 vertices[3] = { p_v0, p_v1, p_v2 };

		for (int i = 0; i < 3; ++i) {
			Vector3 edge = edges[i];
			Vector3 to_point = p_point - vertices[i];
			float edge_len_sq = edge.dot(edge);
			if (edge_len_sq < 0.0001f) {
				Vector3 candidate = vertices[i];
				Vector3 diff = candidate - p_point;
				float dist = diff.dot(diff);
				if (dist < closest_dist) {
					closest_dist = dist;
					closest = candidate;
				}
				continue;
			}
			float t = to_point.dot(edge) / edge_len_sq;
			t = CLAMP(t, 0.0f, 1.0f);
			Vector3 candidate = vertices[i] + edge * t;
			Vector3 diff = candidate - p_point;
			float dist = diff.dot(diff);
			if (dist < closest_dist) {
				closest_dist = dist;
				closest = candidate;
			}
		}

		Vector3 closest_v0v1 = p_v1 - p_v0;
		Vector3 closest_v0v2 = p_v2 - p_v0;
		Vector3 closest_v0p = closest - p_v0;

		float closest_d00 = closest_v0v1.dot(closest_v0v1);
		float closest_d01 = closest_v0v1.dot(closest_v0v2);
		float closest_d11 = closest_v0v2.dot(closest_v0v2);
		float closest_d20 = closest_v0p.dot(closest_v0v1);
		float closest_d21 = closest_v0p.dot(closest_v0v2);
		float closest_denom = closest_d00 * closest_d11 - closest_d01 * closest_d01;

		if (Math::abs(closest_denom) < 0.0001f) {
			return Vector3(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f);
		}

		float closest_v = (closest_d11 * closest_d20 - closest_d01 * closest_d21) / closest_denom;
		float closest_w = (closest_d00 * closest_d21 - closest_d01 * closest_d20) / closest_denom;
		float closest_u = 1.0f - closest_v - closest_w;
		return Vector3(closest_u, closest_v, closest_w);
	}

public:
	// Public Functions
	Vector2 get_uv_at_position(const Vector3 &world_point) const {
		// Get Mesh
		Ref<Mesh> _mesh = get_mesh();
		if (_mesh.is_null()) {
			return Vector2(0, 0);
		}

		// Build Cache if Needed
		if (cache_dirty) {
			build_cache();
		}

		Transform3D global_transform = get_global_transform();
		Vector3 local_point = to_local(world_point);

		float best_distance = INFINITY;
		Vector2 best_uv = Vector2(0, 0);
		bool found = false;

		if (cached_triangles.size() <= BRUTE_FORCE_THRESHOLD) {
			for (const TriangleData &tri : cached_triangles) {
				Vector3 closest = closest_point_on_triangle(local_point, tri.v0, tri.v1, tri.v2);
				float distance = local_point.distance_squared_to(closest);
				if (distance < best_distance) {
					best_distance = distance;
					Vector3 bary = barycentric_coords(closest, tri.v0, tri.v1, tri.v2);
					best_uv = tri.uv0 * bary.x + tri.uv1 * bary.y + tri.uv2 * bary.z;
					found = true;
				}
			}
		} else {
			// Use Pre-Built Spatial Grid
			Vector3i cell_idx = spatial_grid.get_cell_index(local_point);
			int linear_idx = spatial_grid.get_cell_linear_index(cell_idx);

			// Find Candidates
			Vector<int> candidates;
			spatial_grid.get_cell_triangles(linear_idx, candidates);
			for (int tri_idx : candidates) {
				const TriangleData &tri = cached_triangles[tri_idx];
				Vector3 closest = closest_point_on_triangle(local_point, tri.v0, tri.v1, tri.v2);
				float distance = local_point.distance_squared_to(closest);

				if (distance < best_distance) {
					best_distance = distance;
					Vector3 bary = barycentric_coords(closest, tri.v0, tri.v1, tri.v2);
					best_uv = tri.uv0 * bary.x + tri.uv1 * bary.y + tri.uv2 * bary.z;
					found = true;
				}
			}

			// If Not Found in Immediate Cell, Fallback to Full Search
			if (!found) {
				for (const TriangleData &tri : cached_triangles) {
					Vector3 closest = closest_point_on_triangle(local_point, tri.v0, tri.v1, tri.v2);
					float distance = local_point.distance_squared_to(closest);

					if (distance < best_distance) {
						best_distance = distance;
						Vector3 bary = barycentric_coords(closest, tri.v0, tri.v1, tri.v2);
						best_uv = tri.uv0 * bary.x + tri.uv1 * bary.y + tri.uv2 * bary.z;
						found = true;
					}
				}
			}
		}

		return found ? best_uv : Vector2(0, 0);
	}
	Color get_color_from_uv(const Vector2 &hit_uv) const {
		if (id_map.is_null()) {
			print_error("No ID Map is Assigned, Cannot sample color!");
			return Color(0, 0, 0, 0);
		}
		Ref<Image> image = id_map->get_image();
		if (image.is_null()) {
			print_error("Image is Null, Cannot sample color!");
			return Color(0, 0, 0, 0);
		}
		int width = image->get_width();
		int height = image->get_height();
		int x = CLAMP(int(hit_uv.x * width), 0, width - 1);
		int y = CLAMP(int(hit_uv.y * height), 0, height - 1);
		return image->get_pixel(x, y);
	}
	void invalidate_cache() {
		cache_dirty = true;
	}

public:
	void _notification(int p_what) {
		switch (p_what) {
			case NOTIFICATION_READY:
				// Skip Editor
				if (Engine::get_singleton()->is_editor_hint()) {
					return;
				}

				// Build Cache for First Time
				build_cache();
				break;
			case NOTIFICATION_EXIT_TREE:
				// Skip Editor
				if (Engine::get_singleton()->is_editor_hint()) {
					return;
				}

				// Clean Up
				cached_triangles.clear();
				cached_triangles.resize(0);
				spatial_grid.cells.clear();
				spatial_grid.cells.resize(0);
				spatial_grid.cell_offsets.clear();
				spatial_grid.cell_offsets.resize(0);
				break;
			default:
				break;
		}
	}
};

// Module Routines
void initialize_mesh_ex_module(ModuleInitializationLevel p_level) {
	// Avoid Running In Project Settings
	if (Engine::get_singleton()->is_project_manager_hint()) {
		return;
	}

	// Register Class
	if (p_level == GDEXTENSION_INITIALIZATION_SCENE) {
		ClassDB::register_class<MeshInstanceEx3D>();
	}
}
void uninitialize_mesh_ex_module(ModuleInitializationLevel p_level) {
}
