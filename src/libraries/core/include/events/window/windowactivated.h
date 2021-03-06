#pragma once

#include "rex_core_global.h"

#include "events/window/windowevent.h"
#include "events/genericevent.h"

namespace rex
{
	class Window;

	namespace events
	{
		class WindowActivated : public WindowEvent, public GenericEvent
		{
		public:
			EVENT_CLASS_TYPE(EventType::WINDOW_ACTIVATED);

			REX_CORE_EXPORT WindowActivated(rex::Window* window);
			REX_CORE_EXPORT ~WindowActivated() final;

            //-------------------------------------------------------------------------
            inline const NativeEvent* getNativeEvent() const final { return this; }
		};
	}
}
