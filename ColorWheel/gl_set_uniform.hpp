//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#ifndef SET_UNIFORM_H
#define SET_UNIFORM_H

#include <array>


namespace gl { namespace impl
{

    // Template parameters for Matrix memory layout.
    struct ColumnMajor {}; // OpenGL's native layout.
    struct RowMajor {};    // DirectX & C++'s native layout (requires the driver to transpose).

    // Any of these functions take a uniform location and parameter(s) to set the uniform.
    // Throws:  InvalidOperation : there is no current program object;
    //              or, the size of the uniform variable declared in the shader does not match the
    //                  size indicated by the passed uniform type;
    //              or, an integer variant of these functions was used to load a uniform variable of
    //                  a floating-point type;
    //              or, a floating-point variant of these functions was used to load a uniform
    //                  variable of an interger type;
    //              or, a signed interger varient of these functions was used to load an unsigned
    //                  uniform variable;
    //              or, an unsigned interger varient of these functions was used to load a signed
    //                  uniform variable;
    //              or, location is an invalid uniform location for the current program object and
    //                  location is not equal to -1;
    //              or, a sampler is loaded using a command other than a single GLint or an array of
    //                  GLints
    //          InvalidValue : buffer count is less than 0
    namespace detail
    {

        // Convert a scalar element to a GLint based boolean
        ////////////////////////////////////////////////////
        template<typename B>
        inline GLint b2i(const B f)
        {
            return (f) ? GL_TRUE : GL_FALSE;
        }

        // Set Elements
        ///////////////
        template<
            typename arithmetic_t = GLfloat // GLfloat, GLdouble, GLint, GLuint, GLboolean, bool
        >
        struct SetElements
        {
            static void Set(const GLint, const arithmetic_t);
            static void Set(const GLint, const arithmetic_t, const arithmetic_t);
            static void Set(const GLint, const arithmetic_t, const arithmetic_t, const arithmetic_t);
            static void Set(const GLint, const arithmetic_t, const arithmetic_t, const arithmetic_t, const arithmetic_t);
        };

#define SET_ELEMENTS(element_type, gl_suffix)\
        template<>\
        struct SetElements<element_type>\
        {\
            typedef element_type T;\
            static void Set(const GLint ul, const T v0)\
            {\
                ::glUniform1##gl_suffix(ul, v0);\
            }\
            static void Set(const GLint ul, const T v0, const T v1)\
            {\
                ::glUniform2##gl_suffix(ul, v0, v1);\
            }\
            static void Set(const GLint ul, const T v0, const T v1, const T v2)\
            {\
                ::glUniform3##gl_suffix(ul, v0, v1, v2);\
            }\
            static void Set(const GLint ul, const T v0, const T v1, const T v2, const T v3)\
            {\
                ::glUniform4##gl_suffix(ul, v0, v1, v2, v3);\
            }\
        }

        SET_ELEMENTS(GLfloat, f);
        SET_ELEMENTS(GLdouble, d);
        SET_ELEMENTS(GLint, i);
        SET_ELEMENTS(GLuint, ui);
        SET_ELEMENTS(bool, ui);

        // Set Array
        ////////////
        template<
            typename arithmetic_t = GLfloat,    // GLfloat, GLdouble, GLint, GLuint, GLboolean, bool
            int element_count = 1               // 1 - 4
        >
        struct SetArray
        {
            static void Set(const GLint, const arithmetic_t[], const GLsizei);
        };
        
#define SET_ARRAY(element_type, element_count, gl_suffix)\
        template<>\
        struct SetArray<element_type, element_count>\
        {\
            typedef element_type T;\
            static void Set(const GLint ul, const T v[], const GLsizei s)\
            {\
                ::glUniform##element_count##gl_suffix##v(ul, s, v);\
            }\
        }
        
        SET_ARRAY(GLfloat, 1, f);
        SET_ARRAY(GLfloat, 2, f);
        SET_ARRAY(GLfloat, 3, f);
        SET_ARRAY(GLfloat, 4, f);
        SET_ARRAY(GLdouble, 1, d);
        SET_ARRAY(GLdouble, 2, d);
        SET_ARRAY(GLdouble, 3, d);
        SET_ARRAY(GLdouble, 4, d);
        SET_ARRAY(GLint, 1, i);
        SET_ARRAY(GLint, 2, i);
        SET_ARRAY(GLint, 3, i);
        SET_ARRAY(GLint, 4, i);
        SET_ARRAY(GLuint, 1, ui);
        SET_ARRAY(GLuint, 2, ui);
        SET_ARRAY(GLuint, 3, ui);
        SET_ARRAY(GLuint, 4, ui);
        
#define SET_GL_BOOLEAN_ARRAY(element_count)\
        template<>\
        struct SetArray<GLboolean, element_count>\
        {\
            typedef GLboolean T;\
            static void Set(const GLint ul, const T v[], const GLsizei s)\
            {\
                std::vector<GLint> buffer(s);\
                std::transform(&v[0], &v[s], std::begin(buffer),\
                       [](const T& e) { return b2i(e); });\
                ::glUniform##element_count##iv(ul, s, &buffer[0]);\
            }\
        }

        SET_GL_BOOLEAN_ARRAY(1);
        SET_GL_BOOLEAN_ARRAY(2);
        SET_GL_BOOLEAN_ARRAY(3);
        SET_GL_BOOLEAN_ARRAY(4);

        // Converters for arrays of bool
        ////////////////////////////////
        namespace b2ia
        {
            template<bool bool_eq_GLint, int element_count>
            struct BoolToIntArray
            {
                static void Set(const GLint ul, const bool v[], const GLsizei s);
            };

#define BOOL_TO_INT_ARRAY_CONVERTER(element_count)\
            template<>\
            struct BoolToIntArray<true, element_count>\
            {\
                static void Set(const GLint ul, const bool v[], const GLsizei s)\
                {\
                    ::glUniform1iv(ul, s, reinterpret_cast<const GLint*>(v));\
                }\
            };\
\
            template<>\
            struct BoolToIntArray<false, element_count>\
            {\
                static void Set(const GLint ul, const bool v[], const GLsizei s)\
                {\
                    const auto length = s * element_count;\
                    std::vector<GLint> buffer(length);\
                    std::transform(&v[0], &v[length], std::begin(buffer),\
                                   [](const bool& e) { return b2i(e); });\
                    ::glUniform##element_count##iv(ul, s, &buffer[0]);\
                }\
            }

            BOOL_TO_INT_ARRAY_CONVERTER(1);
            BOOL_TO_INT_ARRAY_CONVERTER(2);
            BOOL_TO_INT_ARRAY_CONVERTER(3);
            BOOL_TO_INT_ARRAY_CONVERTER(4);
        } // namespace b2ia

        template<int element_count>
        struct SetArray<bool, element_count>
        {
            typedef bool T;

            static void Set(const GLint ul, const T v[], const GLsizei s)
            {
                b2ia::BoolToIntArray<sizeof(T) != sizeof(GLint), element_count>::Set(ul, v, s);
            }
        };


        // Set Matrices
        ///////////////
        template<typename matrix_layout_t>
        constexpr GLboolean Transpose()
        {
            static_assert(std::is_same<matrix_layout_t, ColumnMajor>::value ||
                          std::is_same<matrix_layout_t, RowMajor>::value,
                          "matrix layout must be ColumnMajor or RowMajor");
            return std::is_same<RowMajor, matrix_layout_t>::value ? GL_TRUE : GL_FALSE;
        }

        template<
            typename arithmetic_t = GLfloat,    // GLfloat, GLdouble
            int cols = 4,                       // 2 - 4; first number in glUniformMatrix() function suffix.
            int rows = 4,                       // 2 - 4; second number in glUniformMatrix() function suffix.
            typename matrix_layout_t = ColumnMajor  // ColumnMajor (OpenGL native format), RowMajor (needs to be transposed)
        >
        struct SetMatrices
        {
            static_assert(std::is_same<GLfloat, arithmetic_t>::value ||
                          std::is_same<GLdouble, arithmetic_t>::value,
                          "matrix element type must be the same as GLfloat or GLdouble");
            static_assert(cols >= 2 && cols <= 4, "unsupported number of matrix columns");
            static_assert(rows >= 2 && rows <= 4, "unsupported number of matrix rows");
            static void Set(const GLint, const GLfloat[], const GLsizei);
        };

#define SET_MATRICES(element_type, row_count, col_count, gl_suffix)\
        template<typename matrix_layout_t>\
        struct SetMatrices<element_type, row_count, col_count, matrix_layout_t>\
        {\
            typedef element_type T;\
            static void Set(const GLint ul, const T m[], const GLsizei s)\
            {\
                ::glUniformMatrix##gl_suffix##v(\
                ul, s, Transpose<matrix_layout_t>(), m);\
            }\
        }

        SET_MATRICES(GLfloat, 2, 2, 2f);
        SET_MATRICES(GLfloat, 3, 3, 3f);
        SET_MATRICES(GLfloat, 4, 4, 4f);
        SET_MATRICES(GLfloat, 2, 3, 2x3f);
        SET_MATRICES(GLfloat, 2, 4, 2x4f);
        SET_MATRICES(GLfloat, 3, 2, 3x2f);
        SET_MATRICES(GLfloat, 3, 4, 3x4f);
        SET_MATRICES(GLfloat, 4, 2, 4x2f);
        SET_MATRICES(GLfloat, 4, 3, 4x3f);
        SET_MATRICES(GLdouble, 2, 2, 2d);
        SET_MATRICES(GLdouble, 3, 3, 3d);
        SET_MATRICES(GLdouble, 4, 4, 4d);
        SET_MATRICES(GLdouble, 2, 3, 2x3d);
        SET_MATRICES(GLdouble, 2, 4, 2x4d);
        SET_MATRICES(GLdouble, 3, 2, 3x2d);
        SET_MATRICES(GLdouble, 3, 4, 3x4d);
        SET_MATRICES(GLdouble, 4, 2, 4x2d);
        SET_MATRICES(GLdouble, 4, 3, 4x3d);


        // Set from C array
        ///////////////////
        template<typename T, std::size_t N, glm::precision P = glm::defaultp>
        struct SetCArray
        {
            static_assert(std::is_arithmetic<T>::value, "invalid array element type");
            typedef const T (&const_array_ref)[N];

            static void Set(const GLint ul, const_array_ref v)
            {
                static_assert(N <= std::numeric_limits<GLsizei>::max(), "array is too large for OpenGL");
                SetArray<T, 1>::Set(ul, &v[0], N);
            }
        };

        // Set from C array of glm::vec<>
        /////////////////////////////////
#define SET_ARRAY_OF_VEC(element_count)\
        template<typename T, std::size_t N, glm::precision P>\
        struct SetCArray<glm::tvec##element_count<T, P>, N, P>\
        {\
            typedef const glm::tvec##element_count<T, P> (&const_array_ref)[N];\
\
            static void Set(const GLint ul, const_array_ref& v)\
            {\
                static_assert(N <= std::numeric_limits<GLsizei>::max(), "array is too large for OpenGL");\
                SetArray<T, element_count>::Set(ul, &(v[0])[0], N);\
            }\
        }
        
        SET_ARRAY_OF_VEC(2);
        SET_ARRAY_OF_VEC(3);
        SET_ARRAY_OF_VEC(4);

        // Set from C array of glm::mat<>
        /////////////////////////////////
#define SET_ARRAY_OF_MATRICES(row_count, col_count)\
        template<typename T, std::size_t N, glm::precision P>\
        struct SetCArray<glm::tmat##row_count##x##col_count<T, P>, N, P>\
        {\
            typedef const glm::tmat##row_count##x##col_count<T, P> (&const_array_ref)[N];\
            static void Set(const GLint ul, const_array_ref& v)\
            {\
                static_assert(N <= std::numeric_limits<GLsizei>::max(), "array is too large for OpenGL");\
                SetMatrices<T, row_count, col_count>::Set(ul, &(v[0])[0][0], N);\
            }\
        }

        SET_ARRAY_OF_MATRICES(2, 2);
        SET_ARRAY_OF_MATRICES(3, 3);
        SET_ARRAY_OF_MATRICES(4, 4);
        SET_ARRAY_OF_MATRICES(2, 3);
        SET_ARRAY_OF_MATRICES(2, 4);
        SET_ARRAY_OF_MATRICES(3, 2);
        SET_ARRAY_OF_MATRICES(3, 4);
        SET_ARRAY_OF_MATRICES(4, 2);
        SET_ARRAY_OF_MATRICES(4, 3);

        // Set from std::array<>
        ////////////////////////
        template<typename T, std::size_t N, glm::precision P = glm::defaultp>
        struct SetStdArray
        {
            static_assert(std::is_arithmetic<T>::value, "invalid array element type");
            typedef std::array<T, N> array_type;

            static void Set(const GLint ul, const array_type& v)
            {
                static_assert(N <= std::numeric_limits<GLsizei>::max(), "array is too large for OpenGL");
                SetArray<T, 1>::Set(ul, &v[0], N);
            }
        };

        // Set from std::array<> of glm::vec<>
        //////////////////////////////////////
#define SET_STD_ARRAY_OF_VEC(element_count)\
        template<typename T, std::size_t N, glm::precision P>\
        struct SetStdArray<glm::tvec##element_count<T, P>, N, P>\
        {\
            typedef std::array<glm::tvec##element_count<T, P>, N> array_type;\
\
            static void Set(const GLint ul, const array_type& v)\
            {\
                static_assert(N <= std::numeric_limits<GLsizei>::max(), "array is too large for OpenGL");\
                SetArray<T, element_count>::Set(ul, &(v[0])[0], N);\
            }\
        }
        
        SET_STD_ARRAY_OF_VEC(2);
        SET_STD_ARRAY_OF_VEC(3);
        SET_STD_ARRAY_OF_VEC(4);

        // Set from std::array<> of glm::mat<>
        //////////////////////////////////////
#define SET_STD_ARRAY_OF_MATRICES(row_count, col_count)\
        template<typename T, std::size_t N, glm::precision P>\
        struct SetStdArray<glm::tmat##row_count##x##col_count<T, P>, N, P>\
        {\
            typedef std::array<glm::tmat##row_count##x##col_count<T, P>, N> array_type;\
            static void Set(const GLint ul, const array_type& v)\
            {\
                static_assert(N <= std::numeric_limits<GLsizei>::max(), "array is too large for OpenGL");\
                SetMatrices<T, row_count, col_count>::Set(ul, &(v[0])[0][0], N);\
            }\
        }

        SET_STD_ARRAY_OF_MATRICES(2, 2);
        SET_STD_ARRAY_OF_MATRICES(3, 3);
        SET_STD_ARRAY_OF_MATRICES(4, 4);
        SET_STD_ARRAY_OF_MATRICES(2, 3);
        SET_STD_ARRAY_OF_MATRICES(2, 4);
        SET_STD_ARRAY_OF_MATRICES(3, 2);
        SET_STD_ARRAY_OF_MATRICES(3, 4);
        SET_STD_ARRAY_OF_MATRICES(4, 2);
        SET_STD_ARRAY_OF_MATRICES(4, 3);

        // Set from buffer
        //////////////////
        template<typename T, glm::precision P = glm::defaultp>
        struct SetBuffer
        {
            static_assert(std::is_arithmetic<T>::value, "invalid buffer element type");
            static void Set(const GLint ul, const T* const v, const GLsizei count)
            {
                SetArray<T, 1>::Set(ul, &v[0], count);
            }
        };

        // Set from buffer of glm::vec<>
        ////////////////////////////////
#define SET_BUFFER_OF_VEC(element_count)\
        template<typename T, glm::precision P>\
        struct SetBuffer<glm::tvec##element_count<T, P>, P>\
        {\
            typedef glm::tvec##element_count<T, P> vec_type;\
            static void Set(const GLint ul, const vec_type* const v, const GLsizei count)\
            {\
                SetArray<T, element_count>::Set(ul, &(v[0])[0], count);\
            }\
        }

        SET_BUFFER_OF_VEC(2);
        SET_BUFFER_OF_VEC(3);
        SET_BUFFER_OF_VEC(4);

        // Set from buffer of glm::mat<>
        ////////////////////////////////
#define SET_BUFFER_OF_MATRICES(row_count, col_count)\
        template<typename T, glm::precision P>\
        struct SetBuffer<glm::tmat##row_count##x##col_count<T, P>, P>\
        {\
            typedef glm::tmat##row_count##x##col_count<T, P> mat_type;\
            static void Set(const GLint ul, const mat_type* const v, const GLsizei count)\
            {\
                SetMatrices<T, row_count, col_count>::Set(ul, &(v[0])[0][0], count);\
            }\
        }

        SET_BUFFER_OF_MATRICES(2, 2);
        SET_BUFFER_OF_MATRICES(3, 3);
        SET_BUFFER_OF_MATRICES(4, 4);
        SET_BUFFER_OF_MATRICES(2, 3);
        SET_BUFFER_OF_MATRICES(2, 4);
        SET_BUFFER_OF_MATRICES(3, 2);
        SET_BUFFER_OF_MATRICES(3, 4);
        SET_BUFFER_OF_MATRICES(4, 2);
        SET_BUFFER_OF_MATRICES(4, 3);

        // Set from std::vector<>
        /////////////////////////
        template<typename T, typename A, glm::precision P = glm::defaultp>
        struct SetStdVector
        {
            static_assert(std::is_arithmetic<T>::value, "invalid std::vector<> element type");

            typedef std::vector<T, A> vector_type;

            static void Set(const GLint ul, const vector_type& v)
            {
                const typename vector_type::size_type MaxCount = std::numeric_limits<GLsizei>::max();
                assert(v.size() <= MaxCount);
                const auto count = static_cast<GLsizei>(std::min(MaxCount, v.size()));
                SetArray<T, 1>::Set(ul, &v[0], count);
            }
        };

        // NOTE: Special case required for std::vector<bool>
        // Set from std::vector<bool>
        /////////////////////////////
        template<typename A, glm::precision P>
        struct SetStdVector<bool, A, P>
        {
            typedef std::vector<bool, A> vector_type;

            static void Set(const GLint ul, const vector_type& v)
            {
                const typename vector_type::size_type MaxCount = std::numeric_limits<GLsizei>::max();
                assert(v.size() <= MaxCount);
                const auto count = static_cast<GLsizei>(std::min(MaxCount, v.size()));
                std::vector<GLint> buffer(count);
                std::transform(v.cbegin(), v.cend(), std::begin(buffer),
                               [](const bool& e) { return b2i(e); });
                SetArray<GLint, 1>::Set(ul, &buffer[0], count);
            }
        };

        // Set from std::vector<> of glm::vec<>
        ///////////////////////////////////////
#define SET_STD_VECTOR_OF_VEC(element_count)\
        template<typename T, typename A, glm::precision P>\
        struct SetStdVector<glm::tvec##element_count<T, P>, A, P>\
        {\
            typedef std::vector<glm::tvec##element_count<T, P>, A> vector_type;\
\
            static void Set(const GLint ul, const vector_type& v)\
            {\
                const typename vector_type::size_type MaxCount = std::numeric_limits<GLsizei>::max();\
                assert(v.size() <= MaxCount);\
                const auto count = static_cast<GLsizei>(std::min(MaxCount, v.size()));\
                SetArray<T, element_count>::Set(ul, &(v[0])[0], count);\
            }\
        }

        SET_STD_VECTOR_OF_VEC(2);
        SET_STD_VECTOR_OF_VEC(3);
        SET_STD_VECTOR_OF_VEC(4);

        // Set from std::vector<> of glm::mat<>
        ///////////////////////////////////////
#define SET_STD_VECTOR_OF_MATRICES(row_count, col_count)\
        template<typename T, typename A, glm::precision P>\
        struct SetStdVector<glm::tmat##row_count##x##col_count<T, P>, A, P>\
        {\
            typedef std::vector<glm::tmat##row_count##x##col_count<T, P>, A> vector_type;\
            static void Set(const GLint ul, const vector_type& v)\
            {\
                const typename vector_type::size_type MaxCount = std::numeric_limits<GLsizei>::max();\
                assert(v.size() <= MaxCount);\
                const auto count = static_cast<GLsizei>(std::min(MaxCount, v.size()));\
                SetMatrices<T, row_count, col_count>::Set(ul, &(v[0])[0][0], count);\
            }\
        }

        SET_STD_VECTOR_OF_MATRICES(2, 2);
        SET_STD_VECTOR_OF_MATRICES(3, 3);
        SET_STD_VECTOR_OF_MATRICES(4, 4);
        SET_STD_VECTOR_OF_MATRICES(2, 3);
        SET_STD_VECTOR_OF_MATRICES(2, 4);
        SET_STD_VECTOR_OF_MATRICES(3, 2);
        SET_STD_VECTOR_OF_MATRICES(3, 4);
        SET_STD_VECTOR_OF_MATRICES(4, 2);
        SET_STD_VECTOR_OF_MATRICES(4, 3);

    } // namespace detail


    // For a single scalar.
    template<typename uniform_t>
    inline void SetUniform(const GLint uniformLocation, const uniform_t v0)
    {
        detail::SetElements<uniform_t>::Set(uniformLocation, v0);
    }
    
    // For two scalars.
    template<typename uniform_t>
    inline void SetUniform(const GLint uniformLocation, const uniform_t v0, const uniform_t v1)
    {
        detail::SetElements<uniform_t>::Set(uniformLocation, v0, v1);
    }
    
    // Three scalars.
    template<typename uniform_t>
    inline void SetUniform(
                           const GLint uniformLocation,
                           const uniform_t v0,
                           const uniform_t v1,
                           const uniform_t v2
                           )
    {
        detail::SetElements<uniform_t>::Set(uniformLocation, v0, v1, v2);
    }
    
    // Four scalars.
    template<typename uniform_t>
    inline void SetUniform(
                           const GLint uniformLocation,
                           const uniform_t v0,
                           const uniform_t v1,
                           const uniform_t v2,
                           const uniform_t v3
                           )
    {
        detail::SetElements<uniform_t>::Set(uniformLocation, v0, v1, v2, v3);
    }

    // Single vector.
    template<typename arithmetic_t, glm::precision P = glm::defaultp>
    inline void SetUniform(const GLint uniformLocation, const glm::tvec2<arithmetic_t, P>& v)
    {
        detail::SetArray<arithmetic_t, 2>::Set(uniformLocation, &v[0], 1);
    }

    template<typename arithmetic_t, glm::precision P = glm::defaultp>
    inline void SetUniform(const GLint uniformLocation, const glm::tvec3<arithmetic_t, P>& v)
    {
        detail::SetArray<arithmetic_t, 3>::Set(uniformLocation, &v[0], 1);
    }

    template<typename arithmetic_t, glm::precision P = glm::defaultp>
    inline void SetUniform(const GLint uniformLocation, const glm::tvec4<arithmetic_t, P>& v)
    {
        detail::SetArray<arithmetic_t, 4>::Set(uniformLocation, &v[0], 1);
    }

    // Single Matrix.
    template<typename arithmetic_t, glm::precision P = glm::defaultp>
    inline void SetUniform(const GLint uniformLocation, const glm::tmat2x2<arithmetic_t, P>& m)
    {
        detail::SetMatrices<arithmetic_t, 2, 2>::Set(uniformLocation, &m[0][0], 1);
    }

    template<typename arithmetic_t, glm::precision P = glm::defaultp>
    inline void SetUniform(const GLint uniformLocation, const glm::tmat3x3<arithmetic_t, P>& m)
    {
        detail::SetMatrices<arithmetic_t, 3, 3>::Set(uniformLocation, &m[0][0], 1);
    }

    template<typename arithmetic_t, glm::precision P = glm::defaultp>
    inline void SetUniform(const GLint uniformLocation, const glm::tmat4x4<arithmetic_t, P>& m)
    {
        detail::SetMatrices<arithmetic_t, 4, 4>::Set(uniformLocation, &m[0][0], 1);
    }

    template<typename arithmetic_t, glm::precision P = glm::defaultp>
    inline void SetUniform(const GLint uniformLocation, const glm::tmat2x3<arithmetic_t, P>& m)
    {
        detail::SetMatrices<arithmetic_t, 2, 3>::Set(uniformLocation, &m[0][0], 1);
    }

    template<typename arithmetic_t, glm::precision P = glm::defaultp>
    inline void SetUniform(const GLint uniformLocation, const glm::tmat2x4<arithmetic_t, P>& m)
    {
        detail::SetMatrices<arithmetic_t, 2, 4>::Set(uniformLocation, &m[0][0], 1);
    }

    template<typename arithmetic_t, glm::precision P = glm::defaultp>
    inline void SetUniform(const GLint uniformLocation, const glm::tmat3x2<arithmetic_t, P>& m)
    {
        detail::SetMatrices<arithmetic_t, 3, 2>::Set(uniformLocation, &m[0][0], 1);
    }

    template<typename arithmetic_t, glm::precision P = glm::defaultp>
    inline void SetUniform(const GLint uniformLocation, const glm::tmat3x4<arithmetic_t, P>& m)
    {
        detail::SetMatrices<arithmetic_t, 3, 4>::Set(uniformLocation, &m[0][0], 1);
    }

    template<typename arithmetic_t, glm::precision P = glm::defaultp>
    inline void SetUniform(const GLint uniformLocation, const glm::tmat4x2<arithmetic_t, P>& m)
    {
        detail::SetMatrices<arithmetic_t, 4, 2>::Set(uniformLocation, &m[0][0], 1);
    }

    template<typename arithmetic_t, glm::precision P = glm::defaultp>
    inline void SetUniform(const GLint uniformLocation, const glm::tmat4x3<arithmetic_t, P>& m)
    {
        detail::SetMatrices<arithmetic_t, 4, 3>::Set(uniformLocation, &m[0][0], 1);
    }

    // Generic for a C-array.
    template<typename uniform_t, std::size_t N>
    inline void SetUniform(const GLint uniformLocation, const uniform_t (&v)[N])
    {
        detail::SetCArray<uniform_t, N>::Set(uniformLocation, v);
    }

    // std::array<>
    template<typename uniform_t, std::size_t N>
    inline void SetUniform(const GLint uniformLocation, const std::array<uniform_t, N>& v)
    {
        detail::SetStdArray<uniform_t, N>::Set(uniformLocation, v);
    }

    // Generic for a buffer pointer and count.
    template<typename uniform_t>
    inline void SetUniform(const GLint uniformLocation, const uniform_t* const v, const GLsizei count)
    {
        assert(count >= 0);
        detail::SetBuffer<uniform_t>::Set(uniformLocation, v, count);
    }

    // std::vector<>
    template<typename uniform_t, typename A>
    inline void SetUniform(const GLint uniformLocation, const std::vector<uniform_t, A>& v)
    {
        detail::SetStdVector<uniform_t, A>::Set(uniformLocation, v);
    }

//    void test()
//    {
//        SetUniform(0, 0.0f);
//        SetUniform(0, 0.0f, 0.0f);
//        SetUniform(0, 0.0f, 0.0f, 0.0f);
//        SetUniform(0, 0.0f, 0.0f, 0.0f, 0.0f);
//        SetUniform(0, glm::vec2());
//        SetUniform(0, glm::vec3());
//        SetUniform(0, glm::vec4());
//        SetUniform(0, glm::mat2());
//        SetUniform(0, glm::mat3());
//        SetUniform(0, glm::mat4());
//        SetUniform(0, glm::mat2x3());
//        SetUniform(0, glm::mat2x4());
//        SetUniform(0, glm::mat3x2());
//        SetUniform(0, glm::mat3x4());
//        SetUniform(0, glm::mat4x2());
//        SetUniform(0, glm::mat4x3());
//
//        SetUniform(0, 0.0);
//        SetUniform(0, 0.0, 0.0);
//        SetUniform(0, 0.0, 0.0, 0.0);
//        SetUniform(0, 0.0, 0.0, 0.0, 0.0);
//        SetUniform(0, glm::dvec2());
//        SetUniform(0, glm::dvec3());
//        SetUniform(0, glm::dvec4());
//        SetUniform(0, glm::dmat2());
//        SetUniform(0, glm::dmat3());
//        SetUniform(0, glm::dmat4());
//        SetUniform(0, glm::dmat2x3());
//        SetUniform(0, glm::dmat2x4());
//        SetUniform(0, glm::dmat3x2());
//        SetUniform(0, glm::dmat3x4());
//        SetUniform(0, glm::dmat4x2());
//        SetUniform(0, glm::dmat4x3());
//
//        SetUniform(0, -1);
//        SetUniform(0, -1, -1);
//        SetUniform(0, -1, -1, -1);
//        SetUniform(0, -1, -1, -1, -1);
//        SetUniform(0, glm::ivec2());
//        SetUniform(0, glm::ivec3());
//        SetUniform(0, glm::ivec4());
//
//        SetUniform(0, 1u);
//        SetUniform(0, 1u, 1u);
//        SetUniform(0, 1u, 1u, 1u);
//        SetUniform(0, 1u, 1u, 1u, 1u);
//        SetUniform(0, glm::uvec2());
//        SetUniform(0, glm::uvec3());
//        SetUniform(0, glm::uvec4());
//
//        SetUniform(0, GL_FALSE);
//        SetUniform(0, GL_FALSE, GL_TRUE);
//        SetUniform(0, GL_FALSE, GL_TRUE, GL_FALSE);
//        SetUniform(0, GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE);
//
//        SetUniform(0, true);
//        SetUniform(0, false, true);
//        SetUniform(0, false, true, false);
//        SetUniform(0, false, true, false, true);
//        SetUniform(0, glm::bvec2());
//        SetUniform(0, glm::bvec3());
//        SetUniform(0, glm::bvec4());
//
//        const GLfloat   caf[] = { 0.0f, 1.0f };
//        const GLdouble  cad[] = { 0.0, 1.0 };
//        const GLint     cai[] = { -1, -1 };
//        const GLuint    cau[] = { 1u, 1u };
//        const GLboolean caglb[] = { GL_FALSE, GL_TRUE };
//        const bool      cab[] = { false, true };
//        const glm::vec2 cav2[] = { glm::vec2(), glm::vec2() };
//        const glm::vec3 cav3[] = { glm::vec3(), glm::vec3() };
//        const glm::vec4 cav4[] = { glm::vec4(), glm::vec4() };
//        const glm::dvec2 cadv2[] = { glm::dvec2(), glm::dvec2() };
//        const glm::dvec3 cadv3[] = { glm::dvec3(), glm::dvec3() };
//        const glm::dvec4 cadv4[] = { glm::dvec4(), glm::dvec4() };
//        const glm::ivec2 caiv2[] = { glm::ivec2(), glm::ivec2() };
//        const glm::ivec3 caiv3[] = { glm::ivec3(), glm::ivec3() };
//        const glm::ivec4 caiv4[] = { glm::ivec4(), glm::ivec4() };
//        const glm::uvec2 cauv2[] = { glm::uvec2(), glm::uvec2() };
//        const glm::uvec3 cauv3[] = { glm::uvec3(), glm::uvec3() };
//        const glm::uvec4 cauv4[] = { glm::uvec4(), glm::uvec4() };
//        const glm::bvec2 cabv2[] = { glm::bvec2(), glm::bvec2() };
//        const glm::bvec3 cabv3[] = { glm::bvec3(), glm::bvec3() };
//        const glm::bvec4 cabv4[] = { glm::bvec4(), glm::bvec4() };
//        const glm::mat2 cam2[] = { glm::mat2(), glm::mat2() };
//        const glm::mat3 cam3[] = { glm::mat3(), glm::mat3() };
//        const glm::mat4 cam4[] = { glm::mat4(), glm::mat4() };
//        const glm::mat2x3 cam2x3[] = { glm::mat2x3(), glm::mat2x3() };
//        const glm::mat2x4 cam2x4[] = { glm::mat2x4(), glm::mat2x4() };
//        const glm::mat3x2 cam3x2[] = { glm::mat3x2(), glm::mat3x2() };
//        const glm::mat3x4 cam3x4[] = { glm::mat3x4(), glm::mat3x4() };
//        const glm::mat4x2 cam4x2[] = { glm::mat4x2(), glm::mat4x2() };
//        const glm::mat4x3 cam4x3[] = { glm::mat4x3(), glm::mat4x3() };
//        const glm::dmat2 cadm2[] = { glm::dmat2(), glm::dmat2() };
//        const glm::dmat3 cadm3[] = { glm::dmat3(), glm::dmat3() };
//        const glm::dmat4 cadm4[] = { glm::dmat4(), glm::dmat4() };
//        const glm::dmat2x3 cadm2x3[] = { glm::dmat2x3(), glm::dmat2x3() };
//        const glm::dmat2x4 cadm2x4[] = { glm::dmat2x4(), glm::dmat2x4() };
//        const glm::dmat3x2 cadm3x2[] = { glm::dmat3x2(), glm::dmat3x2() };
//        const glm::dmat3x4 cadm3x4[] = { glm::dmat3x4(), glm::dmat3x4() };
//        const glm::dmat4x2 cadm4x2[] = { glm::dmat4x2(), glm::dmat4x2() };
//        const glm::dmat4x3 cadm4x3[] = { glm::dmat4x3(), glm::dmat4x3() };
//
//        // C-arrays
//        SetUniform(0, caf);
//        SetUniform(0, cad);
//        SetUniform(0, cai);
//        SetUniform(0, cau);
//        SetUniform(0, caglb);
//        SetUniform(0, cab);
//        SetUniform(0, cav2);
//        SetUniform(0, cav3);
//        SetUniform(0, cav4);
//        SetUniform(0, cadv2);
//        SetUniform(0, cadv3);
//        SetUniform(0, cadv4);
//        SetUniform(0, caiv2);
//        SetUniform(0, caiv3);
//        SetUniform(0, caiv4);
//        SetUniform(0, cauv2);
//        SetUniform(0, cauv3);
//        SetUniform(0, cauv4);
//        SetUniform(0, cabv2);
//        SetUniform(0, cabv3);
//        SetUniform(0, cabv4);
//        SetUniform(0, cam2);
//        SetUniform(0, cam3);
//        SetUniform(0, cam4);
//        SetUniform(0, cam2x3);
//        SetUniform(0, cam2x4);
//        SetUniform(0, cam3x2);
//        SetUniform(0, cam3x4);
//        SetUniform(0, cam4x2);
//        SetUniform(0, cam4x3);
//        SetUniform(0, cadm2);
//        SetUniform(0, cadm3);
//        SetUniform(0, cadm4);
//        SetUniform(0, cadm2x3);
//        SetUniform(0, cadm2x4);
//        SetUniform(0, cadm3x2);
//        SetUniform(0, cadm3x4);
//        SetUniform(0, cadm4x2);
//        SetUniform(0, cadm4x3);
//
//        const std::array<GLfloat, 2>    af = { 0.0f, 1.0f };
//        const std::array<GLdouble, 2>   ad = { 0.0, 1.0 };
//        const std::array<GLint, 2>      ai = { -1, -1 };
//        const std::array<GLuint, 2>     au = { 1u, 1u };
//        const std::array<GLboolean, 2>  aglb = { GL_FALSE, GL_TRUE };
//        const std::array<bool, 2>       ab = { false, true };
//        const std::array<glm::vec2, 2> av2 = { glm::vec2(), glm::vec2() };
//        const std::array<glm::vec3, 2> av3 = { glm::vec3(), glm::vec3() };
//        const std::array<glm::vec4, 2> av4 = { glm::vec4(), glm::vec4() };
//        const std::array<glm::dvec2, 2> adv2 = { glm::dvec2(), glm::dvec2() };
//        const std::array<glm::dvec3, 2> adv3 = { glm::dvec3(), glm::dvec3() };
//        const std::array<glm::dvec4, 2> adv4 = { glm::dvec4(), glm::dvec4() };
//        const std::array<glm::ivec2, 2> aiv2 = { glm::ivec2(), glm::ivec2() };
//        const std::array<glm::ivec3, 2> aiv3 = { glm::ivec3(), glm::ivec3() };
//        const std::array<glm::ivec4, 2> aiv4 = { glm::ivec4(), glm::ivec4() };
//        const std::array<glm::uvec2, 2> auv2 = { glm::uvec2(), glm::uvec2() };
//        const std::array<glm::uvec3, 2> auv3 = { glm::uvec3(), glm::uvec3() };
//        const std::array<glm::uvec4, 2> auv4 = { glm::uvec4(), glm::uvec4() };
//        const std::array<glm::bvec2, 2> abv2 = { glm::bvec2(), glm::bvec2() };
//        const std::array<glm::bvec3, 2> abv3 = { glm::bvec3(), glm::bvec3() };
//        const std::array<glm::bvec4, 2> abv4 = { glm::bvec4(), glm::bvec4() };
//        const std::array<glm::mat2, 2> am2 = { glm::mat2(), glm::mat2() };
//        const std::array<glm::mat3, 2> am3 = { glm::mat3(), glm::mat3() };
//        const std::array<glm::mat4, 2> am4 = { glm::mat4(), glm::mat4() };
//        const std::array<glm::mat2x3, 2> am2x3 = { glm::mat2x3(), glm::mat2x3() };
//        const std::array<glm::mat2x4, 2> am2x4 = { glm::mat2x4(), glm::mat2x4() };
//        const std::array<glm::mat3x2, 2> am3x2 = { glm::mat3x2(), glm::mat3x2() };
//        const std::array<glm::mat3x4, 2> am3x4 = { glm::mat3x4(), glm::mat3x4() };
//        const std::array<glm::mat4x2, 2> am4x2 = { glm::mat4x2(), glm::mat4x2() };
//        const std::array<glm::mat4x3, 2> am4x3 = { glm::mat4x3(), glm::mat4x3() };
//        const std::array<glm::dmat2, 2> adm2 = { glm::dmat2(), glm::dmat2() };
//        const std::array<glm::dmat3, 2> adm3 = { glm::dmat3(), glm::dmat3() };
//        const std::array<glm::dmat4, 2> adm4 = { glm::dmat4(), glm::dmat4() };
//        const std::array<glm::dmat2x3, 2> adm2x3 = { glm::dmat2x3(), glm::dmat2x3() };
//        const std::array<glm::dmat2x4, 2> adm2x4 = { glm::dmat2x4(), glm::dmat2x4() };
//        const std::array<glm::dmat3x2, 2> adm3x2 = { glm::dmat3x2(), glm::dmat3x2() };
//        const std::array<glm::dmat3x4, 2> adm3x4 = { glm::dmat3x4(), glm::dmat3x4() };
//        const std::array<glm::dmat4x2, 2> adm4x2 = { glm::dmat4x2(), glm::dmat4x2() };
//        const std::array<glm::dmat4x3, 2> adm4x3 = { glm::dmat4x3(), glm::dmat4x3() };
//
//        // std::array<>
//        SetUniform(0, af);
//        SetUniform(0, ad);
//        SetUniform(0, ai);
//        SetUniform(0, au);
//        SetUniform(0, aglb);
//        SetUniform(0, ab);
//        SetUniform(0, av2);
//        SetUniform(0, av3);
//        SetUniform(0, av4);
//        SetUniform(0, adv2);
//        SetUniform(0, adv3);
//        SetUniform(0, adv4);
//        SetUniform(0, aiv2);
//        SetUniform(0, aiv3);
//        SetUniform(0, aiv4);
//        SetUniform(0, auv2);
//        SetUniform(0, auv3);
//        SetUniform(0, auv4);
//        SetUniform(0, abv2);
//        SetUniform(0, abv3);
//        SetUniform(0, abv4);
//        SetUniform(0, am2);
//        SetUniform(0, am3);
//        SetUniform(0, am4);
//        SetUniform(0, am2x3);
//        SetUniform(0, am2x4);
//        SetUniform(0, am3x2);
//        SetUniform(0, am3x4);
//        SetUniform(0, am4x2);
//        SetUniform(0, am4x3);
//        SetUniform(0, adm2);
//        SetUniform(0, adm3);
//        SetUniform(0, adm4);
//        SetUniform(0, adm2x3);
//        SetUniform(0, adm2x4);
//        SetUniform(0, adm3x2);
//        SetUniform(0, adm3x4);
//        SetUniform(0, adm4x2);
//        SetUniform(0, adm4x3);
//
//        // buffers
//        SetUniform(0, &af[0], static_cast<GLsizei>(af.size()));
//        SetUniform(0, &ad[0], static_cast<GLsizei>(ad.size()));
//        SetUniform(0, &ai[0], static_cast<GLsizei>(ai.size()));
//        SetUniform(0, &au[0], static_cast<GLsizei>(au.size()));
//        SetUniform(0, &aglb[0], static_cast<GLsizei>(aglb.size()));
//        SetUniform(0, &ab[0], static_cast<GLsizei>(ab.size()));
//        SetUniform(0, &av2[0], static_cast<GLsizei>(av2.size()));
//        SetUniform(0, &av3[0], static_cast<GLsizei>(av3.size()));
//        SetUniform(0, &av4[0], static_cast<GLsizei>(av4.size()));
//        SetUniform(0, &adv2[0], static_cast<GLsizei>(adv2.size()));
//        SetUniform(0, &adv3[0], static_cast<GLsizei>(adv3.size()));
//        SetUniform(0, &adv4[0], static_cast<GLsizei>(adv4.size()));
//        SetUniform(0, &aiv2[0], static_cast<GLsizei>(aiv2.size()));
//        SetUniform(0, &aiv3[0], static_cast<GLsizei>(aiv3.size()));
//        SetUniform(0, &aiv4[0], static_cast<GLsizei>(aiv4.size()));
//        SetUniform(0, &auv2[0], static_cast<GLsizei>(auv2.size()));
//        SetUniform(0, &auv3[0], static_cast<GLsizei>(auv3.size()));
//        SetUniform(0, &auv4[0], static_cast<GLsizei>(auv4.size()));
//        SetUniform(0, &abv2[0], static_cast<GLsizei>(abv2.size()));
//        SetUniform(0, &abv3[0], static_cast<GLsizei>(abv3.size()));
//        SetUniform(0, &abv4[0], static_cast<GLsizei>(abv4.size()));
//        SetUniform(0, &am2[0], static_cast<GLsizei>(am2.size()));
//        SetUniform(0, &am3[0], static_cast<GLsizei>(am3.size()));
//        SetUniform(0, &am4[0], static_cast<GLsizei>(am4.size()));
//        SetUniform(0, &am2x3[0], static_cast<GLsizei>(am2x3.size()));
//        SetUniform(0, &am2x4[0], static_cast<GLsizei>(am2x4.size()));
//        SetUniform(0, &am3x2[0], static_cast<GLsizei>(am3x2.size()));
//        SetUniform(0, &am3x4[0], static_cast<GLsizei>(am3x4.size()));
//        SetUniform(0, &am4x2[0], static_cast<GLsizei>(am4x2.size()));
//        SetUniform(0, &am4x3[0], static_cast<GLsizei>(am4x3.size()));
//        SetUniform(0, &adm2[0], static_cast<GLsizei>(adm2.size()));
//        SetUniform(0, &adm3[0], static_cast<GLsizei>(adm3.size()));
//        SetUniform(0, &adm4[0], static_cast<GLsizei>(adm4.size()));
//        SetUniform(0, &adm2x3[0], static_cast<GLsizei>(adm2x3.size()));
//        SetUniform(0, &adm2x4[0], static_cast<GLsizei>(adm2x4.size()));
//        SetUniform(0, &adm3x2[0], static_cast<GLsizei>(adm3x2.size()));
//        SetUniform(0, &adm3x4[0], static_cast<GLsizei>(adm3x4.size()));
//        SetUniform(0, &adm4x2[0], static_cast<GLsizei>(adm4x2.size()));
//        SetUniform(0, &adm4x3[0], static_cast<GLsizei>(adm4x3.size()));
//
//        const std::vector<GLfloat>      vf = { 0.0f, 1.0f };
//        const std::vector<GLdouble>     vd = { 0.0, 1.0 };
//        const std::vector<GLint>        vi = { -1, -1 };
//        const std::vector<GLuint>       vu = { 1u, 1u };
//        const std::vector<GLboolean>    vglb = { GL_FALSE, GL_TRUE };
//        const std::vector<bool>         vb = { false, true };
//        const std::vector<glm::vec2>    vv2 = { glm::vec2(), glm::vec2() };
//        const std::vector<glm::vec3>    vv3 = { glm::vec3(), glm::vec3() };
//        const std::vector<glm::vec4>    vv4 = { glm::vec4(), glm::vec4() };
//        const std::vector<glm::dvec2>   vdv2 = { glm::dvec2(), glm::dvec2() };
//        const std::vector<glm::dvec3>   vdv3 = { glm::dvec3(), glm::dvec3() };
//        const std::vector<glm::dvec4>   vdv4 = { glm::dvec4(), glm::dvec4() };
//        const std::vector<glm::ivec2>   viv2 = { glm::ivec2(), glm::ivec2() };
//        const std::vector<glm::ivec3>   viv3 = { glm::ivec3(), glm::ivec3() };
//        const std::vector<glm::ivec4>   viv4 = { glm::ivec4(), glm::ivec4() };
//        const std::vector<glm::uvec2>   vuv2 = { glm::uvec2(), glm::uvec2() };
//        const std::vector<glm::uvec3>   vuv3 = { glm::uvec3(), glm::uvec3() };
//        const std::vector<glm::uvec4>   vuv4 = { glm::uvec4(), glm::uvec4() };
//        const std::vector<glm::bvec2>   vbv2 = { glm::bvec2(), glm::bvec2() };
//        const std::vector<glm::bvec3>   vbv3 = { glm::bvec3(), glm::bvec3() };
//        const std::vector<glm::bvec4>   vbv4 = { glm::bvec4(), glm::bvec4() };
//        const std::vector<glm::mat2>    vm2 = { glm::mat2(), glm::mat2() };
//        const std::vector<glm::mat3>    vm3 = { glm::mat3(), glm::mat3() };
//        const std::vector<glm::mat4>    vm4 = { glm::mat4(), glm::mat4() };
//        const std::vector<glm::mat2x3>  vm2x3 = { glm::mat2x3(), glm::mat2x3() };
//        const std::vector<glm::mat2x4>  vm2x4 = { glm::mat2x4(), glm::mat2x4() };
//        const std::vector<glm::mat3x2>  vm3x2 = { glm::mat3x2(), glm::mat3x2() };
//        const std::vector<glm::mat3x4>  vm3x4 = { glm::mat3x4(), glm::mat3x4() };
//        const std::vector<glm::mat4x2>  vm4x2 = { glm::mat4x2(), glm::mat4x2() };
//        const std::vector<glm::mat4x3>  vm4x3 = { glm::mat4x3(), glm::mat4x3() };
//        const std::vector<glm::dmat2>   vdm2 = { glm::dmat2(), glm::dmat2() };
//        const std::vector<glm::dmat3>   vdm3 = { glm::dmat3(), glm::dmat3() };
//        const std::vector<glm::dmat4>   vdm4 = { glm::dmat4(), glm::dmat4() };
//        const std::vector<glm::dmat2x3> vdm2x3 = { glm::dmat2x3(), glm::dmat2x3() };
//        const std::vector<glm::dmat2x4> vdm2x4 = { glm::dmat2x4(), glm::dmat2x4() };
//        const std::vector<glm::dmat3x2> vdm3x2 = { glm::dmat3x2(), glm::dmat3x2() };
//        const std::vector<glm::dmat3x4> vdm3x4 = { glm::dmat3x4(), glm::dmat3x4() };
//        const std::vector<glm::dmat4x2> vdm4x2 = { glm::dmat4x2(), glm::dmat4x2() };
//        const std::vector<glm::dmat4x3> vdm4x3 = { glm::dmat4x3(), glm::dmat4x3() };
//
//        // std::vector<>
//        SetUniform(0, vf);
//        SetUniform(0, vd);
//        SetUniform(0, vi);
//        SetUniform(0, vu);
//        SetUniform(0, vglb);
//        SetUniform(0, vb);
//        SetUniform(0, vv2);
//        SetUniform(0, vv3);
//        SetUniform(0, vv4);
//        SetUniform(0, vdv2);
//        SetUniform(0, vdv3);
//        SetUniform(0, vdv4);
//        SetUniform(0, viv2);
//        SetUniform(0, viv3);
//        SetUniform(0, viv4);
//        SetUniform(0, vuv2);
//        SetUniform(0, vuv3);
//        SetUniform(0, vuv4);
//        SetUniform(0, vbv2);
//        SetUniform(0, vbv3);
//        SetUniform(0, vbv4);
//        SetUniform(0, vm2);
//        SetUniform(0, vm3);
//        SetUniform(0, vm4);
//        SetUniform(0, vm2x3);
//        SetUniform(0, vm2x4);
//        SetUniform(0, vm3x2);
//        SetUniform(0, vm3x4);
//        SetUniform(0, vm4x2);
//        SetUniform(0, vm4x3);
//        SetUniform(0, vdm2);
//        SetUniform(0, vdm3);
//        SetUniform(0, vdm4);
//        SetUniform(0, vdm2x3);
//        SetUniform(0, vdm2x4);
//        SetUniform(0, vdm3x2);
//        SetUniform(0, vdm3x4);
//        SetUniform(0, vdm4x2);
//        SetUniform(0, vdm4x3);

//        SetUniform<glm::mat2, RowMajor>(0, glm::mat2());
//        SetUniform<glm::mat3, RowMajor>(0, glm::mat3());
//        SetUniform<glm::mat4, RowMajor>(0, glm::mat4());
//        SetUniform<glm::mat2x3, RowMajor>(0, glm::mat2x3());
//        SetUniform<glm::mat2x4, RowMajor>(0, glm::mat2x4());
//        SetUniform<glm::mat3x2, RowMajor>(0, glm::mat3x2());
//        SetUniform<glm::mat3x4, RowMajor>(0, glm::mat3x4());
//        SetUniform<glm::mat4x2, RowMajor>(0, glm::mat4x2());
//        SetUniform<glm::mat4x3, RowMajor>(0, glm::mat4x3());
//    }
    
} } // namespace gl::impl


#endif
