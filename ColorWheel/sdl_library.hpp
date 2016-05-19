//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#ifndef SDL_LIBRARY_HPP
#define SDL_LIBRARY_HPP


namespace sdl
{

    // SDL Library RAII wrapper
    ///////////////////////////
    class SdlLibrary
    {
    public:
        typedef const int       value_type;
        typedef value_type&     reference;
        
    private:
        value_type _handle;

    public:
        SdlLibrary(const SdlLibrary&) = delete;
        SdlLibrary& operator =(const SdlLibrary&) = delete;

        explicit SdlLibrary(const Uint32 flags)
            : _handle(::SDL_Init(flags))
        {
            if (0 == _handle) // if (the resource was successfully aquired)
            {
                return;
            }

            throw SdlException();
        }

        virtual ~SdlLibrary()
        {
            if (0 == _handle) // if (the handle is valid)
            {
                ::SDL_Quit();
            }
        }

        operator reference() const noexcept
        {
            return _handle;
        }
    };

} // namespace sdl


#endif
