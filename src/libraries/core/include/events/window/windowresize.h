#pragma once

#include "events/window/windowevent.h"

namespace rex
{
	namespace events
	{
		class WindowResize : public WindowEvent
		{
		public:
			EVENT_CLASS_TYPE(EventType::WINDOW_RESIZE);

			REX_CORE_EXPORT WindowResize(rex::Window* window, uint32 width, uint32 height);

			REX_CORE_EXPORT uint32 getWidth() const;
			REX_CORE_EXPORT uint32 getHeight() const;

			REX_CORE_EXPORT std::string toString() const override;

		private:
			uint32 m_width;
			uint32 m_height;
		};
	}
}