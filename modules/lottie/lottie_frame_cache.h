/**************************************************************************/
/*  lottie_fame_cache.h                                                   */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             CHADOT ENGINE                              */
/*                    https://chadotengine.github.io                      */
/**************************************************************************/
/* Copyright (c) 2024-present MemarDesign LLC.                            */
/* Copyright (c) 2025 Godot Lottie Contributors.                          */
/**************************************************************************/

#ifndef LOTTIE_FRAME_CACHE_H
#define LOTTIE_FRAME_CACHE_H

#include "core/io/image.h"
#include "core/string/ustring.h"
#include "scene/resources/image_texture.h"

#include <list>
#include <unordered_map>

class LottieFrameCache {
public:
	static LottieFrameCache *get_singleton();
	static void clear_singleton();

	Ref<ImageTexture> get(const String &anim_key, int frame, const Vector2i &size);
	void put(const String &anim_key, int frame, const Vector2i &size, const Ref<ImageTexture> &tex, size_t bytes);
	void set_capacity_bytes(size_t bytes);
	void clear();

private:
	struct Key {
		String anim;
		int frame;
		int w;
		int h;
		bool operator==(const Key &o) const {
			return frame == o.frame && w == o.w && h == o.h && anim == o.anim;
		}
		struct Hasher {
			size_t operator()(const Key &k) const {
				std::hash<std::string> hs;
				size_t h1 = hs(std::string(k.anim.utf8().get_data()));
				size_t h2 = ((size_t)k.frame << 1) ^ ((size_t)k.w << 17) ^ ((size_t)k.h << 29);
				return h1 ^ h2;
			}
		};
	};

	struct Entry {
		Ref<ImageTexture> tex;
		size_t bytes = 0;
		std::list<Key>::iterator lru_it;
	};

	std::unordered_map<Key, Entry, Key::Hasher> _map;
	std::list<Key> _lru;
	size_t _capacity = 256 * 1024 * 1024;
	size_t _used = 0;

	void _touch(const Key &key);
	void _evict_if_needed();
};

#endif
