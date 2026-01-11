#pragma once

#include "entity.hpp"

enum class CelestialType
{
    Sun,
    Mercury,
    Venus,
    Earth,
    Moon,
    Mars,
    Jupiter,
    Saturn,
    Uranus,
    Neptune,
    Max
};

struct CelestialData
{
    String name;
    vec3 scale;
};

static constexpr CelestialData CELESTIAL_DATA[(size_t)CelestialType::Max] = {{strL("sun"), vec3(8.f)},
    {strL("mercury"), vec3(0.4f)},
    {strL("venus"), vec3(0.9f)},
    {strL("earth"), vec3(1.f)},
    {strL("moon"), vec3(0.27f)},
    {strL("mars"), vec3(0.5f)},
    {strL("jupiter"), vec3(2.2f)},
    {strL("saturn"), vec3(1.8f)},
    {strL("uranus"), vec3(1.5f)},
    {strL("neptune"), vec3(1.45f)}};

static constexpr auto G = 1.f;

struct Celestial
{
    Entity* e;
    float mass;
    vec3 initialVelocity;
    vec3 currentVelocity;
};
