#include "events/input/win32KeyDown.h"

#include "core/window.h"

//-------------------------------------------------------------------------
rex::win32::KeyDown::KeyDown(rex::Window* window, rex::win32::MessageParameters params)
	: rex::events::KeyDown(window, (const char*)&params.wparam)
	, m_key_state(params.lparam)
{}

//-------------------------------------------------------------------------
rex::win32::KeyDown::~KeyDown() = default;
