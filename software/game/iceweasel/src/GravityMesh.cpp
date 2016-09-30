#include "iceweasel/GravityMesh.h"
#include "iceweasel/GravityMeshBuilder.h"
#include "iceweasel/Math.h"
#include "iceweasel/GravityVector.h"

#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Math/BoundingBox.h>

using namespace Urho3D;

// ----------------------------------------------------------------------------
GravityMesh::GravityMesh()
{
}

// ----------------------------------------------------------------------------
GravityMesh::GravityMesh(const GravityMeshBuilder::SharedTetrahedralMesh& sharedVertexMesh)
{
    SetMesh(sharedVertexMesh);
}

// ----------------------------------------------------------------------------
void GravityMesh::SetMesh(const GravityMeshBuilder::SharedTetrahedralMesh& sharedVertexMesh)
{
    tetrahedrons_.Clear();

    for(GravityMeshBuilder::SharedTetrahedralMesh::ConstIterator it = sharedVertexMesh.Begin();
        it != sharedVertexMesh.End();
        ++it)
    {
        GravityMeshBuilder::SharedVertexTetrahedron* t = *it;

        tetrahedrons_.Push(Tetrahedron(
            GravityPoint(t->v_[0]->position_, t->v_[0]->direction_, t->v_[0]->forceFactor_),
            GravityPoint(t->v_[1]->position_, t->v_[1]->direction_, t->v_[1]->forceFactor_),
            GravityPoint(t->v_[2]->position_, t->v_[2]->direction_, t->v_[2]->forceFactor_),
            GravityPoint(t->v_[3]->position_, t->v_[3]->direction_, t->v_[3]->forceFactor_)
        ));
    }
}

// ----------------------------------------------------------------------------
bool GravityMesh::Query(Vector3* gravity, const Vector3& position) const
{
    // Use a linear search for now. Can optimise later
    Vector<Tetrahedron>::ConstIterator tetrahedron = tetrahedrons_.Begin();
    for(; tetrahedron != tetrahedrons_.End(); ++tetrahedron)
    {
        Vector4 bary = tetrahedron->TransformToBarycentric(position);
        if(tetrahedron->PointLiesInside(bary))
        {
            if(gravity != NULL)
                *gravity = tetrahedron->InterpolateGravity(bary);
            return true;
        }
    }

    return false;
}

// ----------------------------------------------------------------------------
void GravityMesh::DrawDebugGeometry(DebugRenderer* debug, bool depthTest, Vector3 pos)
{
    unsigned count = 0;
    Vector<Tetrahedron>::Iterator it = tetrahedrons_.Begin();
    for(; it != tetrahedrons_.End(); ++it)
        if(it->PointLiesInside(it->TransformToBarycentric(pos)))
        {
            it->DrawDebugGeometry(debug, false, Color::RED);
            ++count;
        }
        else
            it->DrawDebugGeometry(debug, depthTest, Color::GRAY);

    //assert(count < 2); // Detects overlapping tetrahedrons (should never happen)
}
