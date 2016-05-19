//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

// C++ Cross Platform Headers
/////////////////////////////
#include <cstdint>              // Cross-platform integers.
#include <stdexcept>            // For std::logic_error and std::runtime_error.
#include <string>               // Strings.
#include <iostream>             // Console and debug output.
#include <fstream>
#include <sstream>              // String manipulation.
#include <memory>               // Smart pointers.
#include <vector>               // Dynamic arrays.
#include <unistd.h>             // for usleep()
#include <unordered_map>

// Math Library
///////////////
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_CXX11
#include <glm/glm.hpp>          // Vector and Matrix math.
//#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

// SDL2 and OpenGL
//////////////////
#include <SDL2/SDL.h>           // Primary SDL include for all platforms.
#include <opengl/gl3.h>         // Probably specific to OS X.
//#include <opengl/gl3ext.h>      // Not using this yet.

// A couple options for using unique_ptr<> types around native resources.
/////////////////////////////////////////////////////////////////////////
#include "sdl.hpp"      // RAII wrappers for SDL resources.
#include "gl_shader.hpp"
#include "gl_program.hpp"
#include "gl_buffer_object.hpp"

#include "math_utils.h"

#include "imgui/imgui.h"
#include "imgui/imguiRenderGL3.h"

#include "sdl_startup.hpp"
#include "gl_startup.hpp"


// Application Constants
////////////////////////
const char WindowName[] = "Cross Platform Skeleton";
const glm::ivec2 ScreenSize(1024, 768);
const auto VerticalFov = trig::_60Degrees;

// OpenGL stuff
///////////////
enum class Vbo
: GLuint
{
    Vertices    = 0u,
    Color       = 1000u,
    AltColor    = 2000u,
    Saturation  = 3000u,
};

// OpenGL Buffer Object IDs
GLuint vertexArrayObjectName;

typedef gl::BufferObject<gl::BufferObjectType::Array> vertex_buffer_obj;
std::unordered_map<GLuint, vertex_buffer_obj> vertexBufferObjects;

// Uniform locations
GLint projectionMatrixLocation = -1;
GLint modelViewMatrixLocation = -1;
GLint colorBlendTLocation = -1;
GLint colorSpace0Location = -1;
GLint colorSpace1Location = -1;
GLint colorSpaceBlendTLocation = -1;
GLint lightnessLocation = -1;
GLint valueLocation = -1;

// Local copies of the uniforms
glm::mat4 projectionMatrix;
glm::mat4 modelViewMatrix;


// Color Wheel Setup
////////////////////
enum class ColorModel { Rgb, Ryb };
enum class ColorSpace : GLint { Undefined = 0, ColorWheel, Hsl, Hsv, Hsi, Hsy };

const auto MinHues = 6;
const auto MinSaturations = 2;

const auto MaxHueTessellations = 7;
const auto MaxSaturationTessellations = 8;
const auto WheelInnerRadius = 0.5f;

const double RgbColorModel[] =
{
    1.0, 0.0, 0.0, // Red
    //1.0, 0.5, 0.0, // Orange
    1.0, 1.0, 0.0, // Yellow
    //0.5, 1.0, 0.0, // Chartreuse
    0.0, 1.0, 0.0, // Green
    //0.0, 1.0, 0.5, // Spring
    0.0, 1.0, 1.0, // Cyan
    //0.0, 0.5, 1.0, // Azure
    0.0, 0.0, 1.0, // Blue
    //0.5, 0.0, 1.0, // Violet
    1.0, 0.0, 1.0, // Magenta
    //1.0, 0.0, 0.5, // Rose
};

const double RybColorModel[] = // RYB
{
    double(0xfe) / 0xff, double(0x27) / 0xff, double(0x12) / 0xff, // Red
    double(0xfb) / 0xff, double(0x99) / 0xff, double(0x02) / 0xff, // Orange
    double(0xfe) / 0xff, double(0xfe) / 0xff, double(0x33) / 0xff, // Yellow
    double(0x66) / 0xff, double(0xb0) / 0xff, double(0x32) / 0xff, // Green
    double(0x02) / 0xff, double(0x47) / 0xff, double(0xfe) / 0xff, // Blue
    double(0x86) / 0xff, double(0x01) / 0xff, double(0xaf) / 0xff, // Violet
};


template<size_t ColorModelSize>
void CalculateHueForColorModel(
    const double hueRadians,
    const double (&colorModel)[ColorModelSize],
    glm::dvec3& color
    )
{
    using namespace trig;
    static_assert(0 != ColorModelSize, "ColorModelSize == 0");

    const int ColorCount = ColorModelSize / 3;
    assert(0 != ColorCount);

    const double ColorRange = TwoPi / ColorCount;
    const double clampedHue = ClampRadians(hueRadians);

    const int lowerColor = (int(clampedHue / ColorRange) >= ColorCount) ?
        ColorCount - 1 : int(clampedHue / ColorRange);
    const int upperColor = (lowerColor + 1 >= ColorCount) ? 0 : lowerColor + 1;

    const double distFromLower = clampedHue - (lowerColor * ColorRange);

    const double upperContribution = distFromLower / ColorRange;
    const double lowerContribution = 1.0 - upperContribution;

    const int i = lowerColor * 3;
    const int j = upperColor * 3;
    color.r = (lowerContribution * colorModel[i + 0]) + (upperContribution * colorModel[j + 0]);
    color.g = (lowerContribution * colorModel[i + 1]) + (upperContribution * colorModel[j + 1]);
    color.b = (lowerContribution * colorModel[i + 2]) + (upperContribution * colorModel[j + 2]);
}


// Must make sure there's room reserved for (colorCount * 4) additional glm::vec3s.
//      colorWheelColors.reserve(colorWheelColors.size() + (colorCount * 4));
// Returns: iterator pointing to end()
template<
    typename iterator_type,
    typename std::enable_if<
        std::is_same<typename std::iterator_traits<iterator_type>::value_type, glm::vec3>::value
        >::type* = nullptr
    >
iterator_type CreateColorWheelColors(
    const unsigned int colorCount,
    const unsigned int trackCount,
    const double (&colorModel)[3 * 6],
    const iterator_type position
    )
{
    const auto SliceStep = trig::TwoPi / static_cast<decltype(trig::TwoPi)>(colorCount);

    auto it = position;
    auto hue = 0.0;
    glm::dvec3 color;

    for (auto j = 0; j < colorCount; ++j, hue += SliceStep)
    {
        CalculateHueForColorModel(hue, colorModel, color);

        // 4 points per quad
        for (auto i = 0; i < 4; ++i, ++it)
        {
            *it = color;
        }
    }

    // Copy the colors of the first track to all the rest.
    for (auto j = 1; j < trackCount; ++j)
    {
        auto srcIt = position;
        for (auto i = 0; i < colorCount * 4; ++i, ++srcIt, ++it)
        {
            *it = *srcIt;
        }
    }

    return it;
}


// Must make sure there's room reserved for (trackCount * sectorCount * 4) additional glm::vec2s.
//      colorWheelVerts.reserve(colorWheelVerts.size() + (trackCount * sectorCount * 4));
// Returns: iterator pointing to end()
template<
    typename iterator_type,
    typename std::enable_if<
        std::is_same<typename std::iterator_traits<iterator_type>::value_type, glm::vec2>::value
        >::type* = nullptr
    >
iterator_type CreateColorWheelGeometry(
    const unsigned int  sectorCount,
    const unsigned int  trackCount,
    const float         innerRadius,        // inner radius of the color wheel
    const float         sectorBlendT,
    const float         trackBlendT,
    const iterator_type position
    )
{
    assert(sectorCount >= 2);
    assert(sectorCount % 2 == 0);
    assert(trackCount == 1 || trackCount % 2 == 0);

    const auto SliceStep = trig::TwoPi / sectorCount; // Radians of a single slice.
    const auto SectorHalfBlend = sectorBlendT / 2.0; // Clamp sector blend from [0.0, 0.5].
    // Blend even slice half-size from [1.0, 0.5].
    const auto EvenSectorHalfAngle = SliceStep * (1.0 - SectorHalfBlend);
    // Blend odd slice half-size from [0.0, 0.5].
    const auto OddSectorHalfAngle = SliceStep * SectorHalfBlend;

    // Width of a single track.
    const auto TrackStep = (trackCount > 0) ? (1.0 - innerRadius) / trackCount : 1.0 - innerRadius;
    // Blend even track width from double track to single track width.
    const auto EvenTrackWidth = TrackStep + TrackStep * (1.0 - trackBlendT);
    // Blend odd track width from [0.0, 1.0].
    const auto OddTrackWidth = TrackStep * trackBlendT;
    const auto minTracks = (0 != trackCount) ? trackCount : 1;

    auto it = position;
    auto trackInnerRadius = innerRadius;
    auto isEvenTrack = true;

    for (auto j = 0; j < minTracks; j += 2)
    {
        for (auto k = 0; k < 2; ++k)
        {
            const auto              Inner = trackInnerRadius;
            const decltype(Inner)   Outer = Inner + ((isEvenTrack) ? EvenTrackWidth : OddTrackWidth);

            auto sectorAngle = 0.0;

            for (auto l = 0; l < sectorCount; l += 2)
            {
                auto isEvenSector = true;
                
                for (auto m = 0; m < 2; ++m)
                {
                    const auto SectorHalfAngle = (isEvenSector) ?
                        EvenSectorHalfAngle : OddSectorHalfAngle;
                    const auto lower = sectorAngle - SectorHalfAngle;
                    const auto upper = sectorAngle + SectorHalfAngle;
                    const auto lr = glm::vec2(std::cos(lower), std::sin(lower));
                    const auto ur = glm::vec2(std::cos(upper), std::sin(upper));

                    *it++ = lr * Inner;
                    *it++ = lr * Outer;
                    *it++ = ur * Inner;
                    *it++ = ur * Outer;

                    sectorAngle += SliceStep;

                    isEvenSector = !isEvenSector;
                }
            }

            trackInnerRadius = Outer;

            isEvenTrack = !isEvenTrack;
        }
    }

    return it;
}


template<
    typename iterator_type,
    typename std::enable_if<
        std::is_same<typename std::iterator_traits<iterator_type>::value_type, GLfloat>::value
        >::type* = nullptr
    >
iterator_type CreateColorWheelSaturation(
    const unsigned int  sectorCount,
    const unsigned int  trackCount,
    const float         trackBlendT,
    const iterator_type position
    )
{
    assert(trackCount == 1 || trackCount % 2 == 0);

    // Relative width of a single track for a unit circle.
    const auto TrackStep = (trackCount > 0) ? 1.0 / trackCount : 1.0;
    // Blend even track width from double track to single track width.
    const auto EvenTrackWidth = TrackStep + TrackStep * (1.0 - trackBlendT);
    // Blend odd track width from [0.0, 1.0].
    const auto OddTrackWidth = TrackStep * trackBlendT;
    const auto minTracks = (0 != trackCount) ? trackCount : 1;

    auto it = position;
    auto trackInnerRadius = 0.0f;
    auto isEven = trackCount >= 2;

    for (auto j = 0; j < minTracks; ++j)
    {
        const float Outer = trackInnerRadius + (isEven ? EvenTrackWidth : OddTrackWidth);
        isEven = !isEven;

        for (auto k = 0; k < sectorCount; ++k)
        {
            for (auto l = 0; l < 4; ++l)
            {
                *it++ = Outer; // Change to Inner for smooth shading.
            }
        }

        trackInnerRadius = Outer;
    }

    return it;
}


// 6, 12, 24, 48, 96, 192, 384
inline constexpr int ColorCountForTessellation(const int divisions)
{
    return MinHues * (1 << divisions);
}


inline constexpr int SaturationCountForTessellation(const int divisions)
{
    return 2 << divisions;
}


void InitializeVertexArrayBufferObjects()
{
    vertexBufferObjects.emplace(std::make_pair(static_cast<GLuint>(Vbo::Saturation), vertex_buffer_obj()));
    vertexBufferObjects.emplace(std::make_pair(static_cast<GLuint>(Vbo::Color), vertex_buffer_obj()));
    vertexBufferObjects.emplace(std::make_pair(static_cast<GLuint>(Vbo::AltColor), vertex_buffer_obj()));
    vertexBufferObjects.emplace(std::make_pair(static_cast<GLuint>(Vbo::Vertices), vertex_buffer_obj()));
}


void CreateSaturation(
    const unsigned int colorCount,
    const unsigned int trackCount,
    const float trackBlendT,
    std::vector<GLfloat>& colorWheelSaturation
    )
{
    const auto minTracks = (0 != trackCount) ? trackCount : 1;
    const auto TotalVertices = minTracks * colorCount * 4;

    if (colorWheelSaturation.size() < TotalVertices)
    {
        colorWheelSaturation.resize(TotalVertices);
    }

    CreateColorWheelSaturation(colorCount, minTracks, trackBlendT, std::begin(colorWheelSaturation));

    auto& saturationBuffer = vertexBufferObjects[static_cast<GLuint>(Vbo::Saturation)];
    saturationBuffer.Bind();
    saturationBuffer.Data(gl::UsagePattern::StaticDraw, &colorWheelSaturation[0], TotalVertices);
}


void CreateColors(
    const unsigned int colorCount,
    const unsigned int trackCount,
    std::vector<glm::vec3>& colorWheelColors,
    std::vector<glm::vec3>& altColorWheelColors
    )
{
    const auto minTracks = (0 != trackCount) ? trackCount : 1;
    const auto TotalVertices = minTracks * colorCount * 4;

    if (colorWheelColors.size() < TotalVertices)
    {
        colorWheelColors.resize(TotalVertices);
    }

    if (altColorWheelColors.size() < TotalVertices)
    {
        altColorWheelColors.resize(TotalVertices);
    }

    auto colorIt = std::begin(colorWheelColors);
    CreateColorWheelColors(colorCount, minTracks, RgbColorModel, colorIt);

    colorIt = std::begin(altColorWheelColors);
    CreateColorWheelColors(colorCount, minTracks, RybColorModel, colorIt);

    auto& colorBuffer = vertexBufferObjects[static_cast<GLuint>(Vbo::Color)];
    colorBuffer.Bind();
    colorBuffer.Data(gl::UsagePattern::DynamicDraw, &colorWheelColors[0], TotalVertices);

    auto& altColorBuffer = vertexBufferObjects[static_cast<GLuint>(Vbo::AltColor)];
    altColorBuffer.Bind();
    altColorBuffer.Data(gl::UsagePattern::DynamicDraw, &altColorWheelColors[0], TotalVertices);
}


void CreateGeometry(
    const unsigned int      sectorCount,
    const unsigned int      trackCount,
    const float             innerRadius,
    const float             sectorBlendT,
    const float             trackBlendT,
    std::vector<glm::vec2>& colorWheelVerts
    )
{
    const auto minTracks = (0 != trackCount) ? trackCount : 1;
    const auto TotalVertices = minTracks * sectorCount * 4;

    if (colorWheelVerts.size() < TotalVertices)
    {
        colorWheelVerts.resize(TotalVertices);
    }

    auto vertIt = std::begin(colorWheelVerts);
    vertIt = CreateColorWheelGeometry(
        sectorCount,
        minTracks,
        innerRadius,
        sectorBlendT,
        trackBlendT,
        vertIt
        );

    auto& vertexBuffer = vertexBufferObjects[static_cast<GLuint>(Vbo::Vertices)];
    vertexBuffer.Bind();
    vertexBuffer.Data(gl::UsagePattern::DynamicDraw, &colorWheelVerts[0], TotalVertices);
}


inline void RecalculateIndices(
    const unsigned int      sectorCount,
    const unsigned int      trackCount,
    std::vector<GLint>&     trackStartPositions,
    std::vector<GLsizei>&   trackIndiceCounts
    )
{
    const auto minTracks = (0 != trackCount) ? trackCount : 1;
    const auto ElementsPerTrack = sectorCount * 4;
    auto startPosition = 0;

    for (auto i = 0; i < minTracks; ++i, startPosition += ElementsPerTrack)
    {
        trackStartPositions[i] = startPosition;
        trackIndiceCounts[i] = ElementsPerTrack;
    }
}


inline void DrawColorWheel(
    const unsigned int          trackCount,
    const std::vector<GLint>&   trackStartPositions,
    const std::vector<GLsizei>& trackIndiceCounts
    )
{
    const auto minTracks = (0 != trackCount) ? trackCount : 1;
    ::glMultiDrawArrays(
        GL_TRIANGLE_STRIP,
        &trackStartPositions[0],
        &trackIndiceCounts[0],
        minTracks
        );
    gl::VerifyNoErrors();
}


// GUI
//////
void InitializeGui()
{
    // Init UI
    if (!::imguiRenderGLInit("DroidSans.ttf"))
    {
        throw std::runtime_error("Could not init GUI renderer.");
    }
}


// A simple game loop.
//////////////////////
void RunGameLoop(const sdl::WindowHandle& windowHandle)
{
    const auto pShader = InitializeGraphics(
        ScreenSize,
        VerticalFov,
        vertexArrayObjectName,
        projectionMatrixLocation,
        modelViewMatrixLocation,
        colorBlendTLocation,
        colorSpace0Location,
        colorSpace1Location,
        colorSpaceBlendTLocation,
        lightnessLocation,
        valueLocation,
        projectionMatrix,
        modelViewMatrix
    );

    const auto InitialModelViewMatrix = modelViewMatrix;

    // Create some data buffers
    const auto MaxColorCount = ColorCountForTessellation(MaxHueTessellations) *
        SaturationCountForTessellation(MaxSaturationTessellations);
    const auto TotalVertices = MaxColorCount * 4;

    std::vector<GLfloat>    colorWheelSaturation(TotalVertices);

    std::vector<glm::vec3>  colorWheelColors(TotalVertices);
    std::vector<glm::vec3>  altColorWheelColors(TotalVertices);

    std::vector<glm::vec2>  colorWheelVerts(TotalVertices);

    InitializeVertexArrayBufferObjects();

    CreateSaturation(MinHues, MinSaturations, 0.0f, colorWheelSaturation);
    auto& saturationBuffer = vertexBufferObjects[static_cast<GLuint>(Vbo::Saturation)];

    CreateColors(MinHues, MinSaturations, colorWheelColors, altColorWheelColors);
    auto& colorBuffer = vertexBufferObjects[static_cast<GLuint>(Vbo::Color)];
    auto& altColorBuffer = vertexBufferObjects[static_cast<GLuint>(Vbo::AltColor)];

    CreateGeometry(MinHues, MinSaturations, WheelInnerRadius, 0.0f, 0.0f, colorWheelVerts);
    auto& vertexBuffer = vertexBufferObjects[static_cast<GLuint>(Vbo::Vertices)];

    std::vector<GLint>      trackStartPositions(SaturationCountForTessellation(MaxSaturationTessellations));
    std::vector<GLsizei>    trackIndiceCounts(SaturationCountForTessellation(MaxSaturationTessellations));
    {
        saturationBuffer.Bind();
        ::glVertexAttribPointer(
            SaturationLocation,
            sizeof(colorWheelSaturation[0]) / sizeof(GLfloat),
            GL_FLOAT,
            GL_FALSE,
            0,
            nullptr
            );
        gl::VerifyNoErrors();

        vertexBuffer.Bind();
        ::glVertexAttribPointer(
            PositionLocation,
            sizeof(colorWheelVerts[0]) / sizeof(GLfloat),
            GL_FLOAT,
            GL_FALSE,
            0,
            nullptr
            );
        gl::VerifyNoErrors();

        colorBuffer.Bind();
        ::glVertexAttribPointer(
            ColorLocation,
            sizeof(colorWheelColors[0]) / sizeof(GLfloat),
            GL_FLOAT,
            GL_FALSE,
            0,
            nullptr
            );
        gl::VerifyNoErrors();

        altColorBuffer.Bind();
        ::glVertexAttribPointer(
            AltColorLocation,
            sizeof(altColorWheelColors[0]) / sizeof(GLfloat),
            GL_FLOAT,
            GL_FALSE,
            0,
            nullptr
            );
        gl::VerifyNoErrors();

        // Enable the vertex attribute buffers for use with OpenGL drawing commands.
        ::glEnableVertexAttribArray(SaturationLocation);
        gl::VerifyNoErrors();

        ::glEnableVertexAttribArray(PositionLocation);
        gl::VerifyNoErrors();

        ::glEnableVertexAttribArray(ColorLocation);
        gl::VerifyNoErrors();

        ::glEnableVertexAttribArray(AltColorLocation);
        gl::VerifyNoErrors();
    }

    InitializeGui();

    // GUI state
    glm::ivec2 mouseCursor;
    glm::bvec2 mouseButtons;
    glm::ivec2 mouseWheel;
    auto scrollarea1 = 0;
    auto colorModel = ColorModel::Rgb;
    auto colorCountPower = 1.0f;
    auto saturationCountPower = 6.0f;
    auto colorSpace = ColorSpace::ColorWheel;
    auto lightnessValue = 0.5f;
    auto isGeometryCylinder = true;
    // GUI state

    auto prev_ticks = ::SDL_GetTicks();
    auto x = 100.0f;

    const auto VertexBlendRate = 0.005f;

    auto currentColorCountPower = static_cast<int>(colorCountPower);
    auto sectorBlendT = 1.0f;
    auto sectorBlendVelocity = 0.0f;

    auto currentSaturationCountPower = static_cast<int>(saturationCountPower);
    auto trackBlendT = 0.0f;
    auto trackBlendVelocity = 0.0f;

    const auto InnerRadiusBlendRate = 0.005f;
    auto innerRadiusBlendDirection = 0.0f;
    auto innerRadiusBlendT = 1.0f;
    auto currentInnerRadius = innerRadiusBlendT * WheelInnerRadius;

    const auto ColorBlendRate = 0.001f;
    auto colorBlendDirection = 0.0f;
    auto colorBlendT = 0.0f;
    
    const auto ColorSpaceBlendRate = InnerRadiusBlendRate;
    auto colorSpaceBlendDirection = 0.0f;
    auto colorSpaceBlendT = 0.0f;

    auto currentColorSpace = colorSpace;
    auto nextColorSpace = colorSpace;

    auto hsLightness = 0.5f;
    auto hsValue = 1.0f;

    const auto ConeScaleRate = InnerRadiusBlendRate / 4.0f;
    auto coneScaleDirection = 0.0f;
    auto coneScale = 1.0f;
    auto currentConeScale = 1.0f;
    auto isUserScalingCone = false;

    auto isMorphing = true;

    bool done = false;
    while (!done)
    {
        // input
        SDL_Event event;
        while (::SDL_PollEvent(&event))
        {
            if (SDL_KEYDOWN == event.type)
            {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                {
                    done = true;
                }
            }

            else if (SDL_MOUSEMOTION == event.type)
            {
                mouseButtons[0] = (0 != (event.motion.state & SDL_BUTTON_LMASK));
                mouseButtons[1] = (0 != (event.motion.state & SDL_BUTTON_RMASK));
                mouseCursor.x = event.motion.x;
                mouseCursor.y = ScreenSize.y - event.motion.y;
            }

            else if (SDL_MOUSEBUTTONDOWN == event.type || SDL_MOUSEBUTTONUP == event.type)
            {
                if (SDL_BUTTON_LEFT == event.button.button)
                {
                    mouseButtons[0] = SDL_PRESSED == event.button.state;
                }
                if (SDL_BUTTON_RIGHT == event.button.button)
                {
                    mouseButtons[1] = SDL_PRESSED == event.button.state;
                }

                mouseCursor.x = event.button.x;
                mouseCursor.y = ScreenSize.y - event.button.y;
            }

            else if (SDL_MOUSEWHEEL == event.type)
            {
                mouseWheel.x = event.wheel.x;
                mouseWheel.y = event.wheel.y;
            }
        }

        double delta_t = 0;
        {
            auto current_ticks = ::SDL_GetTicks();
            delta_t = current_ticks - prev_ticks;
            prev_ticks = current_ticks;
        }

        // transform
        // collisions

        // render
        {
            ::glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
            ::glClear(GL_COLOR_BUFFER_BIT); // GL_DEPTH_BUFFER_BIT |

            ::glBindVertexArray(vertexArrayObjectName);
            gl::VerifyNoErrors();
            {
                pShader->Use();

                if (0.0f != colorBlendDirection) // if (animating a color change)
                {
                    colorBlendT += delta_t * colorBlendDirection;

                    if (colorBlendT <= 0.0f || colorBlendT >= 1.0f)
                    {
                        colorBlendDirection = 0.0f;
                    }

                    colorBlendT = Clamp(colorBlendT, 0.0f, 1.0f);

                    pShader->SetUniform(colorBlendTLocation, colorBlendT);
                    gl::VerifyNoErrors();
                }

                if (colorSpace != currentColorSpace || 0.0f != colorSpaceBlendDirection)
                {
                    if (0.0f == colorSpaceBlendT)
                    {
                        nextColorSpace = colorSpace;
                    }

                    colorSpaceBlendDirection = ColorSpaceBlendRate;

                    colorSpaceBlendT += delta_t * colorSpaceBlendDirection;
                    colorSpaceBlendT = Clamp(colorSpaceBlendT, 0.0f, 1.0f);

                    if (colorSpaceBlendT >= 1.0f)
                    {
                        currentColorSpace = nextColorSpace;
                        colorSpaceBlendDirection = 0.0f;
                        colorSpaceBlendT = 0.0f;

                        pShader->SetUniform(colorSpace0Location, static_cast<GLint>(currentColorSpace));
                        gl::VerifyNoErrors();
                    }
                    else
                    {
                        pShader->SetUniform(colorSpace0Location, static_cast<GLint>(currentColorSpace));
                        gl::VerifyNoErrors();

                        pShader->SetUniform(colorSpace1Location, static_cast<GLint>(nextColorSpace));
                        gl::VerifyNoErrors();
                    }

                    pShader->SetUniform(colorSpaceBlendTLocation, colorSpaceBlendT);
                    gl::VerifyNoErrors();
                }

                if (0.0f != innerRadiusBlendDirection)
                {
                    innerRadiusBlendT += delta_t * innerRadiusBlendDirection;

                    if (innerRadiusBlendT <= 0.0f || innerRadiusBlendT >= 1.0f)
                    {
                        innerRadiusBlendDirection = 0.0f;
                    }

                    innerRadiusBlendT = Clamp(innerRadiusBlendT, 0.0f, 1.0f);

                    currentInnerRadius = innerRadiusBlendT * WheelInnerRadius;
                    isMorphing = true;
                }

                // if (animating color count change)
                const auto GuiColorCountPower = static_cast<int>(colorCountPower);

                // if (the current number of colors doesn't match the GUI setting OR
                //      the geometry is still animating into place)
                if (GuiColorCountPower != currentColorCountPower || 0.0f != sectorBlendVelocity)
                {
                    if (GuiColorCountPower >= currentColorCountPower)
                    {
                        sectorBlendVelocity = VertexBlendRate;
                    }
                    else if (GuiColorCountPower < currentColorCountPower)
                    {
                        sectorBlendVelocity = -VertexBlendRate;
                    }

                    sectorBlendT += delta_t * sectorBlendVelocity;

                    if (sectorBlendT <= 0.0f)
                    {
                        --currentColorCountPower;
                        sectorBlendT = 1.0f;

                        if (GuiColorCountPower == currentColorCountPower)
                        {
                            sectorBlendVelocity = 0.0f;
                        }
                    }
                    else if (sectorBlendT >= 1.0f)
                    {
                        if (GuiColorCountPower == currentColorCountPower)
                        {
                            sectorBlendT = 1.0f;
                            sectorBlendVelocity = 0.0f;
                        }
                        else
                        {
                            ++currentColorCountPower;
                            sectorBlendT = 0.0f;
                        }
                    }

                    isMorphing = true;
                }

                // if (animating saturation count change)
                const auto GuiSaturationCountPower = static_cast<int>(saturationCountPower);

                // if (the current number of colors doesn't match the GUI setting OR
                //      the geometry is still animating into place)
                if (GuiSaturationCountPower != currentSaturationCountPower || 0.0f != trackBlendVelocity)
                {
                    if (GuiSaturationCountPower >= currentSaturationCountPower)
                    {
                        trackBlendVelocity = VertexBlendRate;
                    }
                    else if (GuiSaturationCountPower < currentSaturationCountPower)
                    {
                        trackBlendVelocity = -VertexBlendRate;
                    }

                    trackBlendT += delta_t * trackBlendVelocity;

                    if (trackBlendT <= 0.0f)
                    {
                        --currentSaturationCountPower;
                        trackBlendT = 1.0f;

                        if (GuiSaturationCountPower == currentSaturationCountPower)
                        {
                            trackBlendVelocity = 0.0f;
                        }
                    }
                    else if (trackBlendT >= 1.0f)
                    {
                        if (GuiSaturationCountPower == currentSaturationCountPower)
                        {
                            trackBlendT = 1.0f;
                            trackBlendVelocity = 0.0f;
                        }
                        else
                        {
                            ++currentSaturationCountPower;
                            trackBlendT = 0.0f;
                        }
                    }

                    isMorphing = true;
                }

                pShader->SetUniform(lightnessLocation, hsLightness);
                gl::VerifyNoErrors();
                pShader->SetUniform(valueLocation, hsValue);
                gl::VerifyNoErrors();

                {
                    if (colorSpace == ColorSpace::ColorWheel || isGeometryCylinder)
                    {
                        coneScale = 1.0f;
                    }
                    else if (colorSpace == ColorSpace::Hsl)
                    {
                        coneScale = 1.0f - std::abs(hsLightness * 2.0f - 1.0f);
                    }
                    else // colorSpace == ColorSpace::Hsv
                    {
                        coneScale = hsValue;
                    }

                    if (isUserScalingCone)
                    {
                        currentConeScale = coneScale;
                    }
                    else
                    {
                        if (currentConeScale < coneScale)
                        {
                            coneScaleDirection = ConeScaleRate;
                        }
                        else if (currentConeScale > coneScale)
                        {
                            coneScaleDirection = -ConeScaleRate;
                        }

                        if (delta_t * coneScaleDirection >= std::abs(coneScale - currentConeScale))
                        {
                            coneScaleDirection = 0.0f;
                            currentConeScale = coneScale;
                            isUserScalingCone = true;
                        }
                        else
                        {
                            currentConeScale += delta_t * coneScaleDirection;
                        }
                    }

                    if (1.0f == currentConeScale)
                    {
                        pShader->SetUniform(modelViewMatrixLocation, InitialModelViewMatrix);
                    }
                    else
                    {
                        modelViewMatrix = glm::scale(InitialModelViewMatrix, glm::vec3(currentConeScale));
                        pShader->SetUniform(modelViewMatrixLocation, modelViewMatrix);
                    }
                }

                const auto TrackCount = SaturationCountForTessellation(currentSaturationCountPower);

                if (isMorphing)
                {
                    const auto ColorCount = ColorCountForTessellation(currentColorCountPower);

                    RecalculateIndices(ColorCount, TrackCount, trackStartPositions, trackIndiceCounts);
                    CreateColors(ColorCount, TrackCount, colorWheelColors, altColorWheelColors);
                    CreateSaturation(ColorCount, TrackCount, trackBlendT, colorWheelSaturation);
                    CreateGeometry(
                        ColorCount,
                        TrackCount,
                        currentInnerRadius,
                        sectorBlendT,
                        trackBlendT,
                        colorWheelVerts
                        );
                    isMorphing = false;
                }

                DrawColorWheel(TrackCount, trackStartPositions, trackIndiceCounts);
            }
            ::glBindVertexArray(0);
            gl::VerifyNoErrors();


            // Draw GUI
            ///////////

            // Pass input to GUI
            ::imguiBeginFrame(
                mouseCursor.x,
                mouseCursor.y,
                (mouseButtons[0]) ? IMGUI_MBUT_LEFT : 0 |
                (mouseButtons[1]) ? IMGUI_MBUT_RIGHT : 0,
                mouseWheel.y
                );
            {
                ::imguiBeginScrollArea(
                    "Color Wheel Control",
                    10, 10,
                    ScreenSize.x / 5, ScreenSize.y - 20,
                    &scrollarea1
                    );
                {
                    ::imguiSeparator();
                    ::imguiLabel("Color Model");
                    ::imguiSeparatorLine();

                    {
                        const auto original = colorModel;
                        if (ColorModel::Ryb == colorModel)
                        {
                            colorModel = ::imguiCheck("RGB", false) ?
                                ColorModel::Rgb : ColorModel::Ryb;
                            ::imguiCheck("RYB", true, false);
                        }
                        else
                        {
                            ::imguiCheck("RGB", true, false);
                            colorModel = ::imguiCheck("RYB", false) ?
                                ColorModel::Ryb : ColorModel::Rgb;
                        }
                        if (original != colorModel)
                        {
                            colorBlendDirection = ColorModel::Ryb == colorModel ?
                                ColorBlendRate : -ColorBlendRate;
                        }
                    }

                    ::imguiSeparator();
                    ::imguiLabel("Colors");
                    ::imguiSeparatorLine();

                    {
                        // 6, 12, 24, 48, 96, 192, 384
                        std::ostringstream os;
                        os << "Sectors: " <<
                            ColorCountForTessellation(static_cast<int>(colorCountPower));
                        ::imguiSlider(
                            os.str().c_str(),
                            &colorCountPower,
                            0,
                            MaxHueTessellations,
                            1
                            );
                    }

                    ::imguiSeparator();
                    ::imguiLabel("Saturation");
                    ::imguiSeparatorLine();

                    {
                        // 2, 4, 8, 16, 32, 64, 128, 256, 512
                        std::ostringstream os;
                        os << "Tracks: " <<
                            SaturationCountForTessellation(static_cast<int>(saturationCountPower));
                        ::imguiSlider(
                            os.str().c_str(),
                            &saturationCountPower,
                            -1,
                            MaxSaturationTessellations,
                            1
                            );
                    }

                    ::imguiSeparator();
                    ::imguiLabel("Color Space");
                    ::imguiSeparatorLine();

                    {
//                        ColorWheel, Hsl, Hsv, Hsi, Hsy
                        int next = 0;

                        if (ColorSpace::Hsl == colorSpace)
                        {
                            next |= static_cast<int>(::imguiCheck("Color Wheel", false) ?
                                ColorSpace::ColorWheel : ColorSpace::Undefined);
                            ::imguiCheck("HSL", true, false);
                            next |= static_cast<int>(::imguiCheck("HSV", false) ?
                                ColorSpace::Hsv : ColorSpace::Undefined);
                            
                            if (static_cast<ColorSpace>(next) == ColorSpace::ColorWheel)
                            {
                                innerRadiusBlendDirection = InnerRadiusBlendRate;
                            }
                        }
                        else if (ColorSpace::Hsv == colorSpace)
                        {
                            next |= static_cast<int>(::imguiCheck("Color Wheel", false) ?
                                ColorSpace::ColorWheel : ColorSpace::Undefined);
                            next |= static_cast<int>(::imguiCheck("HSL", false) ?
                                ColorSpace::Hsl : ColorSpace::Undefined);
                            ::imguiCheck("HSV", true, false);

                            if (static_cast<ColorSpace>(next) == ColorSpace::ColorWheel)
                            {
                                innerRadiusBlendDirection = InnerRadiusBlendRate;
                            }
                        }
                        else // (ColorSpace::ColorWheel == colorSpace)
                        {
                            colorSpace = ColorSpace::ColorWheel;
                            ::imguiCheck("Color Wheel", true, false);
                            next |= static_cast<int>(::imguiCheck("HSL", false) ?
                                ColorSpace::Hsl : ColorSpace::Undefined);
                            next |= static_cast<int>(::imguiCheck("HSV", false) ?
                                ColorSpace::Hsv : ColorSpace::Undefined);

                            if (static_cast<ColorSpace>(next) != ColorSpace::Undefined)
                            {
                                innerRadiusBlendDirection = -InnerRadiusBlendRate;
                            }
                        }

                        if (static_cast<ColorSpace>(next) != ColorSpace::Undefined)
                        {
                            colorSpace = static_cast<ColorSpace>(next);
                            isUserScalingCone = false;
                        }
                    }

                    if (colorSpace != ColorSpace::ColorWheel)
                    {
                        const auto label = colorSpace == ColorSpace::Hsl ? "Lightness" : "Value / Brightness";
                        const auto sliderLabel = colorSpace == ColorSpace::Hsl ? "Lightness" : "Value";
                        lightnessValue = colorSpace == ColorSpace::Hsl ? hsLightness : hsValue;

                        ::imguiSeparator();
                        ::imguiLabel(label);
                        ::imguiSeparatorLine();

                        ::imguiSlider(sliderLabel, &lightnessValue, 0.0f, 1.0f, 1.0f / 256.0f);

                        if (colorSpace == ColorSpace::Hsl)
                        {
                            hsLightness = lightnessValue;
                        }
                        else if (colorSpace == ColorSpace::Hsv)
                        {
                            hsValue = lightnessValue;
                        }

                        {
                            ::imguiSeparator();
                            ::imguiLabel("Geometry Model");
                            ::imguiSeparatorLine();

                            const auto HexconeModel = colorSpace == ColorSpace::Hsl ?
                                "Bi-cone" : "Cone";

                            const auto originalValue = isGeometryCylinder;

                            if (isGeometryCylinder)
                            {
                                ::imguiCheck("Cylinder", true, false);
                                isGeometryCylinder = !::imguiCheck(HexconeModel, false);
                            }
                            else
                            {
                                isGeometryCylinder = ::imguiCheck("Cylinder", false);
                                ::imguiCheck(HexconeModel, true, false);
                            }

                            if (isGeometryCylinder != originalValue)
                            {
                                isUserScalingCone = false;
                            }
                        }
                    }
                }
                ::imguiEndScrollArea();
            }
            ::imguiEndFrame();

//            ::imguiDrawText(
//                20 + ScreenSize.x / 5,
//                ScreenSize.y - 20,
//                IMGUI_ALIGN_LEFT,
//                "Free text",
//                imguiRGBA(32,192, 32,192)
//                );

            ::imguiRenderGLDraw(ScreenSize.x, ScreenSize.y);
            gl::VerifyNoErrors();
        } // render commands

        x += static_cast<float>(delta_t / 100.0);

        // swap buffers
        ::SDL_GL_SwapWindow(windowHandle);

        // Slow the loop down... for now.
//        ::usleep(1000 * 400);
    }

    ::imguiRenderGLDestroy();
}


// Initializes everything and starts running the game loop.
int main(const int argc, const char* const argv[])
{
    (void)(argc); (void)(argv); // Unused parameters.

    // Create some resources that will exist for the life of the application.
    auto pSdllibrary = InitializeApplication();
    auto pWindow = CreateMainWindow(WindowName, ScreenSize);
    auto pGlContext = CreateRenderingContext(*pWindow);

    // OKGO!
    RunGameLoop(*pWindow);

    return 0;
}
