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

#include "core/string/regex.h"
#include "core/config/engine.h"
#include "core/object/class_db.h"
#include "core/object/callable_mp.h"
#include "core/io/file_access.h"
#include "core/io/dir_access.h"

#include "scene/gui/label.h"
#include "scene/gui/box_container.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/spin_box.h"
#include "scene/gui/check_button.h"

#include "scene/3d/node_3d.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/light_3d.h"
#include "scene/3d/skeleton_3d.h"
#include "scene/resources/animation_library.h"
#include "scene/animation/animation_player.h"

#include <cstring>
#include <functional>

#ifdef TOOLS_ENABLED
#include "editor/gui/editor_file_dialog.h"
#include "editor/editor_node.h"
#include "editor/themes/editor_scale.h"
#endif

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

// JenovaFrameImporter Implementation
class JenovaFrameImporter : public Object {
	GDCLASS(JenovaFrameImporter, Object);

protected:
	static void _bind_methods() {
		// Bind Methods
		ClassDB::bind_method(D_METHOD("open_import_dialog"), &JenovaFrameImporter::open_import_dialog);
		ClassDB::bind_method(D_METHOD("import_animation_sequence", "sequence_path", "framerate", "validate"), &JenovaFrameImporter::import_animation_sequence);
	}

public:
	void open_import_dialog() {
#ifdef TOOLS_ENABLED
		EditorFileDialog *dialog = memnew(EditorFileDialog);
		dialog->set_file_mode(EditorFileDialog::FILE_MODE_OPEN_FILE);
		dialog->set_access(EditorFileDialog::ACCESS_FILESYSTEM);
		dialog->set_title("Select Jenova Frame Sequence to Import");
		dialog->add_filter("*.jframe, *.jnvframe, *.jmotion ; Jenova Frame Data");

		// Options User Interface
		float scale = EditorScale::get_scale();

		MarginContainer *marg = memnew(MarginContainer);
		marg->add_theme_constant_override("margin_left", (int)(8 * scale));
		marg->add_theme_constant_override("margin_right", (int)(8 * scale));
		marg->add_theme_constant_override("margin_top", (int)(6 * scale));
		marg->add_theme_constant_override("margin_bottom", (int)(6 * scale));

		VBoxContainer *vbox = memnew(VBoxContainer);
		vbox->set_h_size_flags(Control::SIZE_SHRINK_BEGIN);
		marg->add_child(vbox);

		HBoxContainer *options_row = memnew(HBoxContainer);
		options_row->set_h_size_flags(Control::SIZE_SHRINK_BEGIN);
		options_row->add_theme_constant_override("separation", (int)(12 * scale));

		Label *fps_label = memnew(Label);
		fps_label->set_text("Framerate:");
		options_row->add_child(fps_label);

		SpinBox *fps_spin = memnew(SpinBox);
		fps_spin->set_min(1);
		fps_spin->set_max(1000);
		fps_spin->set_step(1);
		fps_spin->set_value(60);
		fps_spin->set_custom_minimum_size(Size2(80 * scale, 0));
		fps_spin->set_name("FramerateInput");
		options_row->add_child(fps_spin);

		Label *validate_label = memnew(Label);
		validate_label->set_text("Validate Nodes and Data");
		options_row->add_child(validate_label);

		CheckButton *validate_check = memnew(CheckButton);
		validate_check->set_name("ValidateNodesInput");
		validate_check->set_pressed(true);
		options_row->add_child(validate_check);

		vbox->add_child(options_row);

		dialog->get_vbox()->add_child(marg);

		// Bind File Selection Signal
		dialog->connect("file_selected", callable_mp(this, &JenovaFrameImporter::animation_sequence_selected).bind(fps_spin, validate_check, dialog));

		// Show Import Dialog
		EditorNode::get_singleton()->get_gui_base()->add_child(dialog);
		dialog->popup_centered_ratio(0.6f);
#endif
	}

private:
	void animation_sequence_selected(const String &seq_path, SpinBox *fps, CheckButton *validate, EditorFileDialog *dlg)
	{
		import_animation_sequence(seq_path, fps->get_value(), validate->is_pressed());
		dlg->queue_free();
	}
	void import_animation_sequence(const String &seq_path, float framerate, bool validate = false) {
#ifdef TOOLS_ENABLED
		Node *scene_root = EditorNode::get_singleton()->get_edited_scene();
		ERR_FAIL_NULL_MSG(scene_root, "Open a scene before importing Jenova Frame Sequence.");

		// Sequence Detection
		Ref<RegEx> seq_re = RegEx::create_from_string("^(.*?)(\\d+)$");
		Ref<RegExMatch> sel = seq_re->search(seq_path.get_file().get_basename());
		ERR_FAIL_COND_MSG(sel.is_null(), "File does not follow '<prefix>_<digits>.jframe': " + seq_path.get_file());

		const String seq_prefix = sel->get_string(1);
		const int seq_digits = sel->get_string(2).length();
		const String seq_ext = seq_path.get_extension();
		const String seq_dir = seq_path.get_base_dir();

		Vector<String> sequence;
		{
			Ref<DirAccess> da = DirAccess::open(seq_dir);
			ERR_FAIL_COND_MSG(da.is_null(), "Cannot open directory: " + seq_dir);
			da->list_dir_begin();
			for (String f = da->get_next(); !f.is_empty(); f = da->get_next()) {
				if (da->current_is_dir() || f.get_extension() != seq_ext) {
					continue;
				}
				Ref<RegExMatch> m = seq_re->search(f.get_basename());
				if (m.is_valid() && m->get_string(1) == seq_prefix && m->get_string(2).length() == seq_digits) {
					sequence.push_back(seq_dir.path_join(f));
				}
			}
			da->list_dir_end();
		}
		ERR_FAIL_COND_MSG(sequence.is_empty(), "No frames matching the sequence were found.");
		sequence.sort();

		// Read & Parse Frames
		struct Frame {
			uint32_t index = 0;
			Vector<NodeState> entities;
		};
		auto read_frame = [](const String &p, Frame &out) -> bool {
			Ref<FileAccess> f = FileAccess::open(p, FileAccess::READ);
			if (f.is_null()) {
				return false;
			}
			CaptureDataHeader h;
			if (f->get_buffer((uint8_t *)&h, sizeof(h)) != sizeof(h)) {
				return false;
			}
			if (memcmp(h.magicNumber, "JNVCAP3D", 8) != 0) {
				ERR_PRINT("Selected frame sequence is not a valid Jenova Frame Sequence.");
				return false;
			}
			out.index = h.frameIndex;
			out.entities.resize(h.entityCount);
			if (h.entityCount > 0) {
				size_t bytes = sizeof(NodeState) * h.entityCount;
				if (f->get_buffer((uint8_t *)out.entities.ptrw(), bytes) != bytes) {
					return false;
				}
			}
			return true;
		};

		Vector<Frame> frames;
		for (const String &f : sequence) {
			Frame fd;
			if (read_frame(f, fd)) {
				frames.push_back(fd);
			} else {
				WARN_PRINT("Skipping malformed frame: " + f);
			}
		}
		ERR_FAIL_COND_MSG(frames.is_empty(), "No valid frames could be read.");
		for (int i = 1; i < frames.size(); i++) {
			Frame key = frames[i];
			int j = i - 1;
			while (j >= 0 && frames[j].index > key.index) {
				frames.write[j + 1] = frames[j];
				j--;
			}
			frames.write[j + 1] = key;
		}

		// Index Scene Nodes by Name
		HashMap<StringName, Node *> name_to_node;
		HashMap<StringName, int> name_count;
		{
			std::function<void(Node *)> walk = [&](Node *n) {
				StringName nm = n->get_name();
				if (name_to_node.has(nm)) {
					name_count[nm] += 1;
				} else {
					name_to_node[nm] = n;
					name_count[nm] = 1;
				}
				for (int i = 0; i < n->get_child_count(); i++) {
					walk(n->get_child(i));
				}
			};
			walk(scene_root);
		}

		// Index Bones By Name
		struct BoneRef {
			Skeleton3D *skel = nullptr;
			int bone = 0;
		};
		HashMap<StringName, BoneRef> bone_map;
		HashMap<StringName, int> bone_count;
		{
			std::function<void(Node *)> walk = [&](Node *n) {
				if (Skeleton3D *sk = Object::cast_to<Skeleton3D>(n)) {
					for (int i = 0; i < sk->get_bone_count(); i++) {
						StringName bn = sk->get_bone_name(i);
						if (bone_map.has(bn)) {
							bone_count[bn] += 1;
						} else {
							bone_map[bn] = BoneRef{ sk, i };
							bone_count[bn] = 1;
						}
					}
				}
				for (int i = 0; i < n->get_child_count(); i++) {
					walk(n->get_child(i));
				}
			};
			walk(scene_root);
		}

		// Validation Pass
		if (validate) {
			HashSet<StringName> reported;
			for (const NodeState &ns : frames[0].entities) {
				StringName nm = String::utf8(ns.nodeName);
				if (nm.is_empty() || reported.has(nm)) {
					continue;
				}

				if (ns.nodeType == NodeType::NODE_BONE) {
					if (!bone_map.has(nm)) {
						reported.insert(nm);
						WARN_PRINT("Captured bone not found in scene: " + String(nm));
					} else if (bone_count[nm] > 1) {
						reported.insert(nm);
						WARN_PRINT(vformat("Bone name '%s' appears %d times in scene, only first animated.", String(nm), bone_count[nm]));
					}
				} else {
					if (!name_to_node.has(nm)) {
						reported.insert(nm);
						WARN_PRINT("Captured node not found in scene: " + String(nm));
					} else if (name_count[nm] > 1) {
						reported.insert(nm);
						WARN_PRINT(vformat("Node name '%s' appears %d times in scene, only first animated.", String(nm), name_count[nm]));
					}
				}
			}
		}

		// Find or Create Animation Player at Scene Root
		AnimationPlayer *ap = nullptr;
		for (int i = 0; i < scene_root->get_child_count(); i++) {
			if (AnimationPlayer *cand = Object::cast_to<AnimationPlayer>(scene_root->get_child(i))) {
				ap = cand;
				break;
			}
		}
		if (!ap) {
			ap = memnew(AnimationPlayer);
			ap->set_name("AnimationPlayer");
			scene_root->add_child(ap);
			ap->set_owner(scene_root);
			ap->set_root_node(NodePath(".."));
		}
		Node *ap_root = ap->get_node_or_null(ap->get_root_node());
		if (!ap_root) {
			ap_root = scene_root;
		}

		// Prepare Animation Library
		Ref<AnimationLibrary> lib;
		if (!ap->has_animation_library("")) {
			lib.instantiate();
			ap->add_animation_library("", lib);
		} else {
			lib = ap->get_animation_library("");
		}

		StringName base_name = seq_prefix.is_empty() ? StringName("JenovaMotion") : StringName(seq_prefix);
		StringName anim_name = base_name.string().rstrip("_");
		if (lib->has_animation(anim_name)) {
			int n = 1;
			while (lib->has_animation(StringName(String(base_name) + "_" + itos(n)))) {
				n++;
			}
			anim_name = StringName(String(base_name) + "_" + itos(n));
		}

		Ref<Animation> anim;
		anim.instantiate();
		anim->set_loop_mode(Animation::LOOP_NONE);
		anim->set_length(0.0);

		HashMap<String, int> track_cache;
		auto get_track = [&](const String &key, Animation::TrackType type, const NodePath &path) -> int {
			int *found = track_cache.getptr(key);
			if (found) {
				return *found;
			}
			int t = anim->add_track(type);
			anim->track_set_path(t, path);
			track_cache[key] = t;
			return t;
		};

		// Process Sequence Frames
		const uint32_t first_index = frames[0].index;
		for (const Frame &fr : frames) {
			const float t = float(fr.index - first_index) / framerate;

			// Collect Captured Non-Bone Global Transform for the Frame
			HashMap<StringName, Transform3D> globals;
			for (const NodeState &ns : fr.entities) {
				if (ns.nodeType == NodeType::NODE_BONE) {
					continue;
				}
				StringName nm = String::utf8(ns.nodeName);
				if (nm.is_empty() || globals.has(nm)) {
					continue;
				}
				globals[nm] = ns.nodeTransformation;
			}

			// Animate Generic Nodes
			for (const NodeState &ns : fr.entities) {
				if (ns.nodeType == NodeType::NODE_BONE) {
					continue;
				}
				StringName nm = String::utf8(ns.nodeName);
				if (nm.is_empty()) {
					continue;
				}

				Node **np = name_to_node.getptr(nm);
				if (!np) {
					continue;
				}
				Node *node = *np;
				Node3D *n3d = Object::cast_to<Node3D>(node);
				if (!n3d) {
					continue;
				}

				Transform3D local_xform;
				if (ns.transformMode == TransformMode::TRANSFORM_GLOBAL) {
					Node3D *par = n3d->get_parent_node_3d();
					if (par) {
						Transform3D par_g;
						if (Transform3D *pg = globals.getptr(par->get_name())) {
							par_g = *pg;
						} else {
							par_g = par->get_global_transform();
						}
						local_xform = par_g.affine_inverse() * ns.nodeTransformation;
					} else {
						local_xform = ns.nodeTransformation;
					}
				} else {
					// Was Already Local
					local_xform = ns.nodeTransformation; 
				}

				NodePath node_path = ap_root->get_path_to(node);
				String base = String(node_path);

				int tp = get_track(base + "|p", Animation::TYPE_POSITION_3D, node_path);
				int tr = get_track(base + "|r", Animation::TYPE_ROTATION_3D, node_path);
				int ts = get_track(base + "|s", Animation::TYPE_SCALE_3D, node_path);
				anim->position_track_insert_key(tp, t, local_xform.origin);
				anim->rotation_track_insert_key(tr, t, local_xform.basis.get_rotation_quaternion());
				anim->scale_track_insert_key(ts, t, local_xform.basis.get_scale());

				if (ns.nodeType == NodeType::NODE_CAMERA && Object::cast_to<Camera3D>(node)) {
					int tf = get_track(base + "|fov", Animation::TYPE_VALUE, NodePath(base + ":fov"));
					anim->track_insert_key(tf, t, ns.extraParameters[0]);
				} else if (ns.nodeType == NodeType::NODE_LIGHT && Object::cast_to<Light3D>(node)) {
					int te = get_track(base + "|energy", Animation::TYPE_VALUE, NodePath(base + ":light_energy"));
					anim->track_insert_key(te, t, ns.extraParameters[0]);
				}
			}

			// Anime Bones
			HashMap<StringName, Transform3D> bone_globals;
			for (const NodeState &ns : fr.entities) {
				if (ns.nodeType != NodeType::NODE_BONE) {
					continue;
				}
				StringName bn = String::utf8(ns.nodeName);
				if (bn.is_empty() || bone_globals.has(bn)) {
					continue;
				}
				BoneRef *br = bone_map.getptr(bn);
				if (!br) {
					continue;
				}

				Transform3D skel_global;
				if (Transform3D *sg = globals.getptr(br->skel->get_name())) {
					skel_global = *sg;
				} else {
					skel_global = br->skel->get_global_transform();
				}

				bone_globals[bn] = skel_global.affine_inverse() * ns.nodeTransformation;
			}

			for (const NodeState &ns : fr.entities) {
				if (ns.nodeType != NodeType::NODE_BONE) {
					continue;
				}
				StringName bn = String::utf8(ns.nodeName);
				BoneRef *br = bone_map.getptr(bn);
				Transform3D *bg = bone_globals.getptr(bn);
				if (!br || !bg) {
					continue;
				}

				Transform3D local_pose = *bg;
				int parent_idx = br->skel->get_bone_parent(br->bone);
				if (parent_idx >= 0) {
					StringName parent_bn = br->skel->get_bone_name(parent_idx);
					if (Transform3D *pg = bone_globals.getptr(parent_bn)) {
						local_pose = pg->affine_inverse() * (*bg);
					}
				}

				String skel_path = String(ap_root->get_path_to(br->skel));
				String bone_str = String(bn);
				NodePath track_path = NodePath(skel_path + ":" + bone_str);
				String cache_base = skel_path + "|bone:" + bone_str;

				int tp = get_track(cache_base + "|p", Animation::TYPE_POSITION_3D, track_path);
				int tr = get_track(cache_base + "|r", Animation::TYPE_ROTATION_3D, track_path);
				int ts = get_track(cache_base + "|s", Animation::TYPE_SCALE_3D, track_path);
				anim->position_track_insert_key(tp, t, local_pose.origin);
				anim->rotation_track_insert_key(tr, t, local_pose.basis.get_rotation_quaternion());
				anim->scale_track_insert_key(ts, t, local_pose.basis.get_scale());
			}

			anim->set_length(t);
		}

		// Commit Animation
		lib->add_animation(anim_name, anim);
		ap->set_current_animation(anim_name);

		// Verbose
		print_line(vformat("Imported %d motion frames and %d tracks from '%s'.", frames.size(), anim->get_track_count(), String(anim_name)));
#endif
	}

public:
	static inline JenovaFrameImporter *singleton = nullptr;
};

// Module Routines
void initialize_game_recorder_3d_module(ModuleInitializationLevel p_level) {
	// Avoid Running In Project Settings
	if (Engine::get_singleton()->is_project_manager_hint()) {
		return;
	}

	// Register Objects
	if (p_level == GDEXTENSION_INITIALIZATION_SCENE) {
		// Register Classes
		ClassDB::register_class<GameRecorder3D>();

	} else if (p_level == GDEXTENSION_INITIALIZATION_EDITOR) {
		// Register Editor Classes
		ClassDB::register_class<JenovaFrameImporter>();

		// Register Editor Singletons
		JenovaFrameImporter::singleton = memnew(JenovaFrameImporter);
		Engine::get_singleton()->add_singleton(Engine::Singleton("JenovaFrameImporter", JenovaFrameImporter::singleton));
	}

}
void uninitialize_game_recorder_3d_module(ModuleInitializationLevel p_level) {

	// Unregister Objects
	if (p_level == GDEXTENSION_INITIALIZATION_EDITOR) {
		// Unregister Editor Singletons
		if (JenovaFrameImporter::singleton) {
			Engine::get_singleton()->remove_singleton("JenovaFrameImporter");
			memdelete(JenovaFrameImporter::singleton);
			JenovaFrameImporter::singleton = nullptr;
		}
	}
}
