//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#ifndef OPENGL_EXCEPTIONS_H
#define OPENGL_EXCEPTIONS_H


namespace gl
{

    // OpenGL error codes represented by exceptions within the standard C++ exception categories.
    /////////////////////////////////////////////////////////////////////////////////////////////
    class InvalidEnum
        : public std::out_of_range
    {
    public:
        InvalidEnum() noexcept
            : std::out_of_range("enum argument out of range") {}
    };

    class InvalidFramebufferOperation
        : public std::logic_error
    {
    public:
        InvalidFramebufferOperation() noexcept
            : std::logic_error("framebuffer is incomplete") {}
    };

    class InvalidValue
        : public std::out_of_range
    {
    public:
        InvalidValue() noexcept
            : std::out_of_range("numeric argument out of range") {}
    };

    class InvalidOperation
        : public std::logic_error
    {
    public:
        InvalidOperation() noexcept
            : std::logic_error("operation illegal in current state") {}
    };

    class OutOfMemory
        : public std::bad_alloc
    {
    public:
        OutOfMemory() noexcept
            : std::bad_alloc() {}
        
        virtual const char* what() const noexcept
        {
            return "Not enough memory left to execute command. OpenGL is now in an undefined state.";
        }
    };

    class UnrecognizedErrorCode
        : public std::runtime_error
    {
    public:
        UnrecognizedErrorCode() noexcept
            : std::runtime_error("unrecognized OpenGL error code") {}
    };


    // Call this function after each relevant OpenGL API to check for errors.
    // NOTE: See the functions below.
    /////////////////////////////////////////////////////////////////////////
    inline void ThrowGlError(const GLenum errorCode)
    {
        switch (errorCode)
        {
            case GL_NO_ERROR:
                return;

            case GL_INVALID_ENUM:
                throw InvalidEnum();

            case GL_INVALID_FRAMEBUFFER_OPERATION:
                throw InvalidFramebufferOperation();

            case GL_INVALID_VALUE:
                throw InvalidValue();

            case GL_INVALID_OPERATION:
                throw InvalidOperation();

            case GL_OUT_OF_MEMORY:
                throw OutOfMemory();

            default:
                break;
        }

        throw UnrecognizedErrorCode();
    }


    // This checks the GL error state and throws exceptions, even in release builds.
    inline void VerifyNoErrors()
    {
        ThrowGlError(::glGetError());
    }


    // It's common to disable GL error checks in release builds for better perf.
    // Like a C language assert, this method throws only in debug builds.
    inline void AssertNoErrors()
    {
#ifndef NDEBUG
        VerifyNoErrors();
#endif
    }

} // namespace gl


#endif
