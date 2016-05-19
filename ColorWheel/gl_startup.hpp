//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#ifndef GL_STARTUP
#define GL_STARTUP


// Attribute Locations (per-vertex structure layout)
const GLint PositionLocation = 0;
const GLint ColorLocation = 1;
const GLint AltColorLocation = 2;
const GLint SaturationLocation = 3;

typedef std::unique_ptr<gl::Program> program_ptr;

std::string LoadTextFile(const std::string& filename);
void ResetModelView(const glm::ivec2 windowClientSize, const float verticalFov, glm::mat4& modelViewMatrix);
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
);


#endif
