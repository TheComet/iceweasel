#pragma once

#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Container/Vector.h>

namespace TetrahedralMesh {
class Vertex;

/*!
 * @brief Data type to represent a number of triangles. There will always be a multiple of 3 vertices.
 */
class Polyhedron : public Urho3D::RefCounted,
                   public Urho3D::Vector<Urho3D::SharedPtr<Vertex> >
{
public:
    void AddFace(Vertex* v0, Vertex* v1, Vertex* v2);
    unsigned FaceCount();
};

}
