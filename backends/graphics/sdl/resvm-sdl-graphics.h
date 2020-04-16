/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BACKENDS_GRAPHICS_SDL_RESVMSDLGRAPHICS_H
#define BACKENDS_GRAPHICS_SDL_RESVMSDLGRAPHICS_H

#include "backends/graphics/sdl/sdl-graphics.h"

#include "common/events.h"
#include "common/rect.h"

#include "math/rect2d.h"

class SdlEventSource;

/**
 * Base class for a ResidualVM SDL based graphics manager.
 *
 * Used to share reusable methods between SDL graphics managers
 */
class ResVmSdlGraphicsManager : public SdlGraphicsManager, public Common::EventObserver {
public:
	/**
	 * Capabilities of the current device
	 */
	struct Capabilities {
		/**
		 * Is the device capable of rendering to OpenGL framebuffers
		 */
		bool openGLFrameBuffer;

		/** Supported levels of MSAA when using the OpenGL renderers */
		Common::Array<uint> openGLAntiAliasLevels;

		Capabilities() : openGLFrameBuffer(false) {}
	};

	ResVmSdlGraphicsManager(SdlEventSource *source, SdlWindow *window, const Capabilities &capabilities);
	~ResVmSdlGraphicsManager() override;

	// SdlGraphicsManager API
	void activateManager() override;
	void deactivateManager() override;
	void notifyVideoExpose() override;
	bool notifyMousePosition(Common::Point &mouse) override;

	// GraphicsManager API - Features
	void setFeatureState(OSystem::Feature f, bool enable) override;
	bool getFeatureState(OSystem::Feature f) const override;

	// GraphicsManager API - Graphics mode
#ifdef USE_RGB_COLOR
	Graphics::PixelFormat getScreenFormat() const override { return _screenFormat; }
#endif
	int getScreenChangeID() const override { return _screenChangeCount; }

public:
	// GraphicsManager API - Draw methods
	void saveScreenshot() override;

	// GraphicsManager API - Overlay
	Graphics::PixelFormat getOverlayFormat() const override { return _overlayFormat; }

	// GraphicsManager API - Mouse
	bool showMouse(bool visible) override;
	bool lockMouse(bool lock) override; // ResidualVM specific method

	// Common::EventObserver API
	bool notifyEvent(const Common::Event &event) override;

	/**
	 * Checks if mouse is locked or not.
	 * Avoid to emulate a mouse movement from joystick if locked.
	 */
	bool isMouseLocked() const;

protected:
	const Capabilities &_capabilities;

	bool _fullscreen;
	bool _lockAspectRatio;
	uint _engineRequestedWidth, _engineRequestedHeight;

	int _screenChangeCount;

	bool _overlayVisible;
	Graphics::PixelFormat _overlayFormat;

#ifdef USE_RGB_COLOR
	Graphics::PixelFormat _screenFormat;
#endif

	/** Obtain the user configured fullscreen resolution, or default to the desktop resolution */
	Common::Rect getPreferredFullscreenResolution();

	/** Save a screenshot to the specified file */
	virtual bool saveScreenshot(const Common::String &file) const  = 0;
};

#endif
