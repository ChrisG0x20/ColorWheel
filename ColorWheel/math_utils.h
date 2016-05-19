//  Copyright (c) 2016 Christopher Gassib. All rights reserved.
//

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <algorithm>
#include <cmath>
#include "trig_constants.h"


template<typename scalar_t>
scalar_t Clamp(const scalar_t value, const scalar_t minimum, const scalar_t maximum)
{
    return std::min(std::max(value, minimum), maximum);
}

template<typename scalar_t>
scalar_t ClampRadians(const scalar_t radians)
{
    using namespace trig;
    
    if (radians >= TwoPi)
    {
        return std::fmod(radians, TwoPi);
    }
    if (radians < 0.0)
    {
        return TwoPi + std::fmod(radians, TwoPi);
    }
    return radians;
}

template<typename scalar_t>
scalar_t AbsRadialDistance(const scalar_t lhs, const scalar_t rhs)
{
    using namespace trig;

    const scalar_t distance = std::abs(ClampRadians(lhs) - ClampRadians(rhs));
    return (distance > Pi) ? TwoPi - distance : distance;
}

template<typename scalar_t, glm::precision P>
inline int MaxIndex(const glm::tvec3<scalar_t, P>& v) noexcept
{
    if (v[0] > v[1])
    {
        return v[0] > v[2] ? 0 : 2;
    }
    return v[1] > v[2] ? 1 : 2;
}

template<typename scalar_t, glm::precision P>
inline int MinIndex(const glm::tvec3<scalar_t, P>& v) noexcept
{
    if (v[0] < v[1])
    {
        return v[0] < v[2] ? 0 : 2;
    }
    return v[1] < v[2] ? 1 : 2;
}

template<typename scalar_t, glm::precision P>
inline scalar_t RgbToCircularHue(const glm::tvec3<scalar_t, P>& v, scalar_t& chroma) noexcept
{
    const auto Mi = MaxIndex(v);
    const auto mi = MinIndex(v);
    const auto M = v[Mi];
    const auto m = v[mi];
    const auto C = M - m;

    scalar_t Hp = 0.0;
    if (C == 0.0)
    {
        return 0.0; // NaN
    }
    else if (&v.r == &v[Mi])
    {
        Hp = std::fmod((v.g - v.b) / C, 6.0);
    }
    else if (&v.g == &v[Mi])
    {
        Hp = (v.b - v.r) / C + 2.0;
    }
    else if (&v.b == &v[Mi])
    {
        Hp = (v.r - v.g) / C + 4.0;
    }

    chroma = C;
    return trig::_60Degrees * Hp;
}

template<typename scalar_t, glm::precision P>
inline scalar_t RgbToHexagonalHue(const glm::tvec3<scalar_t, P>& v, scalar_t& chroma) noexcept
{
    const auto a = 0.5 * (2.0 * v.r - v.g - v.b);
    const auto b = (std::sqrt(3.0) / 2) * (v.g - v.b);
    const auto H2 = std::atan2(b, a);
    const auto C2 = std::sqrt(a ^ 2 + b ^ 2);

    chroma = C2;
    return H2;
}


#endif
