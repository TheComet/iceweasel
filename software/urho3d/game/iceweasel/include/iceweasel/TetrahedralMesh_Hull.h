#pragma once

#include "iceweasel/TetrahedralMesh_Edge.h"
#include "iceweasel/TetrahedralMesh_Face.h"
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Math/Vector3.h>

namespace Urho3D {
    class DebugRenderer;
}
namespace TetrahedralMesh {
class Polyhedron;
class Vertex;
class Edge;
class Face;

class Hull : public Urho3D::RefCounted
{
public:
    Hull();

    Hull(Polyhedron* polyhedron);

    void SetMesh(Polyhedron* polyhedron);

    bool Query(Urho3D::Vector3* gravity, const Urho3D::Vector3& position);

    void DrawDebugGeometry(Urho3D::DebugRenderer* debug, bool depthTest, Urho3D::Vector3 pos) const;

private:
    Urho3D::Vector3 centre_;
    Urho3D::Vector<Edge> edges_;
    Urho3D::Vector<Face> faces_;
    Urho3D::SharedPtr<Polyhedron> hullMesh_;

    Urho3D::Vector3 lastIntersection_;
};

}
