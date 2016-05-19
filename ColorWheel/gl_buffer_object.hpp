//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#ifndef GL_BUFFER_OBJECT_HPP
#define GL_BUFFER_OBJECT_HPP

#include <utility>
#include <opengl/gl3.h>
#include "gl_exceptions.hpp"


namespace gl { namespace impl
{

    inline GLuint GenerateBufferObjectName() noexcept
    {
        GLuint result = 0;
        ::glGenBuffers(1, &result);
        return result;
    }

    template<GLsizei count>
    inline void GenerateBufferObjectNames(GLuint (&bufferObjectNames)[count]) noexcept
    {
        assert(count >= 1);
        ::glGenBuffers(count, bufferObjectNames);
    }

    inline void DeleteBufferObjectName(const GLuint bufferObjectName) noexcept
    {
        ::glDeleteBuffers(1, &bufferObjectName);
    }

    template<GLsizei count>
    inline void DeleteBufferObjectNames(const GLuint (&bufferObjectNames)[count]) noexcept
    {
        assert(count >= 1);
        ::glDeleteBuffers(count, bufferObjectNames);
    }

} // namespace impl

    enum class BufferObjectType
        : GLenum
    {
        Invalid             = GL_INVALID_ENUM,
        Array               = GL_ARRAY_BUFFER,
        CopyRead            = GL_COPY_READ_BUFFER,
        CopyWrite           = GL_COPY_WRITE_BUFFER,
        ElementArray        = GL_ELEMENT_ARRAY_BUFFER,
        PixelPack           = GL_PIXEL_PACK_BUFFER,
        PixelUnpack         = GL_PIXEL_UNPACK_BUFFER,
        TextureBuffer       = GL_TEXTURE_BUFFER,
        TransformFeedback   = GL_TRANSFORM_FEEDBACK_BUFFER,
        Uniform             = GL_UNIFORM_BUFFER,
    };

    // Convert the BufferObjectType enumeration to a string.
    inline std::string BufferObjectTypeToString(const BufferObjectType bufferObjectType)
    {
        switch (bufferObjectType)
        {
            case BufferObjectType::Array:
                return "Array";
            case BufferObjectType::CopyRead:
                return "CopyRead";
            case BufferObjectType::CopyWrite:
                return "CopyWrite";
            case BufferObjectType::ElementArray:
                return "ElementArray";
            case BufferObjectType::PixelPack:
                return "PixelPack";
            case BufferObjectType::PixelUnpack:
                return "PixelUnpack";
            case BufferObjectType::TextureBuffer:
                return "TextureBuffer";
            case BufferObjectType::TransformFeedback:
                return "TransformFeedback";
            case BufferObjectType::Uniform:
                return "Uniform";
            default:
                break;
        }
        return "Invalid";
    }

    enum class UsagePattern
        : GLenum
    {
        Invalid     = GL_INVALID_ENUM,
        StreamDraw  = GL_STREAM_DRAW,
        StreamRead  = GL_STREAM_READ,
        StreamCopy  = GL_STREAM_COPY,
        StaticDraw  = GL_STATIC_DRAW,
        StaticRead  = GL_STATIC_READ,
        StaticCopy  = GL_STATIC_COPY,
        DynamicDraw = GL_DYNAMIC_DRAW,
        DynamicRead = GL_DYNAMIC_READ,
        DynamicCopy = GL_DYNAMIC_COPY,
    };

    // Convert the data store UsagePattern enumeration to a string.
    inline std::string UsagePatternToString(const UsagePattern usagePattern)
    {
        switch (usagePattern)
        {
            case UsagePattern::StreamDraw:
                return "StreamDraw";
            case UsagePattern::StreamRead:
                return "StreamRead";
            case UsagePattern::StreamCopy:
                return "StreamCopy";
            case UsagePattern::StaticDraw:
                return "StaticDraw";
            case UsagePattern::StaticRead:
                return "StaticRead";
            case UsagePattern::StaticCopy:
                return "StaticCopy";
            case UsagePattern::DynamicDraw:
                return "DynamicDraw";
            case UsagePattern::DynamicRead:
                return "DynamicRead";
            case UsagePattern::DynamicCopy:
                return "DynamicCopy";
            default:
                break;
        }
        return "Invalid";
    }

    enum class AccessPolicy
        : GLenum
    {
        Invalid     = GL_INVALID_ENUM,
        ReadOnly    = GL_READ_ONLY,
        WriteOnly   = GL_WRITE_ONLY,
        ReadWrite   = GL_READ_WRITE,
    };

    // Convert the AccessPolicy enumeration to a string.
    inline std::string AccessPolicyToString(const AccessPolicy accessPolicy)
    {
        switch (accessPolicy)
        {
            case AccessPolicy::ReadOnly:
                return "ReadOnly";
            case AccessPolicy::WriteOnly:
                return "WriteOnly";
            case AccessPolicy::ReadWrite:
                return "ReadWrite";
            default:
                break;
        }
        return "Invalid";
    }

    enum class BufferParameter
        : GLenum
    {
        Invalid = GL_INVALID_ENUM,
        Access  = GL_BUFFER_ACCESS,
        Mapped  = GL_BUFFER_MAPPED,
        Size    = GL_BUFFER_SIZE,
        Usage   = GL_BUFFER_USAGE,
    };

    // Convert the BufferParameter enumeration to a string.
    inline std::string BufferParameterToString(const BufferParameter bufferParameter)
    {
        switch (bufferParameter)
        {
            case BufferParameter::Access:
                return "Access";
            case BufferParameter::Mapped:
                return "Mapped";
            case BufferParameter::Size:
                return "Size";
            case BufferParameter::Usage:
                return "Usage";
            default:
                break;
        }
        return "Invalid";
    }

    // OpenGL Buffer Object Name wrapper
    ////////////////////////////////////
    class BufferObjectName
    {
    public:
        typedef GLuint          value_type;
        typedef value_type&     reference;

        BufferObjectName()
            : _handle(impl::GenerateBufferObjectName())
        {
            if (0u == _handle)
            {
                throw std::runtime_error("failed to generate an OpenGL buffer object name");
            }
        }

        BufferObjectName(BufferObjectName&& original) noexcept
            : _handle(0u)
        {
            std::swap(original._handle, _handle);
        }

        virtual ~BufferObjectName()
        {
            impl::DeleteBufferObjectName(_handle);
        }

        BufferObjectName& operator =(BufferObjectName&& rhs) noexcept
        {
            if (&rhs == this)
            {
                return *this;
            }

            impl::DeleteBufferObjectName(_handle);
            _handle = 0u;

            std::swap(rhs._handle, _handle);

            return *this;
        }

        operator value_type() const noexcept
        {
            return _handle;
        }

        // Not copyable.
        BufferObjectName(const BufferObjectName&) = delete;
        BufferObjectName& operator =(const BufferObjectName&) = delete;

    private:
        value_type _handle;
    };

    // OpenGL Buffer Object wrapper
    ///////////////////////////////
    template<BufferObjectType buffer_object_t>
    class BufferObject
    {
    public:
        BufferObject()
        {
        }
        
        BufferObject(BufferObject&& original)
            : _name(std::move(original._name))
        {
        }

        virtual ~BufferObject()
        {
        }

        BufferObject& operator =(BufferObject&& rhs)
        {
            if (&rhs == this)
            {
                return *this;
            }

            _name = std::move(rhs._name);

            return *this;
        }

        GLuint Name() const noexcept
        {
            return _name;
        }

        BufferObjectType Type() const noexcept
        {
            return buffer_object_t;
        }

        void Bind() const noexcept
        {
            ::glBindBuffer(static_cast<GLenum>(buffer_object_t), _name);
        }

        static void Unbind() noexcept
        {
            ::glBindBuffer(static_cast<GLenum>(buffer_object_t), 0);
        }

        // Throws:  InvalidOperation : Bind() wasn't called before this method.
        //          OutOfMemory : data is too large for OpenGL.
        template<typename T, std::size_t N>
        void Data(const UsagePattern usagePattern, const T (&data)[N])
        {
            static_assert(N <= std::numeric_limits<GLsizeiptr>::max() / sizeof(T), "array is too large for OpenGL");
            ::glBufferData(
                static_cast<GLenum>(buffer_object_t),
                N * sizeof(T),
                data,
                static_cast<GLenum>(usagePattern)
                );

            // NOTE: An InvalidEnum or InvalidValue here indicates a programming error.
            VerifyNoErrors();
        }

        // Throws:  InvalidOperation : Bind() wasn't called before this method.
        //          OutOfMemory : data is too large for OpenGL.
        template<typename T, std::size_t N>
        void Data(const UsagePattern usagePattern, const std::array<T, N>& data)
        {
            static_assert(N <= std::numeric_limits<GLsizeiptr>::max() / sizeof(T), "array is too large for OpenGL");
            ::glBufferData(
                static_cast<GLenum>(buffer_object_t),
                N * sizeof(T),
                &data[0],
                static_cast<GLenum>(usagePattern)
                );

            // NOTE: An InvalidEnum or InvalidValue here indicates a programming error.
            VerifyNoErrors();
        }

        // Throws:  InvalidValue : count is a negative number.
        //          InvalidOperation : Bind() wasn't called before this method.
        //          OutOfMemory : buffer is too large for OpenGL.
        template<typename T>
        void Data(
            const UsagePattern usagePattern,
            const T* const buffer,
            const std::size_t count
            )
        {
            assert(count <= std::numeric_limits<GLsizeiptr>::max() / sizeof(T));
            const GLsizeiptr size = std::min<std::size_t>(
                count * sizeof(T), std::numeric_limits<GLsizeiptr>::max());

            ::glBufferData(
                static_cast<GLenum>(buffer_object_t),
                size,
                buffer,
                static_cast<GLenum>(usagePattern)
                );

            // NOTE: An InvalidEnum here indicates a programming error.
            VerifyNoErrors();
        }

        // Throws:  InvalidValue : count is a negative number.
        //          InvalidOperation : Bind() wasn't called before this method.
        //          OutOfMemory : vector is too large for OpenGL.
        template<typename T, typename A>
        void Data(const UsagePattern usagePattern, const std::vector<T, A>& v)
        {
            assert(v.size() <= std::numeric_limits<GLsizeiptr>::max() / sizeof(T));
            const GLsizeiptr size = std::min<std::size_t>(
                v.size() * sizeof(T), std::numeric_limits<GLsizeiptr>::max());

            ::glBufferData(
                static_cast<GLenum>(buffer_object_t),
                size,
                &v[0],
                static_cast<GLenum>(usagePattern)
                );

            // NOTE: An InvalidEnum here indicates a programming error.
            VerifyNoErrors();
        }

        template<typename A>
        void Data(const UsagePattern usagePattern, const std::vector<bool, A>& v)
        {
            assert(v.size() <= std::numeric_limits<GLsizeiptr>::max() / sizeof(GLint));
            const GLsizeiptr size = std::min<std::size_t>(
                v.size() * sizeof(GLint), std::numeric_limits<GLsizeiptr>::max());

            std::vector<GLint> data(size);
            std::transform(v.cbegin(), v.cend(), data.begin(),
                           [](const bool b) { return b ? GL_TRUE : GL_FALSE; });

            ::glBufferData(
                static_cast<GLenum>(buffer_object_t),
                size,
                &data[0],
                static_cast<GLenum>(usagePattern)
                );

            // NOTE: An InvalidEnum here indicates a programming error.
            VerifyNoErrors();
        }

        // Throws:  InvalidValue : elementOffset + N are beyond the buffer object's allocated data
        //              store.
        //          InvalidOperation : Bind() wasn't called before this method; -or-, the buffer
        //              object is mapped.
        template<typename T, std::size_t N>
        void SubData(const std::size_t elementOffset, const T (&data)[N])
        {
            // check for overflow
            static_assert(N <= std::numeric_limits<GLsizeiptr>::max() / sizeof(T), "array is too large for OpenGL");
            assert(elementOffset <= std::numeric_limits<GLintptr>::max() / sizeof(T));

            // clip in release mode
            const GLintptr offset = std::min<std::size_t>(
                elementOffset * sizeof(T), std::numeric_limits<GLintptr>::max());
            const GLsizeiptr size = N * sizeof(T);

            ::glBufferSubData(
                static_cast<GLenum>(buffer_object_t),
                offset,
                size,
                data
                );

            // NOTE: An InvalidEnum here indicates a programming error.
            VerifyNoErrors();
        }

        // Throws:  InvalidValue : elementOffset + N are beyond the buffer object's allocated data
        //              store.
        //          InvalidOperation : Bind() wasn't called before this method; -or-, the buffer
        //              object is mapped.
        template<typename T, std::size_t N>
        void SubData(const std::size_t elementOffset, const std::array<T, N>& data)
        {
            // check for overflow
            static_assert(N <= std::numeric_limits<GLsizeiptr>::max() / sizeof(T), "array is too large for OpenGL");
            assert(elementOffset <= std::numeric_limits<GLintptr>::max() / sizeof(T));

            // clip in release mode
            const GLintptr offset = std::min<std::size_t>(
                elementOffset * sizeof(T), std::numeric_limits<GLintptr>::max());
            const GLsizeiptr size = N * sizeof(T);

            ::glBufferSubData(
                static_cast<GLenum>(buffer_object_t),
                offset,
                size,
                &data[0]
                );

            // NOTE: An InvalidEnum here indicates a programming error.
            VerifyNoErrors();
        }

        // Throws:  InvalidValue : elementOffset + count are beyond the buffer object's allocated
        //              data store.
        //          InvalidOperation : Bind() wasn't called before this method; -or-, the buffer
        //              object is mapped.
        template<typename T>
        void SubData(
            const std::size_t elementOffset,
            const T* const buffer,
            const std::size_t count
            )
        {
            // check for overflow
            assert(elementOffset <= std::numeric_limits<GLintptr>::max() / sizeof(T));
            assert(count <= std::numeric_limits<GLsizeiptr>::max() / sizeof(T));

            // clip in release mode
            const GLintptr offset = std::min<std::size_t>(
                elementOffset * sizeof(T), std::numeric_limits<GLintptr>::max());
            const GLsizeiptr size = std::min<std::size_t>(
                count * sizeof(T), std::numeric_limits<GLsizeiptr>::max());

            ::glBufferSubData(
                static_cast<GLenum>(buffer_object_t),
                offset,
                size,
                buffer
                );

            // NOTE: An InvalidEnum here indicates a programming error.
            VerifyNoErrors();
        }

        // Throws:  InvalidValue : elementOffset + N are beyond the buffer object's allocated data
        //              store.
        //          InvalidOperation : Bind() wasn't called before this method; -or-, the buffer
        //              object is mapped.
        template<typename T, typename A>
        void SubData(const std::size_t elementOffset, const std::vector<T, A>& v)
        {
            // check for overflow
            assert(v.size() <= std::numeric_limits<GLsizeiptr>::max() / sizeof(T));
            assert(elementOffset <= std::numeric_limits<GLintptr>::max() / sizeof(T));

            // clip in release mode
            const GLintptr offset = std::min<std::size_t>(
                elementOffset * sizeof(T), std::numeric_limits<GLintptr>::max());
            const GLsizeiptr size = v.size() * sizeof(T);

            ::glBufferSubData(
                static_cast<GLenum>(buffer_object_t),
                offset,
                size,
                &v[0]
                );

            // NOTE: An InvalidEnum here indicates a programming error.
            VerifyNoErrors();
        }

        // Throws:  InvalidValue : elementOffset + N are beyond the buffer object's allocated data
        //              store.
        //          InvalidOperation : Bind() wasn't called before this method; -or-, the buffer
        //              object is mapped.
        template<typename A>
        void SubData(const std::size_t elementOffset, const std::vector<bool, A>& v)
        {
            // check for overflow
            assert(v.size() <= std::numeric_limits<GLsizeiptr>::max() / sizeof(GLint));
            assert(elementOffset <= std::numeric_limits<GLintptr>::max() / sizeof(GLint));

            // clip in release mode
            const GLintptr offset = std::min<std::size_t>(
                elementOffset * sizeof(GLint), std::numeric_limits<GLintptr>::max());
            const GLsizeiptr size = v.size() * sizeof(GLint);

            std::vector<GLint> data(size);
            std::transform(v.cbegin(), v.cend(), data.begin(),
                           [](const bool b) { return b ? GL_TRUE : GL_FALSE; });

            ::glBufferSubData(
                static_cast<GLenum>(buffer_object_t),
                offset,
                size,
                &data[0]
                );

            // NOTE: An InvalidEnum here indicates a programming error.
            VerifyNoErrors();
        }

        // Throws:  InvalidEnum : buffer type is not GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER,
        //              GL_PIXEL_PACK_BUFFER, or GL_PIXEL_UNPACK_BUFFER.
        //          InvalidValue : elementOffset + N are beyond the buffer object's allocated data
        //              store.
        //          InvalidOperation : Bind() wasn't called before this method; -or-, the buffer
        //              object is mapped.
        template<typename T, std::size_t N>
        void GetSubData(const std::size_t elementOffset, T (&data)[N]) const
        {
            // check for overflow
            static_assert(N <= std::numeric_limits<GLsizeiptr>::max() / sizeof(T), "array is too large for OpenGL");
            assert(elementOffset <= std::numeric_limits<GLintptr>::max() / sizeof(T));

            // clip in release mode
            const GLintptr offset = std::min<std::size_t>(
                elementOffset * sizeof(T), std::numeric_limits<GLintptr>::max());
            const GLsizeiptr size = N * sizeof(T);

            ::glGetBufferSubData(
                              static_cast<GLenum>(buffer_object_t),
                              offset,
                              size,
                              data
                              );

            VerifyNoErrors();
        }

        // Throws:  InvalidEnum : buffer type is not GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER,
        //              GL_PIXEL_PACK_BUFFER, or GL_PIXEL_UNPACK_BUFFER.
        //          InvalidValue : elementOffset + N are beyond the buffer object's allocated data
        //              store.
        //          InvalidOperation : Bind() wasn't called before this method; -or-, the buffer
        //              object is mapped.
        template<typename T, std::size_t N>
        void GetSubData(const std::size_t elementOffset, std::array<T, N>& data) const
        {
            // check for overflow
            static_assert(N <= std::numeric_limits<GLsizeiptr>::max() / sizeof(T), "array is too large for OpenGL");
            assert(elementOffset <= std::numeric_limits<GLintptr>::max() / sizeof(T));

            // clip in release mode
            const GLintptr offset = std::min<std::size_t>(
                elementOffset * sizeof(T), std::numeric_limits<GLintptr>::max());
            const GLsizeiptr size = N * sizeof(T);

            ::glGetBufferSubData(
                static_cast<GLenum>(buffer_object_t),
                offset,
                size,
                &data[0]
                );

            VerifyNoErrors();
        }

        // Throws:  InvalidEnum : buffer type is not GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER,
        //              GL_PIXEL_PACK_BUFFER, or GL_PIXEL_UNPACK_BUFFER.
        //          InvalidValue : elementOffset + N are beyond the buffer object's allocated data
        //              store.
        //          InvalidOperation : Bind() wasn't called before this method; -or-, the buffer
        //              object is mapped.
        template<typename T, std::size_t N>
        void GetSubData(
            const std::size_t elementOffset,
            const T* const buffer,
            const std::size_t count
            ) const
        {
            // check for overflow
            assert(count <= std::numeric_limits<GLsizeiptr>::max() / sizeof(T));
            assert(elementOffset <= std::numeric_limits<GLintptr>::max() / sizeof(T));

            // clip in release mode
            const GLintptr offset = std::min<std::size_t>(
                elementOffset * sizeof(T), std::numeric_limits<GLintptr>::max());
            const GLsizeiptr size = count * sizeof(T);

            ::glGetBufferSubData(
                static_cast<GLenum>(buffer_object_t),
                offset,
                size,
                buffer
                );

            VerifyNoErrors();
        }

        // Throws:  InvalidEnum : buffer type is not GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER,
        //              GL_PIXEL_PACK_BUFFER, or GL_PIXEL_UNPACK_BUFFER.
        //          InvalidValue : elementOffset + N are beyond the buffer object's allocated data
        //              store.
        //          InvalidOperation : Bind() wasn't called before this method; -or-, the buffer
        //              object is mapped.
        template<typename T, typename A>
        void GetSubData(const std::size_t elementOffset, std::vector<T, A>& v) const
        {
            // check for overflow
            assert(v.size() <= std::numeric_limits<GLsizeiptr>::max() / sizeof(T));
            assert(elementOffset <= std::numeric_limits<GLintptr>::max() / sizeof(T));

            // clip in release mode
            const GLintptr offset = std::min<std::size_t>(
                elementOffset * sizeof(T), std::numeric_limits<GLintptr>::max());
            const GLsizeiptr size = v.size() * sizeof(T);

            ::glGetBufferSubData(
                static_cast<GLenum>(buffer_object_t),
                offset,
                size,
                &v[0]
                );

            VerifyNoErrors();
        }

        // Throws:  InvalidEnum : buffer type is not GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER,
        //              GL_PIXEL_PACK_BUFFER, or GL_PIXEL_UNPACK_BUFFER.
        //          InvalidValue : elementOffset + N are beyond the buffer object's allocated data
        //              store.
        //          InvalidOperation : Bind() wasn't called before this method; -or-, the buffer
        //              object is mapped.
        template<typename A>
        void GetSubData(const std::size_t elementOffset, std::vector<bool, A>& v) const
        {
            // check for overflow
            assert(v.size() <= std::numeric_limits<GLsizeiptr>::max() / sizeof(GLint));
            assert(elementOffset <= std::numeric_limits<GLintptr>::max() / sizeof(GLint));

            // clip in release mode
            const GLintptr offset = std::min<std::size_t>(
                elementOffset * sizeof(GLint), std::numeric_limits<GLintptr>::max());
            const GLsizeiptr size = v.size() * sizeof(GLint);

            std::vector<GLint> data(size);

            ::glGetBufferSubData(
                static_cast<GLenum>(buffer_object_t),
                offset,
                size,
                &data[0]
                );

            VerifyNoErrors();

            std::transform(data.cbegin(), data.cend(), v.begin(),
                           [](const GLint i) { return GL_FALSE != i; });
        }

        // Throws:  InvalidEnum : buffer type is not GL_TRANSFORM_FEEDBACK_BUFFER or
        //              GL_UNIFORM_BUFFER.
        //          InvalidValue : index is greater than or equal to the number of target-specific
        //              indexed binding points; -or-, buffer does not have an associated data store,
        //              or if the size of that store is zero.
        void BindBase(const GLuint index)
        {
            ::glBindBufferBase(static_cast<GLenum>(buffer_object_t), index, _name);
            VerifyNoErrors();
        }

        // Throws:  InvalidEnum : buffer type is not GL_TRANSFORM_FEEDBACK_BUFFER or
        //              GL_UNIFORM_BUFFER.
        //          InvalidValue : index is greater than or equal to the number of target-specific
        //              indexed binding points; -or-, buffer does not have an associated data store,
        //              the size of that store is zero, or elementOffset + elementCount >
        //              GL_BUFFER_SIZE.
        template<typename T>
        void BindBase(
            const GLuint index,
            const std::size_t elementOffset,
            const std::size_t elementCount
            )
        {
            assert(elementOffset <= std::numeric_limits<GLintptr>::max() / sizeof(T));
            assert(elementCount <= std::numeric_limits<GLsizeiptr>::max() / sizeof(T));
            const GLintptr offset = std::min<std::size_t>(
                elementOffset * sizeof(T), std::numeric_limits<GLintptr>::max());
            const GLsizeiptr size = std::min<std::size_t>(
                elementCount * sizeof(T), std::numeric_limits<GLsizeiptr>::max());

            ::glBindBufferRange(static_cast<GLenum>(buffer_object_t), index, _name, offset, size);
            VerifyNoErrors();
        }

        template<typename T>
        struct UnmapDeleter
        {
            typedef T* pointer;

            void operator()(pointer ptr) const
            {
                if (nullptr == ptr)
                {
                    return;
                }
                const auto result = ::glUnmapBuffer(static_cast<GLenum>(buffer_object_t));
                assert(GL_TRUE == result);
                AssertNoErrors();   // InvalidOperation can happen if the buffer object's data
                                    // store wasn't mapped.
            }
        };

        // Throws:  InvalidEnum : buffer object type is not GL_ARRAY_BUFFER, GL_COPY_READ_BUFFER,
        //              GL_COPY_WRITE_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_PIXEL_PACK_BUFFER,
        //              GL_PIXEL_UNPACK_BUFFER, GL_TEXTURE_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER or
        //              GL_UNIFORM_BUFFER; -or-, accessPolicy is an invalid value.
        //          OutOfMemory : OpenGL is unable to map the buffer object's data store.
        //          InvalidOperation : Bind() wasn't called before this method; -or-, this buffer
        //              object's data store is already mapped.
        // Returns: the buffer object's memory mapped into system memory, or nullptr.
        template<typename T>
        std::unique_ptr<T, UnmapDeleter<T>> Map(const AccessPolicy accessPolicy)
        {
            std::unique_ptr<T, UnmapDeleter<T>> pMappedBuffer(::glMapBuffer(
                static_cast<GLenum>(buffer_object_t),
                static_cast<GLenum>(accessPolicy)
                ));
            VerifyNoErrors();
            return pMappedBuffer;
        }

        // Throws:  InvalidOperation : Bind() wasn't called before calling this method.
        AccessPolicy AccessPolicy() const
        {
            GLint result;
            ::glGetBufferParameteriv(
                static_cast<GLenum>(buffer_object_t),
                GL_BUFFER_ACCESS,
                &result
                );
            VerifyNoErrors();
            return static_cast<enum AccessPolicy>(result);
        }

        // Throws:  InvalidOperation : Bind() wasn't called before calling this method.
        bool IsMapped() const
        {
            GLint result;
            ::glGetBufferParameteriv(
                static_cast<GLenum>(buffer_object_t),
                GL_BUFFER_MAPPED,
                &result
                );
            VerifyNoErrors();
            return GL_FALSE != result;
        }

        // Throws:  InvalidOperation : Bind() wasn't called before calling this method.
        std::int_fast32_t SizeInBytes32() const
        {
            GLint result;
            ::glGetBufferParameteriv(
                static_cast<GLenum>(buffer_object_t),
                GL_BUFFER_SIZE,
                &result
                );
            VerifyNoErrors();
            return static_cast<std::int_fast32_t>(result);
        }

        // Throws:  InvalidOperation : Bind() wasn't called before calling this method.
        std::int_fast64_t SizeInBytes64() const
        {
            GLint64 result;
            ::glGetBufferParameteri64v(
                static_cast<GLenum>(buffer_object_t),
                GL_BUFFER_SIZE,
                &result
                );
            VerifyNoErrors();
            return static_cast<std::int_fast64_t>(result);
        }

        // Throws:  InvalidOperation : Bind() wasn't called before calling this method.
        UsagePattern UsagePattern() const
        {
            GLint result;
            ::glGetBufferParameteriv(
                static_cast<GLenum>(buffer_object_t),
                GL_BUFFER_USAGE,
                &result
                );
            VerifyNoErrors();
            return static_cast<enum UsagePattern>(result);
        }

        // Not copyable.
        BufferObject(const BufferObject&) = delete;
        BufferObject& operator =(const BufferObject&) = delete;

    private:
        BufferObjectName _name;
    };

    typedef std::unique_ptr<BufferObjectName> buffer_object_name_ptr;

} // namespace gl


#endif
