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

#include "backends/graphics/sdl/resvm-sdl-graphics.h"

#include "backends/platform/sdl/sdl-sys.h"
#include "backends/events/sdl/sdl-events.h"
#include "backends/platform/sdl/sdl.h"

#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/file.h"

ResVmSdlGraphicsManager::ResVmSdlGraphicsManager(SdlEventSource *source, SdlWindow *window, const Capabilities &capabilities) :
		SdlGraphicsManager(source, window),
		_capabilities(capabilities)  {
	ConfMan.registerDefault("fullscreen_res", "desktop");
}

ResVmSdlGraphicsManager::~ResVmSdlGraphicsManager() {
}

void ResVmSdlGraphicsManager::activateManager() {
	SdlGraphicsManager::activateManager();

	// Register the graphics manager as a event observer
	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

void ResVmSdlGraphicsManager::deactivateManager() {
	// Unregister the event observer
	if (g_system->getEventManager()->getEventDispatcher()) {
		g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);
	}

	SdlGraphicsManager::deactivateManager();
}

Common::Rect ResVmSdlGraphicsManager::getPreferredFullscreenResolution() {
	// Default to the desktop resolution, unless the user has set a
	// resolution in the configuration file
	const Common::String &fsres = ConfMan.get("fullscreen_res");
	if (fsres != "desktop") {
		uint newW, newH;
		int converted = sscanf(fsres.c_str(), "%ux%u", &newW, &newH);
		if (converted == 2) {
			return Common::Rect(newW, newH);
		} else {
			warning("Could not parse 'fullscreen_res' option: expected WWWxHHH, got %s", fsres.c_str());
		}
	}

	return _window->getDesktopResolution();
}

#pragma mark -
#pragma mark --- Mouse ---
#pragma mark -

bool ResVmSdlGraphicsManager::showMouse(bool visible) {
	SDL_ShowCursor(visible);
	return true;
}

bool ResVmSdlGraphicsManager::lockMouse(bool lock) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	if (lock)
		SDL_SetRelativeMouseMode(SDL_TRUE);
	else
		SDL_SetRelativeMouseMode(SDL_FALSE);
#else
	if (lock)
		SDL_WM_GrabInput(SDL_GRAB_ON);
	else
		SDL_WM_GrabInput(SDL_GRAB_OFF);
#endif
	return true;
}

bool ResVmSdlGraphicsManager::isMouseLocked() const {
#if SDL_VERSION_ATLEAST(2, 0, 0)
	return SDL_GetRelativeMouseMode() == SDL_TRUE;
#else
	return SDL_GrabMode() == SDL_GRAB_ON;
#endif
}

bool ResVmSdlGraphicsManager::notifyEvent(const Common::Event &event) {
	//ResidualVM specific:
	switch ((int)event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.hasFlags(Common::KBD_ALT) && event.kbd.keycode == Common::KEYCODE_s) {
				saveScreenshot();
				return true;
			}
			break;
		case Common::EVENT_KEYUP:
			break;
		default:
			break;
	}

	return false;
}

void ResVmSdlGraphicsManager::notifyVideoExpose() {
	//ResidualVM specific:
	//updateScreen();
}

bool ResVmSdlGraphicsManager::notifyMousePosition(Common::Point &mouse) {
	transformMouseCoordinates(mouse);
	// ResidualVM: not use that:
	//setMousePos(mouse.x, mouse.y);
	return true;
}

void ResVmSdlGraphicsManager::saveScreenshot() {
	OSystem_SDL *g_systemSdl = dynamic_cast<OSystem_SDL*>(g_system);

	if (g_systemSdl) {
		Common::String filename;
		Common::String path = g_systemSdl->getScreenshotsPath();

		// Find unused filename
		int n = 0;
		while (true) {
#ifdef USE_PNG
			filename = Common::String::format("residualvm%05d.png", n);
#else
			filename = Common::String::format("residualvm%05d.bmp", n);
#endif
			SDL_RWops *file = SDL_RWFromFile((path + filename).c_str(), "r");
			if (!file) {
				break;
			}
			SDL_RWclose(file);

			++n;
		}

		if (saveScreenshot(path + filename)) {
			if (path.empty())
				debug("Saved screenshot '%s' in current directory", filename.c_str());
			else
				debug("Saved screenshot '%s' in directory '%s'", filename.c_str(), path.c_str());
		} else {
			if (path.empty())
				warning("Could not save screenshot in current directory");
			else
				warning("Could not save screenshot in directory '%s'", path.c_str());
		}
	}
}
