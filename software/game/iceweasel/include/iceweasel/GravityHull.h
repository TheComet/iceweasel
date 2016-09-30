#pragma once

#include "iceweasel/GravityMeshBuilder.h"
#include "iceweasel/GravityTriangle.h"
#include "iceweasel/GravityEdge.h"

#include <Urho3D/Container/Ptr.h>

namespace Urho3D {
    class DebugRenderer;
}


class GravityHull : public Urho3D::RefCounted
{
public:
    GravityHull();

    GravityHull(const GravityMesh::Polyhedron& polyhedron);

    void SetMesh(const GravityMesh::Polyhedron& polyhedron);

    bool Query(Urho3D::Vector3* gravity, const Urho3D::Vector3& position);

    void DrawDebugGeometry(Urho3D::DebugRenderer* debug, bool depthTest, Urho3D::Vector3 pos) const;

private:
    Urho3D::Vector3 centre_;
    Urho3D::Vector<Urho3D::SharedPtr<GravityMesh::Face> > triangles_;
    Urho3D::Vector<Urho3D::SharedPtr<GravityMesh::Edge> > edges_;
    Urho3D::Vector<Urho3D::SharedPtr<GravityMesh::Vertex> > vertices_;

    Urho3D::Vector3 lastIntersection_;
};
