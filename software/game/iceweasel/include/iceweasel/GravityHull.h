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

    GravityHull(const GravityMeshBuilder::Polyhedron& polyhedron);

    void SetMesh(const GravityMeshBuilder::Polyhedron& polyhedron);

    bool Query(Urho3D::Vector3* gravity, const Urho3D::Vector3& position);

    void DrawDebugGeometry(Urho3D::DebugRenderer* debug, bool depthTest, Urho3D::Vector3 pos) const;

private:
    Urho3D::Vector3 centre_;
    Urho3D::Vector<GravityTriangle> triangles_;
    Urho3D::Vector<GravityEdge> edges_;
    Urho3D::Vector<GravityPoint> points_;

    Urho3D::Vector3 lastIntersection_;
};
