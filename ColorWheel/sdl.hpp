//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#ifndef SDL_HPP
#define SDL_HPP


namespace sdl
{

    // Pointer that auto-frees an SDL allocated string.
    typedef std::unique_ptr<char, decltype(&::SDL_free)> string_ptr;

} // namespace sdl


#ifndef SDL_EXCEPTION_HPP
  #include "sdl_exception.hpp"
#endif

#ifndef SDL_LIBRARY_HPP
  #include "sdl_library.hpp"
#endif

#ifndef SDL_WINDOW_HPP
  #include "sdl_window.hpp"
#endif

#ifndef SDL_GLCONTEXT_HPP
  #include "sdl_glcontext.hpp"
#endif

#ifndef SDL_FILESYSTEM_HPP
  #include "sdl_filesystem.hpp"
#endif


namespace sdl
{

    typedef std::unique_ptr<SdlLibrary> library_ptr;
    typedef std::unique_ptr<WindowHandle> window_ptr;
    typedef std::unique_ptr<GlContext> glcontext_ptr;
    
} // namespace sdl


#endif
