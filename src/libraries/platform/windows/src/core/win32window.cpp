#include "rex_platform_pch.h"

#include "core/win32window.h"

#include "math/bounds.h"

namespace
{
    const DWORD STYLE = WS_POPUPWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

    //-------------------------------------------------------------------------
    rex::win32::EventProcessor* getUserData(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(wParam);

        if (uMsg == WM_CREATE)
        {
            LONG_PTR user_data = (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams;

            SetWindowLongPtr(hWnd, GWLP_USERDATA, user_data);

            return (rex::win32::EventProcessor*)(user_data);
        }

        return (rex::win32::EventProcessor*)(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    //-------------------------------------------------------------------------
    rex::BoundsI createWindowBounds(uint32 width, uint32 height)
    {
        POINT top_left
        {
            GetSystemMetrics(SM_CYSCREEN) / 2 - static_cast<long>(height) / 2,  // top
            GetSystemMetrics(SM_CXSCREEN) / 2 - static_cast<long>(width) / 2    // left
        };
        POINT bottom_right
        {
            top_left.x + static_cast<long>(height),                             // bottom
            top_left.y + static_cast<long>(width)                               // right
        };

        // Bounds order => top - left - bottom - right
        return rex::BoundsI(top_left.x, top_left.y, bottom_right.x, bottom_right.y);
    }
    //-------------------------------------------------------------------------
    WNDCLASS createWindowClass(const std::string& appName, HINSTANCE hInst, WNDPROC windowProcedure)
    {
        WNDCLASS wc;
        memset(&wc, NULL, sizeof(WNDCLASS));

        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = windowProcedure;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInst;

        wc.hIcon = NULL;
        wc.hCursor = NULL;

        wc.hbrBackground = NULL;
        wc.lpszMenuName = NULL;

        wc.lpszClassName = appName.c_str();

        return wc;
    }
    //-------------------------------------------------------------------------
    PIXELFORMATDESCRIPTOR createWindowPixelFormatDescription()
    {
        PIXELFORMATDESCRIPTOR result = {};
        result.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        result.nVersion = 1;
        result.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        result.iPixelType = PFD_TYPE_RGBA;
        result.cColorBits = 32;
        result.cDepthBits = 24;
        result.cStencilBits = 8;
        result.cAuxBuffers = 0;
        result.iLayerType = PFD_MAIN_PLANE;
        return result;
    }
}

//-------------------------------------------------------------------------
rex::win32::Window::Window(const WindowProperties& properties)
    :rex::Window()
    ,m_visible(false)
    ,m_hinstance(NULL)
    ,m_hwnd(NULL)
    ,m_wndproc(NULL)
    ,m_event_processor(this, properties.event_callback)
    ,m_width(properties.width)
    ,m_height(properties.height)
    ,m_title(properties.title)
{
    RX_INFO("Creating window: {0} ({1}, {2})", m_title, m_width, m_height);

    setupWindowClass();
    setupHwnd();
    setupPixelFormat();
}
//-------------------------------------------------------------------------
rex::win32::Window::~Window() = default;

//-------------------------------------------------------------------------
void rex::win32::Window::show()
{
    if (m_visible)
        return;

    m_visible = true;

    if (!ShowWindow(m_hwnd, SW_SHOW))
    {
        RX_ERROR("Could not make the window visible");
        return;
    }

    if (!SetForegroundWindow(m_hwnd))
    {
        RX_ERROR("Could not make the window the foreground window");
        return;
    }
    if (SetFocus(m_hwnd) == NULL)
    {
        RX_ERROR("Could not focus the window");
        return;
    }
}
//-------------------------------------------------------------------------
void rex::win32::Window::hide()
{
    if (!m_visible)
        return;

    m_visible = false;


}

//-------------------------------------------------------------------------
void rex::win32::Window::update()
{
    if (m_visible == false)
        return;

    MSG message;
    while (PeekMessage(&message, NULL, NULL, NULL, PM_REMOVE) > 0)
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}

//-------------------------------------------------------------------------
unsigned int rex::win32::Window::getWidth() const
{
    return m_width;
}
//-------------------------------------------------------------------------
unsigned int rex::win32::Window::getHeight() const
{
    return m_height;
}

//-------------------------------------------------------------------------
void* rex::win32::Window::getHandle()
{
    return (void*)m_hwnd;
}

//-------------------------------------------------------------------------
LRESULT CALLBACK rex::win32::Window::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    rex::win32::EventProcessor* event_processor = getUserData(hWnd, uMsg, wParam, lParam);
    if (event_processor != nullptr)
    {
        return event_processor->process(hWnd, uMsg, { wParam, lParam });
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}   

//-------------------------------------------------------------------------
void rex::win32::Window::setupWindowClass()
{
    const char* classname = typeid(this).name();

    WNDCLASS wndClass = createWindowClass(classname, m_hinstance, m_wndproc);

    auto result = RegisterClass(&wndClass);
    if (!result)
    {
        RX_ERROR("Could not register WNDCLASS");
        return;
    }
}
//-------------------------------------------------------------------------
void rex::win32::Window::setupHwnd()
{
    rex::BoundsI bounds = createWindowBounds(getWidth(), getHeight());

    m_hwnd = CreateWindowEx(0,
        m_title.c_str(),
        m_title.c_str(),
        STYLE,
        bounds.left, bounds.top,
        bounds.width(), bounds.height(),
        NULL,
        NULL,
        (HINSTANCE)GetModuleHandle(NULL),
        &m_event_processor);

    if (m_hwnd == NULL)
    {
        RX_ERROR("Failed to create window");
        return;
    }
}
//-------------------------------------------------------------------------
void rex::win32::Window::setupPixelFormat()
{
    HDC hdc = GetDC(m_hwnd);

    PIXELFORMATDESCRIPTOR pfd = createWindowPixelFormatDescription();

    auto choose_pixel_format = ChoosePixelFormat(hdc, &pfd);
    RX_ASSERT_X(choose_pixel_format != NULL, "Pixel format is null");

    auto set_pixel_format = SetPixelFormat(hdc, choose_pixel_format, &pfd);
    RX_ASSERT_X(set_pixel_format == TRUE, "Failed to set pixel format");

    auto release_dc = ReleaseDC(m_hwnd, hdc);
    RX_ASSERT_X(release_dc != NULL, "Failed to release DC");
}