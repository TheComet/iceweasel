#include "iceweasel/Tetrahedron.h"

using namespace Urho3D;


// ----------------------------------------------------------------------------
Tetrahedron::Tetrahedron(const Vector3& v0,
                         const Vector3& v1,
                         const Vector3& v2,
                         const Vector3& v3) :
    vertices_{v0, v1, v2, v3}
{
    PrecomputeBarycentricMatrix();
}

// ----------------------------------------------------------------------------
void Tetrahedron::PrecomputeBarycentricMatrix()
{
    Vector3 edge1 = vertices_[0] - vertices_[3];
    Vector3 edge2 = vertices_[1] - vertices_[3];
    Vector3 edge3 = vertices_[2] - vertices_[3];

    barycentricMatrix_ = Matrix3(
        edge1.x_, edge2.x_, edge3.x_,
        edge1.y_, edge2.y_, edge3.y_,
        edge1.z_, edge2.z_, edge3.z_
    ).Inverse();
}
