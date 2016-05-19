//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#ifndef SDL_WINDOW_HPP
#define SDL_WINDOW_HPP


namespace sdl
{

    // WindowHandle and window_ptr types:
    /////////////////////////////////////

    // This example is probably the easiest to understand and most conventional way to wrap a native
    // resource. Just make a tiny "handle class" around it. This has implications for how the
    // unique_ptr is used within the the program:
    //
    // To create a new window_handle:
    //      auto pWindow = std::make_unique<WindowHandle>(...);
    //
    // To check to see if a window_handle is valid or not:
    //      if (pWindow) // if (!pWindow)
    //
    // To use a window_ptr in an API that takes a SDL_Window*:
    //      SDL_WindowFunction(*pWindow);
    //
    class WindowHandle
    {
    public:
        typedef SDL_Window* const   value_type;
        typedef value_type&         reference;

        WindowHandle(const WindowHandle&) = delete;
        WindowHandle& operator =(const WindowHandle&) = delete;

        WindowHandle(
            const char *title,
            int x, int y,
            int w, int h,
            Uint32 flags
            )
            : _handle(::SDL_CreateWindow(title, x, y, w, h, flags))
        {
            if (nullptr != _handle) // if (the resource was successfully aquired)
            {
                return;
            }

            throw SdlException();
        }

        virtual ~WindowHandle()
        {
            if (nullptr != _handle) // if (the handle is valid)
            {
                ::SDL_DestroyWindow(_handle);
            }
        }

        operator reference() const noexcept
        {
            return _handle;
        }

    private:
        value_type _handle;
    };

} // namespace sdl

#endif
