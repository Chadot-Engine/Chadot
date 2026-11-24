/**************************************************************************/
/*  splash_screen.cpp                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             CHADOT ENGINE                              */
/*                   https://chadot-engine.github.io/                     */
/**************************************************************************/
/* Copyright (c) 2024-present MemarDesign LLC. (Hamid.Memar)              */
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

#include "splash_screen.h"

#define _CRT_SECURE_NO_WARNINGS

#include <atomic>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <limits>
#include <mutex>
#include <thread>
#include <vector>

#include <png.h>

#define PICOFB_IMPLEMENTATION
#define PICOFB_X11_NO_SHM
#include <thirdparty/picofb/picofb.h>

static std::atomic<bool> splash_stop_requested{ false };
static std::atomic<bool> splash_running{ false };
static std::thread splash_thread;
static std::mutex splash_mutex;

// Splash Loader
static bool _load_splash_image(std::vector<uint32_t> &r_pixels, uint16_t &r_width, uint16_t &r_height) {
	png_image image = {};
	image.version = PNG_IMAGE_VERSION;

	if (!png_image_begin_read_from_memory(&image, splash_screen_png, sizeof(splash_screen_png))) {
		return false;
	}

	if (image.width == 0 || image.height == 0 || image.width > std::numeric_limits<uint16_t>::max() || image.height > std::numeric_limits<uint16_t>::max()) {
		png_image_free(&image);
		return false;
	}

	image.format = PNG_FORMAT_RGBA;

	std::vector<uint8_t> rgba(PNG_IMAGE_SIZE(image));

	if (!png_image_finish_read(&image, nullptr, rgba.data(), 0, nullptr)) {
		png_image_free(&image);
		return false;
	}

	r_width = static_cast<uint16_t>(image.width);
	r_height = static_cast<uint16_t>(image.height);

	png_image_free(&image);

	const size_t pixel_count = static_cast<size_t>(r_width) * r_height;
	r_pixels.resize(pixel_count);

	for (size_t i = 0; i < pixel_count; i++) {
		r_pixels[i] = 0xFF000000 | (static_cast<uint32_t>(rgba[i * 4 + 0]) << 16) | (static_cast<uint32_t>(rgba[i * 4 + 1]) << 8) | rgba[i * 4 + 2];
	}

	return true;
}
static void _splash_window_thread(std::vector<uint32_t> pixels, uint16_t width, uint16_t height) {
#ifdef WINDOWS_ENABLED
	PICOFB_Window window = {};

	if (!PICOFB_init("SplashScreen", width, height, &window)) {
		splash_running.store(false);
		return;
	}

	if (window.frame_buffer) {
		std::memcpy(window.frame_buffer, pixels.data(), pixels.size() * sizeof(uint32_t));
	}

	HWND hwnd = window.dont_touch.hwnd;
	if (hwnd) {
		PICOFB_update(&window);
		ShowWindow(hwnd, SW_SHOWNOACTIVATE);
		UpdateWindow(hwnd);
	}

	while (!splash_stop_requested.load() && !window.quit) {
		PICOFB_update(&window);

		if (window.keyboard[PICOFB_Key_F8]) {
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}

	PICOFB_cleanup(&window);
	splash_running.store(false);
#endif
}

// Shared API
void create_editor_splash_screen() {
#ifdef WINDOWS_ENABLED
	std::lock_guard<std::mutex> lock(splash_mutex);

	if (splash_running.load()) {
		return;
	}

	if (splash_thread.joinable()) {
		splash_thread.join();
	}

	std::vector<uint32_t> pixels;
	uint16_t width = 0;
	uint16_t height = 0;

	if (!_load_splash_image(pixels, width, height)) {
		return;
	}

	splash_stop_requested.store(false);
	splash_running.store(true);

	splash_thread = std::thread(_splash_window_thread, std::move(pixels), width, height);
#endif
}
void destroy_editor_splash_screen() {
#ifdef WINDOWS_ENABLED
	std::lock_guard<std::mutex> lock(splash_mutex);

	splash_stop_requested.store(true);

	if (splash_thread.joinable()) {
		splash_thread.join();
	}

	splash_running.store(false);
#endif
}
