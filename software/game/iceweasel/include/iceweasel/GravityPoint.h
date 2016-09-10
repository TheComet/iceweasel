#pragma once

#include <Urho3D/Math/Vector3.h>

class GravityPoint
{
public:
    GravityPoint() : forceFactor_(0) {}
    GravityPoint(const Urho3D::Vector3& vertex, const Urho3D::Vector3& direction, float forceFactor);

    Urho3D::Vector3 position_;
    Urho3D::Vector3 direction_;
    float forceFactor_;
};
