//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#ifndef GET_UNIFORM_H
#define GET_UNIFORM_H

#include <iterator>


namespace gl { namespace impl
{

    // Pre-declare the user callable template function for the friend references.
    // Returns: the requested uniform variable
    // Throws:  InvalidValue : programName is bad
    //          InvalidOperation : programName or uniformLocation is bad;
    //                             or, the program hasn't been linked
    template<typename result_t>
    inline result_t GetProgramUniform(const GLuint programName, const GLint uniformLocation);


    // Implementation details not intended to be accessed directly by client code.
    namespace detail
    {

        // These types are used as template parameters for meta-programming.
        struct ScalarType;
        struct VectorType;
        struct MatrixType;

        // Specializations of this template class provide the implementation of calls into the different
        // flavors of the glGetUniform() function calls.
        template<
            typename base_t,        // There are three version of glGetUniform: float, int, and uint.
            typename dimension_t,   // Scalar, vector or matrix type.
            typename value_t        // The returned OpenGL or GLM type.
        >
        class GetUniform
        {
            static value_t Get(const GLuint programName, const GLint uniformLocation);
        };

        template<typename value_t>
        class GetUniform<GLfloat, ScalarType, value_t>
        {
            friend inline value_t GetProgramUniform<value_t>(const GLuint, const GLint);

            static value_t Get(const GLuint programName, const GLint uniformLocation)
            {
                value_t result = 0.0f;
                ::glGetUniformfv(programName, uniformLocation, &result);
                return result;
            }
        };

        template<typename value_t>
        class GetUniform<GLfloat, VectorType, value_t>
        {
            friend inline value_t GetProgramUniform<value_t>(const GLuint, const GLint);

            static value_t Get(const GLuint programName, const GLint uniformLocation)
            {
                value_t result;
                ::glGetUniformfv(programName, uniformLocation, &result[0]);
                return result;
            }
        };

        template<typename value_t>
        class GetUniform<GLfloat, MatrixType, value_t>
        {
            friend inline value_t GetProgramUniform<value_t>(const GLuint, const GLint);

            static value_t Get(const GLuint programName, const GLint uniformLocation)
            {
                value_t result;
                ::glGetUniformfv(programName, uniformLocation, &result[0][0]);
                return result;
            }
        };

        template<typename value_t>
        class GetUniform<GLint, ScalarType, value_t>
        {
            friend inline value_t GetProgramUniform<value_t>(const GLuint, const GLint);

            static value_t Get(const GLuint programName, const GLint uniformLocation)
            {
                value_t result = 0;
                ::glGetUniformiv(programName, uniformLocation, &result);
                return result;
            }
        };

        template<typename value_t>
        class GetUniform<GLint, VectorType, value_t>
        {
            friend inline value_t GetProgramUniform<value_t>(const GLuint, const GLint);

            static value_t Get(const GLuint programName, const GLint uniformLocation)
            {
                value_t result;
                ::glGetUniformiv(programName, uniformLocation, &result[0]);
                return result;
            }
        };

        template<typename value_t>
        class GetUniform<GLuint, ScalarType, value_t>
        {
            friend inline value_t GetProgramUniform<value_t>(const GLuint, const GLint);

            static value_t Get(const GLuint programName, const GLint uniformLocation)
            {
                value_t result = 0u;
                ::glGetUniformuiv(programName, uniformLocation, &result);
                return result;
            }
        };

        template<typename value_t>
        class GetUniform<GLuint, VectorType, value_t>
        {
            friend inline value_t GetProgramUniform<value_t>(const GLuint, const GLint);

            static value_t Get(const GLuint programName, const GLint uniformLocation)
            {
                value_t result;
                ::glGetUniformuiv(programName, uniformLocation, &result[0]);
                return result;
            }
        };

    } // namespace detail


    // Get Program Uniform specializations.
    template<>
    inline GLfloat GetProgramUniform<GLfloat>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLfloat, detail::ScalarType, GLfloat>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::vec2 GetProgramUniform<glm::vec2>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLfloat, detail::VectorType, glm::vec2>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::vec3 GetProgramUniform<glm::vec3>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLfloat, detail::VectorType, glm::vec3>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::vec4 GetProgramUniform<glm::vec4>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLfloat, detail::VectorType, glm::vec4>::Get(programName, uniformLocation);
    }

    template<>
    inline GLint GetProgramUniform<GLint>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLint, detail::ScalarType, GLint>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::ivec2 GetProgramUniform<glm::ivec2>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLint, detail::VectorType, glm::ivec2>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::ivec3 GetProgramUniform<glm::ivec3>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLint, detail::VectorType, glm::ivec3>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::ivec4 GetProgramUniform<glm::ivec4>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLint, detail::VectorType, glm::ivec4>::Get(programName, uniformLocation);
    }

    template<>
    inline GLuint GetProgramUniform<GLuint>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLuint, detail::ScalarType, GLuint>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::uvec2 GetProgramUniform<glm::uvec2>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLuint, detail::VectorType, glm::uvec2>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::uvec3 GetProgramUniform<glm::uvec3>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLuint, detail::VectorType, glm::uvec3>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::uvec4 GetProgramUniform<glm::uvec4>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLuint, detail::VectorType, glm::uvec4>::Get(programName, uniformLocation);
    }

    template<>
    inline bool GetProgramUniform<bool>(const GLuint programName, const GLint uniformLocation)
    {
        return 0 != GetProgramUniform<GLuint>(programName, uniformLocation);
    }

    template<>
    inline glm::bvec2 GetProgramUniform<glm::bvec2>(const GLuint programName, const GLint uniformLocation)
    {
        return glm::bvec2(GetProgramUniform<glm::uvec2>(programName, uniformLocation));
    }

    template<>
    inline glm::bvec3 GetProgramUniform<glm::bvec3>(const GLuint programName, const GLint uniformLocation)
    {
        return glm::bvec3(GetProgramUniform<glm::uvec3>(programName, uniformLocation));
    }

    template<>
    inline glm::bvec4 GetProgramUniform<glm::bvec4>(const GLuint programName, const GLint uniformLocation)
    {
        return glm::bvec4(GetProgramUniform<glm::uvec4>(programName, uniformLocation));
    }

    template<>
    inline glm::mat2 GetProgramUniform<glm::mat2>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLfloat, detail::MatrixType, glm::mat2>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::mat3 GetProgramUniform<glm::mat3>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLfloat, detail::MatrixType, glm::mat3>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::mat4 GetProgramUniform<glm::mat4>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLfloat, detail::MatrixType, glm::mat4>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::mat2x3 GetProgramUniform<glm::mat2x3>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLfloat, detail::MatrixType, glm::mat2x3>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::mat2x4 GetProgramUniform<glm::mat2x4>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLfloat, detail::MatrixType, glm::mat2x4>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::mat3x2 GetProgramUniform<glm::mat3x2>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLfloat, detail::MatrixType, glm::mat3x2>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::mat3x4 GetProgramUniform<glm::mat3x4>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLfloat, detail::MatrixType, glm::mat3x4>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::mat4x2 GetProgramUniform<glm::mat4x2>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLfloat, detail::MatrixType, glm::mat4x2>::Get(programName, uniformLocation);
    }

    template<>
    inline glm::mat4x3 GetProgramUniform<glm::mat4x3>(const GLuint programName, const GLint uniformLocation)
    {
        return detail::GetUniform<GLfloat, detail::MatrixType, glm::mat4x3>::Get(programName, uniformLocation);
    }

} } // namespace gl::impl


#endif
