#include "iceweasel/TetrahedralMesh_Vertex.h"

using namespace Urho3D;
using namespace TetrahedralMesh;

// ----------------------------------------------------------------------------
Vertex::Vertex(const Vector3& vertex, const Vector3& direction, float forceFactor) :
    position_(vertex),
    direction_(direction),
    forceFactor_(forceFactor)
{
}
