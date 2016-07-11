#pragma once

#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Container/Ptr.h>
#include "iceweasel/Tetrahedron.h"


namespace Urho3D
{

class TetrahedralMesh : public RefCounted
{
public:
    TetrahedralMesh(const Vector<Vector3>& vertexList);

    void DrawDebugGeometry(DebugRenderer* debug, bool depthTest, Vector3 pos);

private:
    void Construct(const Vector<Vector3>& vertexList);

    Vector<Tetrahedron> tetrahedrons_;
};

} // namespace Urho3D
