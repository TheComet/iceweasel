#include "iceweasel/Tetrahedron.h"

#include <Urho3D/Graphics/DebugRenderer.h>


namespace Urho3D
{

// ----------------------------------------------------------------------------
Tetrahedron::Tetrahedron(const Vector3& v0,
                         const Vector3& v1,
                         const Vector3& v2,
                         const Vector3& v3)
{
    vertices_[0] = v0;
    vertices_[1] = v1;
    vertices_[2] = v2;
    vertices_[3] = v3;
    PrecomputeBarycentricMatrix();
}

// ----------------------------------------------------------------------------
void Tetrahedron::InvertVolume(unsigned vertexID)
{
    assert(vertexID < 4);

    int i = 4;
    float* column = &barycentricTransform_.m00_ + vertexID * 4;
    while(i--)
        *column++ *= -1;
}

// ----------------------------------------------------------------------------
void Tetrahedron::PrecomputeBarycentricMatrix()
{
    // https://en.wikipedia.org/wiki/Barycentric_coordinate_system#Conversion_between_barycentric_and_Cartesian_coordinates
    barycentricTransform_ = Matrix4(
        vertices_[0].x_, vertices_[1].x_, vertices_[2].x_, vertices_[3].x_,
        vertices_[0].y_, vertices_[1].y_, vertices_[2].y_, vertices_[3].y_,
        vertices_[0].z_, vertices_[1].z_, vertices_[2].z_, vertices_[3].z_,
        1, 1, 1, 1
    ).Inverse();
}

// ----------------------------------------------------------------------------
void Tetrahedron::DrawDebugGeometry(DebugRenderer* debug, bool depthTest, const Color& color)
{
    for(unsigned i = 0; i != 4; ++i)
    {
        for(unsigned j = i + 1; j != 4; ++j)
            debug->AddLine(
                Vector3(vertices_[i].x_, vertices_[i].y_, vertices_[i].z_),
                Vector3(vertices_[j].x_, vertices_[j].y_, vertices_[j].z_),
                color, depthTest
            );
    }
}

} // namespace Urho3D
