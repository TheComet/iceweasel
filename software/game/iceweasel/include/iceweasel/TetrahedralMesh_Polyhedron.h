#pragma once

#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Container/Vector.h>

namespace TetrahedralMesh {
class Vertex;

/*!
 * @brief Data type to represent a number of triangles. There will always be a multiple of 3 vertices.
 */
class Polyhedron : public Urho3D::RefCounted
{
public:
    typedef Urho3D::Vector<Urho3D::SharedPtr<Vertex> > ContainerType;
    typedef ContainerType::Iterator Iterator;
    typedef ContainerType::ConstIterator ConstIterator;

    Polyhedron() {}

    void AddFace(Vertex* v0, Vertex* v1, Vertex* v2);

    unsigned FaceCount();

    Iterator Begin();
    Iterator End();
    ConstIterator Begin() const;
    ConstIterator End() const;

private:
    ContainerType vertices_;
};

}
