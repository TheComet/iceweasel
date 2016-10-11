#pragma once
#include <Urho3D/Core/Variant.h>

struct Args : public Urho3D::RefCounted
{
    Args();

    Urho3D::StringVector resourcePaths_;
    Urho3D::String sceneName_;
    bool editor_;
    bool headless_;
    bool windowed_;
    bool vsync_;
    int multisample_;
};
