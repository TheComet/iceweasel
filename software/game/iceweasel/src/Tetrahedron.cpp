#include "iceweasel/Tetrahedron.h"

#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Math/Matrix2.h>


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
/*
void Tetrahedron::InvertVolume(unsigned int vertexID)
{
    assert(vertexID < 4);

    int i = 4;
    float* column = &barycentricTransform_.m00_ + vertexID * 4;
    while(i--)
        *column++ *= -1;
}*/

// ----------------------------------------------------------------------------
void Tetrahedron::ExtendIntoInfinity(unsigned vertexID)
{
    assert(vertexID < 4);

    // Construct projection matrix onto the triangle of the tetrahedron that
    // doesn't share the specified vertex. See doc for more details on this, or see wikipedia
    // https://en.wikipedia.org/wiki/Projection_(linear_algebra)#Properties_and_classification

    // Select 3 vertices that exclude vertexID (since vertexID is the
    // infinitely far away vertex)
    const Vector3* vertices[3];
    for(unsigned i = 0; i != 3; ++i)
        vertices[i] = i < vertexID ? &vertices_[i] : &vertices_[i + 1];

    // Let vertex 0 be our anchor point.
    Vector3 span1 = *vertices[1] - *vertices[0];
    Vector3 span2 = *vertices[2] - *vertices[0];

    // First calculate (A^T * A)^-1
    Matrix2 b = Matrix2(
        span1.x_*span1.x_ + span1.y_*span1.y_ + span1.z_*span1.z_,
        span1.x_*span2.x_ + span1.y_*span2.y_ + span1.z_*span2.z_,
        span2.x_*span1.x_ + span2.y_*span1.y_ + span2.z_*span1.z_,
        span2.x_*span2.x_ + span2.y_*span2.y_ + span2.z_*span2.z_
    ).Inverse();

    // Sandwich the resulting vector with A * b * A^T. The result is a
    // projection matrix without offset.
    Matrix3 projection = Matrix3(
        // This is matrix A
        span1.x_, span2.x_, 0,
        span1.y_, span2.y_, 0,
        span1.z_, span2.z_, 0
    ) * Matrix3(
        // Matrix multiplication b * A^T
        b.m00_*span1.x_ + b.m01_*span2.x_,
        b.m00_*span1.y_ + b.m01_*span2.y_,
        b.m00_*span1.z_ + b.m01_*span2.z_,

        b.m10_*span1.x_ + b.m11_*span2.x_,
        b.m10_*span1.y_ + b.m11_*span2.y_,
        b.m10_*span1.z_ + b.m11_*span2.z_,

        0, 0, 0
    );

    barycentricTransform_ = Matrix4(
        // Projection needs to be translated by the anchor point, because the
        // triangle's anchor point isn't located at (0, 0, 0)
        1, 0, 0, -vertices[0]->x_,
        0, 1, 0, -vertices[0]->y_,
        0, 0, 1, -vertices[0]->z_,
        0, 0, 0, 1
    ) * Matrix4(projection);
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
