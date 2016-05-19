//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <opengl/gl3.h>
#include <glm/glm.hpp>
#include "gl_exceptions.hpp"
#include "gl_info_log.inl"


namespace gl
{

    enum class ShaderType
        : GLenum
    {
        Invalid     = GL_INVALID_ENUM,
        Vertex      = GL_VERTEX_SHADER,
        Fragment    = GL_FRAGMENT_SHADER,
        Geometry    = GL_GEOMETRY_SHADER,
    };

    // Convert the ShaderType enumeration to a string.
    inline std::string ShaderTypeToString(const ShaderType shaderType)
    {
        switch (shaderType)
        {
        case ShaderType::Vertex:
            return "Vertex";
        case ShaderType::Fragment:
            return "Fragment";
        case ShaderType::Geometry:
            return "Geometry";
        default:
            break;
        }
        return "Invalid";
    }


    namespace impl
    {

        // Returns: the shader type or ShaderType::Invalid.
        inline ShaderType Type(const GLuint name) noexcept
        {
            GLint result = static_cast<GLint>(ShaderType::Invalid);

            ::glGetShaderiv(name, GL_SHADER_TYPE, &result);
            if (GL_NO_ERROR == ::glGetError())
            {
                return static_cast<ShaderType>(result);
            }

            return ShaderType::Invalid;
        }

        // Throws: InvalidValue, InvalidOperation : name is bad.
        inline void Compile(const GLuint name, const std::string& shaderSourceCode)
        {
            assert(shaderSourceCode.length() <= std::numeric_limits<GLint>::max());

            const GLchar*   strings[] { shaderSourceCode.data() };
            const GLint     lengths[] { static_cast<GLint>(shaderSourceCode.length()) };

            ::glShaderSource(name, 1, strings, lengths);
            VerifyNoErrors(); // An exception here indicates a programming error.

            ::glCompileShader(name);
            AssertNoErrors(); // An exception here indicates a programming error.
        }

        // Returns: true if the shader was compiled successfully.
        // Throws: InvalidValue, InvalidOperation : name is bad.
        inline bool IsValidShader(const GLuint name)
        {
            GLint result = GL_FALSE;

            ::glGetShaderiv(name, GL_COMPILE_STATUS, &result);
            VerifyNoErrors(); // NOTE: An InvalidEnum exception here indicates a programming error.

            return GL_TRUE == result;
        }

        inline std::string CopyShaderInfoLog(const GLuint name)
        {
            return detail::CopyGlInfoLog(name, &::glGetShaderiv, &::glGetShaderInfoLog);
        }

    } // namespace impl


    // Acts as a wrapper around a native OpenGL shader resource.
    class Shader
    {
    public:
        // Allocates a shader resource.
        // Throws:  InvalidEnum : shaderType is bad
        //          std::runtime_error : failed to create shader resource
        Shader(const ShaderType shaderType)
        {
            assert(
                ShaderType::Vertex == shaderType    ||
                ShaderType::Fragment == shaderType  ||
                ShaderType::Geometry == shaderType
                );

            _name = ::glCreateShader(static_cast<GLenum>(shaderType));
            if (0 != _name)
            {
                return;
            }

            VerifyNoErrors();   // GL_INVALID_ENUM is generated if shaderType is not an accepted
                                // value. This could happen in release mode by skipping the assert.

            // It's not clear from the GL documentation if a shader can fail to create for any other
            // reasons. In any event, it would be an exceptional situation.
            throw std::runtime_error("failed to create shader resource");
        }

        // Allocates a shader resource and compiles a single shader source file.
        // Throws:  InvalidEnum : shaderType is bad
        //          std::runtime_error : failed to create shader resource;
        //                               or, failed to compile shader
        //          std::bad_alloc
        Shader(const ShaderType shaderType, const std::string& shaderSourceCode)
            : Shader(shaderType)
        {
            impl::Compile(_name, shaderSourceCode);

            if (impl::IsValidShader(_name))
            {
                return;
            }

            throw std::runtime_error(
                std::string("failed to compile shader\n\n") +
                impl::CopyShaderInfoLog(_name)
                );
        }

        virtual ~Shader()
        {
            ::glDeleteShader(_name);
            AssertNoErrors(); // This would indicate a programming error.
        }

        // Get the OpenGL shader object's name.
        GLuint Name() const noexcept
        {
            return _name;
        }

        // Returns: the shader type or ShaderType::Invalid.
        ShaderType Type() const noexcept
        {
            return impl::Type(_name);
        }

        // Replaces the shader source and compiles.
        // compilerOutput is assigned any log messages produced by the driver's compiler.
        // Returns: true if the source compiled successfully.
        bool Compile(const std::string& shaderSourceCode) noexcept
        {
            impl::Compile(_name, shaderSourceCode);
            return impl::IsValidShader(_name);
        }

        // Returns: true if the shader was compiled successfully.
        bool IsValid() const noexcept
        {
            return impl::IsValidShader(_name); // Any exception here indicates a programming error.
        }

        // Throws: std::bad_alloc
        std::string GetCompilerOutput() const
        {
            // NOTE: Any other exceptions here indicate a programming error.
            return impl::CopyShaderInfoLog(_name);
        }

        // Not copyable.
        Shader(const Shader&) = delete;
        Shader& operator =(const Shader&) = delete;

    private:
        GLuint _name;
    };

} // namespace gl


#endif
