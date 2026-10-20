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
#include "core/io/file_access.h"
#include "scene/3d/node_3d.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/skeleton_3d.h"

// GameRecorder3D Types
enum CaptureMode : uint8_t
{
	CAPTURE_DISABLED 			= 0,
	CAPTURE_RENDER_SYNC 		= 1,
	CAPTURE_PHYSICS_SYNC 		= 2
};
VARIANT_ENUM_CAST(CaptureMode);
enum CacheMode : uint8_t
{
	CACHE_INMEMORY 				= 0,
	CACHE_ONDISK 				= 1,
};
VARIANT_ENUM_CAST(CacheMode);
enum TransformMode : uint8_t
{
	TRANSFORM_LOCAL 			= 0,
	TRANSFORM_GLOBAL 			= 1,
};
enum NodeType : uint8_t
{
	NODE_GENERIC 				= 0,
	NODE_CAMERA 				= 1,
	NODE_LIGHT 					= 2,
	NODE_BONE 					= 3,
};
struct CaptureDataHeader
{
	char magicNumber[8]			= {'J', 'N', 'V', 'C', 'A', 'P', '3', 'D'};
	uint32_t frameIndex			= 0;
	uint32_t entityCount		= 0;
	unsigned char reserved[16]	= {0};
};
struct NodeState {
	char nodeName[128] = { 0 };
	Transform3D nodeTransformation;
	TransformMode transformMode = TransformMode::TRANSFORM_GLOBAL;
	NodeType nodeType = NodeType::NODE_GENERIC;
	float extraParameters[4];
	unsigned char reserved[60] = { 0 };
};
struct CaptureBuffer {
	void *ptr;
	size_t size;
};

// GameRecorder3D Implementation
class GameRecorder3D : public Node3D {
	GDCLASS(GameRecorder3D, Node3D);

private:
	CaptureMode captureMode = CaptureMode::CAPTURE_DISABLED;
	CacheMode cacheMode = CacheMode::CACHE_ONDISK;
	String outputDirectory = "res://";
	bool captureTargetList = false;
	bool captureChildren = false;

protected:
	static void _bind_methods() {
		// Capture Mode
		BIND_ENUM_CONSTANT(CaptureMode::CAPTURE_DISABLED);
		BIND_ENUM_CONSTANT(CaptureMode::CAPTURE_RENDER_SYNC);
		BIND_ENUM_CONSTANT(CaptureMode::CAPTURE_PHYSICS_SYNC);
		ClassDB::bind_method(D_METHOD("set_capture_mode", "mode"), &GameRecorder3D::set_capture_mode);
		ClassDB::bind_method(D_METHOD("get_capture_mode"), &GameRecorder3D::get_capture_mode);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "capture_mode", PROPERTY_HINT_ENUM, "Disabled,Render,Physics"), "set_capture_mode", "get_capture_mode");

		// Cache Mode
		BIND_ENUM_CONSTANT(CacheMode::CACHE_INMEMORY);
		BIND_ENUM_CONSTANT(CacheMode::CACHE_ONDISK);
		ClassDB::bind_method(D_METHOD("set_cache_mode", "mode"), &GameRecorder3D::set_cache_mode);
		ClassDB::bind_method(D_METHOD("get_cache_mode"), &GameRecorder3D::get_cache_mode);
		ADD_PROPERTY(PropertyInfo(Variant::INT, "cache_mode", PROPERTY_HINT_ENUM, "In-Memory,On Disk"), "set_cache_mode", "get_cache_mode");

		// Output Directory
		ClassDB::bind_method(D_METHOD("set_output_directory", "path"), &GameRecorder3D::set_output_directory);
		ClassDB::bind_method(D_METHOD("get_output_directory"), &GameRecorder3D::get_output_directory);
		ADD_PROPERTY(PropertyInfo(Variant::STRING, "output_directory", PROPERTY_HINT_DIR), "set_output_directory", "get_output_directory");

		// Capture Flags
		ClassDB::bind_method(D_METHOD("set_capture_target_list", "enabled"), &GameRecorder3D::set_capture_target_list);
		ClassDB::bind_method(D_METHOD("get_capture_target_list"), &GameRecorder3D::get_capture_target_list);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "capture_target_list"), "set_capture_target_list", "get_capture_target_list");
		ClassDB::bind_method(D_METHOD("set_capture_children", "enabled"), &GameRecorder3D::set_capture_children);
		ClassDB::bind_method(D_METHOD("get_capture_children"), &GameRecorder3D::get_capture_children);
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "capture_children"), "set_capture_children", "get_capture_children");

		// Bind Methods
		ClassDB::bind_method(D_METHOD("write_collected_frames"), &GameRecorder3D::write_collected_frames);
	}

public:
	void set_capture_mode(CaptureMode mode) {
		captureMode = mode;
	}
	CaptureMode get_capture_mode() const {
		return captureMode;
	}
	void set_cache_mode(CacheMode mode) {
		cacheMode = mode;
	}
	CacheMode get_cache_mode() const {
		return cacheMode;
	}
	void set_output_directory(const String &path) {
		outputDirectory = path;
	}
	String get_output_directory() const {
		return outputDirectory;
	}
	void set_capture_target_list(bool enabled) {
		captureTargetList = enabled;
	}
	bool get_capture_target_list() const {
		return captureTargetList;
	}
	void set_capture_children(bool enabled) {
		captureChildren = enabled;
	}
	bool get_capture_children() const {
		return captureChildren;
	}

public:
	void collect_target_nodes() {
		targetNodes.clear();
		if (captureChildren) {
			collect_children_recursive(this);
		}
	}
	void collect_children_recursive(Node *parent) {
		for (int i = 0; i < parent->get_child_count(); ++i) {
			Node *child = parent->get_child(i);
			Node3D *node3d = Object::cast_to<Node3D>(child);
			if (node3d && !targetNodes.has(node3d)) {
				targetNodes.push_back(node3d);
			}
			collect_children_recursive(child);
		}
	}
	void set_computed_node_transformation(const Node3D *node3D, NodeState &state) {
		state.nodeTransformation = (state.transformMode == TRANSFORM_GLOBAL) ? node3D->get_global_transform() : node3D->get_transform();
	}
	void capture_frame(double delta) {
		// Create Header
		CaptureDataHeader header;
		header.frameIndex = frameNumber;

		// Create Node State Database
		Vector<NodeState> nodeStates;

		// Capture Target Nodes
		for (const Node3D *node : targetNodes) {

			// Validate (For Future Use)
			if (!node) {
				continue;
			}
			if (node->is_class("XRNode3D")) {
				continue;
			}

			// Create Node State
			NodeState nodeState;

			// Assign Name
			sprintf_s(nodeState.nodeName, sizeof nodeState.nodeName, "%s", node->get_name().string().utf8().get_data());

			// Assign Transformation
			nodeState.transformMode = TransformMode::TRANSFORM_GLOBAL;
			nodeState.nodeType = NodeType::NODE_GENERIC;
			set_computed_node_transformation(node, nodeState);

			// Set Special Parameters
			if (node->is_class("Camera3D")) {
				const Camera3D *camera = static_cast<const Camera3D *>(node);
				nodeState.nodeType = NodeType::NODE_CAMERA;
				nodeState.extraParameters[0] = camera->get_fov();
				nodeState.extraParameters[1] = camera->get_near();
				nodeState.extraParameters[2] = camera->get_far();
			}

			// Add Node State
			nodeStates.push_back(nodeState);

			// Increase Entity Count
			header.entityCount++;

			// Capture Bones
			if (node->is_class("Skeleton3D")) {
				const Skeleton3D *skeleton = static_cast<const Skeleton3D *>(node);
				int boneCount = skeleton->get_bone_count();

				for (int i = 0; i < boneCount; ++i) {
					NodeState boneState;

					// Assign Bone Name
					sprintf_s(boneState.nodeName, sizeof boneState.nodeName, "%s", skeleton->get_bone_name(i).utf8().get_data());

					// Assign Local Transform
					boneState.transformMode = TransformMode::TRANSFORM_GLOBAL;
					boneState.nodeType = NodeType::NODE_BONE;
					if (boneState.transformMode == TRANSFORM_GLOBAL) {
						Transform3D bone_local = skeleton->get_bone_global_pose(i);
						Transform3D skeleton_global = skeleton->get_global_transform();
						boneState.nodeTransformation = skeleton_global * bone_local;
					} else {
						boneState.nodeTransformation = skeleton->get_bone_pose(i);
					}

					// Add Bone State
					nodeStates.push_back(boneState);

					// Increase Entity Count
					header.entityCount++;
				}
			}
		}

		// Create Frame Cache
		CaptureBuffer captureBuffer;

		// Calculate Sizes
		size_t headerSize = sizeof(CaptureDataHeader);
		size_t nodeBlockSize = header.entityCount * sizeof(NodeState);
		size_t totalSize = headerSize + nodeBlockSize;

		// Allocate Buffer
		captureBuffer.ptr = memalloc(totalSize);
		captureBuffer.size = totalSize;

		// Write Capture Buffer Data
		memcpy(captureBuffer.ptr, &header, headerSize);
		memcpy(static_cast<char *>(captureBuffer.ptr) + headerSize, nodeStates.ptr(), nodeBlockSize);

		// Cache Frame
		if (cacheMode == CacheMode::CACHE_INMEMORY) {
			memoryCaptureBuffer.push_back(captureBuffer);
		}
		if (cacheMode == CacheMode::CACHE_ONDISK) {
			String filePath = outputDirectory.path_join(vformat("%s_%05d.jframe", this->get_name(), frameNumber));
			Ref<FileAccess> file = FileAccess::open(filePath, FileAccess::WRITE);
			if (file.is_valid()) {
				file->store_buffer(static_cast<const uint8_t *>(captureBuffer.ptr), captureBuffer.size);
			}

			// Release Buffer
			memfree(captureBuffer.ptr);
		}

		// Increase Frame
		frameNumber++;
	}
	void write_collected_frames() {

		// Write Memory Cache to Disk
		if (cacheMode == CacheMode::CACHE_INMEMORY) {
			for (int i = 0; i < memoryCaptureBuffer.size(); ++i) {
				const CaptureBuffer &buffer = memoryCaptureBuffer[i];
				String filePath = outputDirectory.path_join(vformat("%s_%05d.jframe", this->get_name(), i));
				Ref<FileAccess> file = FileAccess::open(filePath, FileAccess::WRITE);
				if (file.is_valid()) {
					file->store_buffer(static_cast<const uint8_t *>(buffer.ptr), buffer.size);
				}

				// Release buffer
				memfree(buffer.ptr);
			}
		}

		// Clear Buffers
		if (memoryCaptureBuffer.size() != 0) {
			memoryCaptureBuffer.clear();
		}
	}

	// Handle Routines
	void _notification(int32_t p_what) {
		if (Engine::get_singleton()->is_editor_hint()) {
			return;
		}
		switch (p_what) {
			case NOTIFICATION_READY: {
				if (captureMode == CaptureMode::CAPTURE_RENDER_SYNC) {
					set_process(true);
				}
				if (captureMode == CaptureMode::CAPTURE_PHYSICS_SYNC) {
					set_physics_process(true);
				}
				collect_target_nodes();
			} break;
			case NOTIFICATION_EXIT_TREE: {
				write_collected_frames();
			} break;
			case NOTIFICATION_PROCESS: {
				if (captureMode == CaptureMode::CAPTURE_RENDER_SYNC) {
					capture_frame(get_process_delta_time());
				}
			} break;
			case NOTIFICATION_PHYSICS_PROCESS: {
				if (captureMode == CaptureMode::CAPTURE_PHYSICS_SYNC) {
					capture_frame(get_physics_process_delta_time());
				}
			} break;
			default:
				break;
		}
	}

private:
	size_t frameNumber = 0;
	Vector<Node3D *> targetNodes;
	Vector<CaptureBuffer> memoryCaptureBuffer;
};


// Module Routines
void initialize_game_recorder_3d_module(ModuleInitializationLevel p_level) {
	// Avoid Running In Project Settings
	if (Engine::get_singleton()->is_project_manager_hint()) {
		return;
	}

	// Register Class
	if (p_level == GDEXTENSION_INITIALIZATION_SCENE) {
		ClassDB::register_class<GameRecorder3D>();
	}
}
void uninitialize_game_recorder_3d_module(ModuleInitializationLevel p_level) {
}
