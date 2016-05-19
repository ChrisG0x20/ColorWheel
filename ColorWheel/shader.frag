//  Copyright (c) 2016 Christopher Gassib. All rights reserved.

#version 150 core

in vec4 thru_color;

out vec4 out_color;

void main()
{
    out_color = thru_color;
//    out_color = vec4(1.0, 1.0, 1.0, 1.0);
}
