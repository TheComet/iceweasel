#include "iceweasel/TetrahedralMesh_Mesh.h"
#include "iceweasel/TetrahedralMeshBuilder.h"
#include "iceweasel/Math.h"
#include "iceweasel/TetrahedralMesh_Vertex.h"

#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Math/BoundingBox.h>

using namespace Urho3D;
using namespace TetrahedralMesh;

// ----------------------------------------------------------------------------
Mesh::Mesh()
{
}

// ----------------------------------------------------------------------------
Mesh::Mesh(const TetrahedralMeshBuilder::CircumscribedTetrahedralMesh& sharedVertexMesh)
{
    SetMesh(sharedVertexMesh);
}

// ----------------------------------------------------------------------------
void Mesh::SetMesh(const TetrahedralMeshBuilder::CircumscribedTetrahedralMesh& sharedVertexMesh)
{
    tetrahedrons_.Clear();

    for(TetrahedralMeshBuilder::CircumscribedTetrahedralMesh::ConstIterator it = sharedVertexMesh.Begin();
        it != sharedVertexMesh.End();
        ++it)
    {
        TetrahedralMeshBuilder::CircumscribedTetrahedron* t = *it;
        tetrahedrons_.Push(Tetrahedron(t->v_[0], t->v_[1], t->v_[2], t->v_[3]));
    }
}

// ----------------------------------------------------------------------------
bool Mesh::Query(Vector3* gravity, const Vector3& position) const
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
void Mesh::DrawDebugGeometry(DebugRenderer* debug, bool depthTest, Vector3 pos)
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
