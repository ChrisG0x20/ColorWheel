//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#ifndef INFO_LOG_H
#define INFO_LOG_H

#include <string>
#include <vector>
#include <opengl/gl3.h>
#include "gl_exceptions.hpp"


namespace gl { namespace impl { namespace detail
{

    // OpenGL defines a couple of functions with identical prototypes for reading driver log output.
    // OpenGL API functions are passed to a generic log reading function to avoid some copy & paste.
    typedef void (log_length_func)(GLuint, GLenum, GLint*);
    typedef void (copy_log_func)(GLuint, GLsizei, GLsizei*, GLchar*);

    // Returns: information copied from the OpenGL driver's output log
    // Throws:  InvalidValue, InvalidOperation : name is bad.
    //          std::bad_alloc
    inline std::string CopyGlInfoLog(
                                     const GLuint name,
                                     log_length_func LogLength,
                                     copy_log_func CopyLog
                                     )
    {
        // Allocate a buffer for the log messages.
        std::vector<GLchar> logBuffer;
        {
            GLint logLength = 0;
            // ::glGetShaderiv() -OR- ::glGetProgramiv()
            LogLength(name, GL_INFO_LOG_LENGTH, &logLength);
            VerifyNoErrors(); // NOTE: An InvalidEnum exception here indicates a programming error.

            if (0 == logLength)
            {
                return "";
            }

            logBuffer.resize(logLength);
        }

        // Pass the log buffer to OpenGL to fill.
        GLsizei length = 0;
        {
            // ::glGetShaderInfoLog() -OR- ::glGetProgramInfoLog()
            CopyLog(name, static_cast<GLsizei>(logBuffer.size()), &length, &logBuffer[0]);
            AssertNoErrors(); // This check should be redundant with the VerifyGlState() call above.

            // NOTE: This is a really paranoid check. This would only happen in the event of a bad
            // OpenGL implementation. It would imply the heap is corrupt.
            assert(length <= logBuffer.size());
        }

        // Finally, copy the log buffer into a string.
        return std::string(&logBuffer[0], std::min(static_cast<size_t>(length), logBuffer.size()));
    }

} } } // namespace gl::impl::detail


#endif
