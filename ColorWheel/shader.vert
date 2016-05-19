//  Copyright (c) 2016 Christopher Gassib. All rights reserved.

#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform float colorBlendT;
uniform int colorSpace0;
uniform int colorSpace1;
uniform float colorSpaceBlendT;
uniform float lightness;
uniform float value;

in vec2 in_position;
in vec3 in_color;
in vec3 in_altColor;
in float in_saturation;

out vec4 thru_color;

vec3 CalcColorSpaceAdjustment(const int colorSpace, const float saturation, const vec3 originalValue)
{
    vec3 blend = originalValue;

    if (colorSpace == 2) // HSL
    {
        blend += lightness * 2.0 - 1.0;
        return mix(vec3(lightness), blend, saturation);
    }
    else if (colorSpace == 3) // HSV
    {
        blend *= value;
        return mix(vec3(1.0f * value), blend, saturation);
    }

    return originalValue;
}

void main()
{
    gl_Position = projectionMatrix * modelViewMatrix * vec4(in_position, 0.0, 1.0);

    vec3 blend = mix(in_color, in_altColor, colorBlendT);

    vec3 s = CalcColorSpaceAdjustment(colorSpace0, in_saturation, blend);
    vec3 e = CalcColorSpaceAdjustment(colorSpace1, in_saturation, blend);
    blend = mix(s, e, colorSpaceBlendT);

    thru_color = vec4(blend, 1.0);
}
