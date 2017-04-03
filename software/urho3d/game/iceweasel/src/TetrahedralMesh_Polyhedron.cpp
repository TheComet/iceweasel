#include "iceweasel/TetrahedralMesh_Polyhedron.h"
#include "iceweasel/TetrahedralMesh_Vertex.h"

using namespace Urho3D;
using namespace TetrahedralMesh;

// ----------------------------------------------------------------------------
void Polyhedron::AddFace(Vertex* v0, Vertex* v1, Vertex* v2)
{
    Push(SharedPtr<Vertex>(v0));
    Push(SharedPtr<Vertex>(v1));
    Push(SharedPtr<Vertex>(v2));
}

// ----------------------------------------------------------------------------
unsigned int Polyhedron::FaceCount()
{
    assert(Size() % 3 == 0);

    return Size() / 3;
}
