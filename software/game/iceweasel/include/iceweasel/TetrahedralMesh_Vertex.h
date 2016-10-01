#pragma once

#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Math/Vector3.h>

namespace TetrahedralMesh {

/*!
 * @brief Reference counted vertex designed to form gravity meshes. Holds
 * position, (gravitational) direction, and (gravitational) force factor as
 * attributes.
 */
class Vertex : public Urho3D::RefCounted
{
public:
    Vertex() : forceFactor_(0) {}
    Vertex(const Urho3D::Vector3& position,
           const Urho3D::Vector3& direction,
           float forceFactor=1.0f);

    Urho3D::Vector3 position_;
    Urho3D::Vector3 direction_;
    float forceFactor_;
};

}
