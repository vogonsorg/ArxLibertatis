/*
 * Copyright 2015-2019 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_GUI_WIDGET_TEXTINPUTWIDGET_H
#define ARX_GUI_WIDGET_TEXTINPUTWIDGET_H

#include <functional>
#include <limits>
#include <string_view>

#include "graphics/Color.h"
#include "gui/widget/Widget.h"
#include "input/TextInput.h"
#include "platform/Platform.h"

class Font;

class TextInputWidget final : public Widget, public BasicTextInput {
	
	Font * m_font;
	bool m_editing;
	size_t m_maxLength;
	
	bool keyPressed(Keyboard::Key key, KeyModifiers mod) override;
	void textUpdated() override;
	
public:
	
	std::function<void(TextInputWidget * /* widget */)> unfocused;
	
	TextInputWidget(Font * font, std::string_view text, const Rectf & rect);
	
	bool click() override;
	
	bool doubleClick() override;
	
	void render(bool mouseOver = false) override;
	
	bool wantFocus() const override { return m_editing; }
	
	void unfocus() override;
	
	void setMaxLength(size_t length = std::numeric_limits<size_t>::max()) { m_maxLength = length; }
	
	Font * font() const { return m_font; }
	
	WidgetType type() const override {
		return WidgetType_TextInput;
	}
	
};

#endif // ARX_GUI_WIDGET_TEXTINPUTWIDGET_H
