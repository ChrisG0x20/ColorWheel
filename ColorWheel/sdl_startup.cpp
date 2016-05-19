//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#include <memory>
#include <stdexcept>
#include <string>
#include <iostream>

#include <SDL2/SDL.h>           // Primary SDL include for all platforms.
#include <opengl/gl3.h>         // Probably specific to OS X.
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>          // Vector and Matrix math.

#include "sdl.hpp"
#include "sdl_startup.hpp"


// Configure the OpenGL / OpenGL ES settings for the SDL.
//
// SDL requires many settings to be configured before creating an application window or OpenGL
// rendering context.
void ConfigureSdlOpenGLSettings() noexcept
{
    // NOTE: "core profile" is supposed to remove legacy APIs.
    ::SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // OpenGL ES 2.0 is based roughly on OpenGL 2.0 functionality.
    // When using the "core profile", OpenGL 3.1, 3.2, and 3.3 provide a very similar
    //   programming model to OpenGL ES 2.0.
    // OpenGL 4.1 is the first version that claims full API compatibility with OpenGL ES 2.0
    //   (ARB_ES2_compatibility).
    // NOTE: OpenGL 4.3 (not yet available for OS X) provides full API compatibility with OpenGL ES 3.0.
    ::SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    ::SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    // Use hardware acceleration (this may matter for Linux).
    ::SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    // 8-bits for each color and Alpha channel (32-bit color mode).
    ::SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    ::SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    ::SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    ::SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    // 24-bit depth (Z buffer).
    ::SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    //::SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Use double buffering.
    // NOTE: It's possible that disabling this might provide a better debug experience,
    // but maybe not with modern hardware and software.
    ::SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
}


// Writes out information read back from the system's OpenGL driver.
// NOTE: This could be stuffed into its own "OpenGL utility functions" file somewhere.
void WriteOpenGLInfo() noexcept
{
    // Some basic info about the OpenGL renderer (driver).
    std::cout << "OpenGL vendor  : " << ::glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL renderer: " << ::glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL version : " << ::glGetString(GL_VERSION) << std::endl;
    std::cout << "Shader version : " << ::glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Version of the _current_ rendering context.
    // Not what the driver can do, but what it's configured for _right now_.
    {
        auto contextMajorVersion = 0;
        ::glGetIntegerv(GL_MAJOR_VERSION, &contextMajorVersion);
        auto contextMinorVersion = 0;
        ::glGetIntegerv(GL_MINOR_VERSION, &contextMinorVersion);

        std::cout << "Context version: ";
        if (0 != contextMajorVersion)
        {
            std::cout << contextMajorVersion << "." << contextMinorVersion << std::endl;
        }
        else
        {
            std::cout << "unknown" << std::endl;
        }
    }

    // Get a list of the OpenGL extensions supported by this driver.
    {
        auto extensionCount = 0;
        ::glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);

        std::cout << "OpenGL extensions: ";
        if (0 != extensionCount)
        {
            std::cout << std::endl << std::endl;
            for (auto i = 0; i < extensionCount; ++i)
            {
                auto ext = ::glGetStringi(GL_EXTENSIONS, i);
                std::cout << reinterpret_cast<const char*>(ext) << " ";
            }
            std::cout << std::endl << std::endl;
        }
        else
        {
            std::cout << "none" << std::endl;
        }
    }

    // This should always be zero (0).
    auto err = ::glGetError();
    std::cout << "GL ES Error Status: " << err << std::endl;
}


// Initializes the SDL library.
// Throws: SdlException
sdl::library_ptr InitializeApplication()
{
    // Look at the SDL library version info.
    {
        SDL_version compiled;
        SDL_VERSION(&compiled);

        SDL_version linked;
        ::SDL_GetVersion(&linked);

        std::cout << "Compiled against SDL version        : " <<
        static_cast<int>(compiled.major) << '.' << static_cast<int>(compiled.minor) << '.' <<
        static_cast<int>(compiled.patch) << std::endl;
        std::cout << "Currently linked against SDL version: " <<
        static_cast<int>(linked.major) << '.' << static_cast<int>(linked.minor) << '.' <<
        static_cast<int>(linked.patch) << std::endl;
    }

    // Some boring diagnostics from the SDL.
    // NOTE: These APIs can be called pre-initialization of the SDL.
    {
        std::cout << "SDL Video drivers:" << std::endl << std::endl;
        const auto c = ::SDL_GetNumVideoDrivers();
        for (auto i = 0; i < c; ++i)
        {
            std::cout << ::SDL_GetVideoDriver(i) << std::endl;
        }
        std::cout << std::endl;
    }

    // Initialize the SDL.
    auto result = sdl::library_ptr(new sdl::SdlLibrary(SDL_INIT_EVERYTHING));

    ConfigureSdlOpenGLSettings();

    return result;
}


// Creates the main application window.
// Throws: SdlException
sdl::window_ptr CreateMainWindow(const std::string& windowName, const glm::ivec2 windowSize)
{
    return sdl::window_ptr(new sdl::WindowHandle(
        windowName.c_str(),         // Title of the window.
        1400, 100,                  // Location to put the window on screen.
        windowSize.x, windowSize.y, // Size of the window.
        SDL_WINDOW_OPENGL |         // This window will be used for OpenGL rendering.
        SDL_WINDOW_ALLOW_HIGHDPI    // Use retina resolution if available.
        // SDL_WINDOW_SHOWN // NOTE: This is in every example, but I believe it is the default behavior.
        ));
}


// Causes the main application window to be the target of OpenGL drawing commands.
// Throws: SdlException
sdl::glcontext_ptr CreateRenderingContext(const sdl::WindowHandle& windowHandle)
{
    auto result = sdl::glcontext_ptr(new sdl::GlContext(windowHandle));

    WriteOpenGLInfo();

    return result;
}
