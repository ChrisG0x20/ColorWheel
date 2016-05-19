//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#ifndef SDL_EXCEPTION_HPP
#define SDL_EXCEPTION_HPP


namespace sdl
{

    // SDL only produces text errors for clients. The "throw" sites inside the SDL library contain
    // more categorical information about the exception types, but this information is not exposed
    // to clients. All this translates into a single exception type. :(
    ////////////////////////////////////////////////////////////////////////////////////////////////
    class SdlException
        : public std::runtime_error
    {
    public:
        SdlException() noexcept
            : runtime_error(::SDL_GetError())
        {
            ::SDL_ClearError();
        }

        explicit SdlException(const std::string& what) noexcept
            : runtime_error(std::string(::SDL_GetError()) + " : " + what)
        {
            ::SDL_ClearError();
        }

        explicit SdlException(const char* what) noexcept
            : runtime_error(std::string(::SDL_GetError()) + " : " + what)
        {
            ::SDL_ClearError();
        }
    };

} // namespace sdl


#endif
