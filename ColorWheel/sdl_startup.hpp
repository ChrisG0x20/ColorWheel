//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#ifndef SDL_STARTUP
#define SDL_STARTUP

void ConfigureSdlOpenGLSettings() noexcept;
void WriteOpenGLInfo() noexcept;
sdl::library_ptr InitializeApplication();
sdl::window_ptr CreateMainWindow(const std::string& windowName, const glm::ivec2 windowSize);
sdl::glcontext_ptr CreateRenderingContext(const sdl::WindowHandle& windowHandle);

#endif
