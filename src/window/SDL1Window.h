/*
 * Copyright 2011-2020 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_WINDOW_SDL1WINDOW_H
#define ARX_WINDOW_SDL1WINDOW_H

#include <SDL.h>

#include "platform/Platform.h"
#include "window/RenderWindow.h"

class SDL1InputBackend;

class SDL1Window final : public RenderWindow {
	
public:
	
	SDL1Window();
	~SDL1Window() override;
	
	bool initializeFramework() override;
	void setTitle(const std::string & title) override;
	bool setVSync(int vsync) override;
	void setFullscreenMode(const DisplayMode & mode) override;
	void setWindowSize(const Vec2i & size) override;
	bool setGamma(float gamma = 1.f) override;
	bool initialize() override;
	void processEvents(bool waitForEvent) override;
	
	void showFrame() override;
	
	void hide() override;
	
	void setMinimizeOnFocusLost(bool enabled) override;
	MinimizeSetting willMinimizeOnFocusLost() override;
	
	std::string getClipboardText() override;
	void setClipboardText(const std::string & text) override;
	
	void allowScreensaver(bool allowed) override;
	
	InputBackend * getInputBackend() override;
	
private:
	
	bool setMode(DisplayMode mode, bool fullscreen);
	void changeMode(DisplayMode mode, bool fullscreen);
	void updateSize(bool force = false);
	
	void restoreGamma();
	
	static int SDLCALL eventFilter(const SDL_Event * event);
	
	bool m_initialized;
	DisplayMode m_desktopMode;
	
	SDL1InputBackend * m_input;
	
	float m_gamma;
	bool m_gammaOverridden;
	u16 m_gammaRed[256];
	u16 m_gammaGreen[256];
	u16 m_gammaBlue[256];
	
	static SDL1Window * s_mainWindow;
	
};

#endif // ARX_WINDOW_SDL1WINDOW_H
