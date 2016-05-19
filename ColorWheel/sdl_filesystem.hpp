//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#ifndef SDL_FILESYSTEM_HPP
#define SDL_FILESYSTEM_HPP


namespace sdl
{

    // Returns : A path to the current executable.
    // Throws : SdlException
    inline string_ptr GetExecutablePath()
    {
        string_ptr pBasePath(::SDL_GetBasePath(), &::SDL_free);
        if (!pBasePath)
        {
            throw SdlException();
        }

        return pBasePath;
    }

    // Returns : A user preferences folder using both org and app folders.
    // Throws : SdlException
    inline string_ptr GetApplicationSettingsPath(
        const std::string& organizationName,
        const std::string& applicationName
        )
    {
        string_ptr pPrefPath(
            ::SDL_GetPrefPath(organizationName.c_str(), applicationName.c_str()),
            &::SDL_free
            );
        if (!pPrefPath)
        {
            throw SdlException();
        }

        return pPrefPath;
    }

} // namespace sdl


#endif
