//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#ifndef SDL_GLCONTEXT_HPP
#define SDL_GLCONTEXT_HPP


namespace sdl
{

    // SDL GlContext and glcontext_ptr types:
    /////////////////////////////////////////
    class GlContext
    {
    public:
        typedef SDL_GLContext const value_type;
        typedef value_type&         reference;

    private:
        value_type _handle;

    public:
        GlContext(const GlContext&) = delete;
        GlContext& operator =(const GlContext&) = delete;

        explicit GlContext(const SDL_Window* const pSdlWindow)
            : _handle(::SDL_GL_CreateContext(const_cast<SDL_Window* const>(pSdlWindow)))
        {
            if (nullptr != _handle) // if (the resource was successfully aquired)
            {
                return;
            }

            throw SdlException();
        }

        virtual ~GlContext()
        {
            if (nullptr != _handle) // if (the handle is valid)
            {
                ::SDL_GL_DeleteContext(_handle);
            }
        }

        operator reference() const noexcept
        {
            return _handle;
        }
    };

} // namespace sdl


#endif
