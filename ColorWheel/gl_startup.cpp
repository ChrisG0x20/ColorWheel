//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#include <string>
#include <iostream>
#include <sstream>
#include <array>
#include <fstream>

#include <SDL2/SDL.h>           // Primary SDL include for all platforms.
#include <opengl/gl3.h>         // Probably specific to OS X.
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>          // Vector and Matrix math.
#include <glm/gtc/matrix_transform.hpp>

#include "sdl.hpp"
#include "gl_shader.hpp"
#include "gl_program.hpp"
#include "trig_constants.h"

#include "gl_startup.hpp"


// Throws : SdlException
//          std::bad_alloc
std::string LoadTextFile(const std::string& filename)
{
    const size_t BufferSize = 4096u;

    std::string fullpath(sdl::GetExecutablePath().get() + filename);

    std::cout << "Loading: " << fullpath << std::endl;

    std::ostringstream text;
    size_t totalRead = 0u;
    {
        std::array<char, BufferSize> buffer;
        std::ifstream ifs(fullpath);
        while (ifs)
        {
            ifs.read(&buffer[0], buffer.size());
            auto charsRead = ifs.gcount();
            text.write(&buffer[0], charsRead);
            totalRead += charsRead;
        }
    }

    std::cout << "Read " << totalRead << " bytes." << std::endl;

    return text.str();
}


void ResetModelView(const glm::ivec2 windowClientSize, const float verticalFov, glm::mat4& modelViewMatrix)
{
    const auto aspect = static_cast<float>(windowClientSize.x) / windowClientSize.y;

    modelViewMatrix = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(aspect / 5.0f, 0.0f, -2.0f * std::cos(verticalFov / 2.0f))
        );
}


// Do the initial OpenGL camera setup.
program_ptr InitializeGraphics(
    const glm::ivec2 windowClientSize,
    const float verticalFov,
    GLuint& vertexArrayObjectName,
    GLint& projectionMatrixLocation,
    GLint& modelViewMatrixLocation,
    GLint& colorBlendTLocation,
    GLint& colorSpace0Location,
    GLint& colorSpace1Location,
    GLint& colorSpaceBlendTLocation,
    GLint& lightnessLocation,
    GLint& valueLocation,
    glm::mat4& projectionMatrix,
    glm::mat4& modelViewMatrix
    )
{
    using namespace trig;
    
    // Setup the viewport, projection, and model-view.
    ::glViewport(0, 0, windowClientSize.x, windowClientSize.y);
    gl::AssertNoErrors();
    
    // Create the OpenGL buffer objects
    ///////////////////////////////////
    {
        ::glGenVertexArrays(1, &vertexArrayObjectName);
        gl::VerifyNoErrors();
        
        std::cout << "Vertex Array ID: " << vertexArrayObjectName << std::endl;
        
        ::glBindVertexArray(vertexArrayObjectName);
        gl::VerifyNoErrors();
    }
    
    // Load the shader programs
    ///////////////////////////
    gl::Shader vertexShader(gl::ShaderType::Vertex);
    {
        const auto& srcCode = LoadTextFile("shader.vert");
        std::cout << "Compiling vertex shader: ";
        vertexShader.Compile(srcCode);
        std::cout << (vertexShader.IsValid() ? "SUCCESS" : "FAILED") << std::endl;
        std::cout << vertexShader.GetCompilerOutput() << std::endl << std::endl;
    }
    
    gl::Shader fragmentShader(gl::ShaderType::Fragment);
    {
        const auto& srcCode = LoadTextFile("shader.frag");
        std::cout << "Compiling fragment shader: ";
        fragmentShader.Compile(srcCode);
        std::cout << (fragmentShader.IsValid() ? "SUCCESS" : "FAILED") << std::endl;
        std::cout << fragmentShader.GetCompilerOutput() << std::endl << std::endl;
    }
    
    program_ptr pShader(new gl::Program);
    {
        // NOTE: This as to happen before the link.
        // Describe to OpenGL how each shader attribute variable is going to map to the per-vertex
        // structures that will be sent to it.
        pShader->BindVectorIndexToShaderInputVariable(PositionLocation, "in_position");
        pShader->BindVectorIndexToShaderInputVariable(ColorLocation, "in_color");
        pShader->BindVectorIndexToShaderInputVariable(AltColorLocation, "in_altColor");
        pShader->BindVectorIndexToShaderInputVariable(SaturationLocation, "in_saturation");
        
        std::cout << "Linking shader object code: ";
        pShader->Attach(vertexShader);
        pShader->Attach(fragmentShader);
        pShader->Link();
        std::cout << (pShader->IsLinked() ? "SUCCESS" : "FAILED") << std::endl;
        std::cout << pShader->GetLinkerOrValidationOutput() << std::endl << std::endl;
    }
    
    {
        std::cout << "Validating complete shader program: ";
        pShader->Validate();
        std::cout << (pShader->IsValid() ? "SUCCESS" : "FAILED") << std::endl;
        std::cout << pShader->GetLinkerOrValidationOutput() << std::endl << std::endl;
    }
    
    pShader->Use(); // NOTE: Easy step to forget.
    
    ///////////////////////
    
    // Ask OpenGL for locations of the shader uniform variables.
    projectionMatrixLocation = pShader->GetUniformLocation("projectionMatrix");
    modelViewMatrixLocation = pShader->GetUniformLocation("modelViewMatrix");
    colorBlendTLocation = pShader->GetUniformLocation("colorBlendT");
    colorSpace0Location = pShader->GetUniformLocation("colorSpace0");
    colorSpace1Location = pShader->GetUniformLocation("colorSpace1");
    colorSpaceBlendTLocation = pShader->GetUniformLocation("colorSpaceBlendT");
    lightnessLocation = pShader->GetUniformLocation("lightness");
    valueLocation = pShader->GetUniformLocation("value");
    
    // Set the edges of the scene form -1.0 to 1.0 in the X and Y directions and 100.0 deep in the Z.
    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    //glFrustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 100.0f);
    projectionMatrix = glm::perspective(
        static_cast<float>(verticalFov),
        static_cast<float>(windowClientSize.x) / windowClientSize.y,
        0.1f,
        100.0f
        );
    
    ResetModelView(windowClientSize, verticalFov, modelViewMatrix);
    
    pShader->SetUniform(projectionMatrixLocation, projectionMatrix);
    pShader->SetUniform(modelViewMatrixLocation, modelViewMatrix);
    pShader->SetUniform(colorBlendTLocation, 0.0f);
    pShader->SetUniform(colorSpace0Location, 0);
    pShader->SetUniform(colorSpace1Location, 0);
    pShader->SetUniform(colorSpaceBlendTLocation, 0.0f);
    pShader->SetUniform(lightnessLocation, 0.5f);
    pShader->SetUniform(valueLocation, 1.0f);
    
    ::glEnable(GL_BLEND);
    ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //::glDisable(GL_DEPTH_TEST);
    
    return pShader;
}
