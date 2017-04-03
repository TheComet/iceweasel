#include "iceweasel/TetrahedralMesh_Face.h"
#include "iceweasel/TetrahedralMesh_Vertex.h"

#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Math/Matrix2.h>

using namespace Urho3D;
using namespace TetrahedralMesh;

// ----------------------------------------------------------------------------
Face::Face(Vertex* v0,
           Vertex* v1,
           Vertex* v2)
{
    vertex_[0] = v0;
    vertex_[1] = v1;
    vertex_[2] = v2;

    normal_ = (vertex_[1]->position_ - vertex_[0]->position_).CrossProduct(
        vertex_[2]->position_ - vertex_[0]->position_).Normalized();

    transform_ = CalculateBarycentricTransformationMatrix() *
                 CalculateSurfaceProjectionMatrix();
}

// ----------------------------------------------------------------------------
Vertex* Face::GetVertex(unsigned char vertexID)
{
    assert(vertexID < 3);
    return vertex_[vertexID];
}

// ----------------------------------------------------------------------------
const Vector3& Face::GetNormal() const
{
    return normal_;
}

// ----------------------------------------------------------------------------
void Face::FlipNormal()
{
    normal_ *= -1;
}

// ----------------------------------------------------------------------------
bool Face::PointLiesInside(const Vector3& bary) const
{
    return (
        bary.x_ >= 0.0f &&
        bary.y_ >= 0.0f &&
        bary.z_ >= 0.0f
    );
}

// ----------------------------------------------------------------------------
Vector3 Face::Intersect(const Vector3& origin, const Vector3& direction) const
{
#define NO_INTERSECTION Vector3(-1, -1, -1)
#define DO_CULL 0

    // Algorithm taken from:
    // "Fast, Minimum Storage Ray/Triangle Intersection"
    // See doc/research/
    Vector3 edge1 = vertex_[1]->position_ - vertex_[0]->position_;
    Vector3 edge2 = vertex_[2]->position_ - vertex_[0]->position_;

    Vector3 p = direction.CrossProduct(edge2);
    float determinant = p.DotProduct(edge1);
#if DO_CULL
    if(determinant == 0.0f)
        return NO_INTERSECTION;
#endif

    Vector3 ray = origin - vertex_[0]->position_;
    float u = p.DotProduct(ray);
#if DO_CULL
    if(u < 0.0f || u > determinant)
        return NO_INTERSECTION;
#endif

    Vector3 q = ray.CrossProduct(edge1);
    float v = q.DotProduct(direction);
#if DO_CULL
    if(v < 0.0f || v > determinant)
        return NO_INTERSECTION;
#endif

    determinant = 1.0f / determinant;
    u *= determinant;
    v *= determinant;

    return Vector3(1.0f - u - v, u, v);
}

// ----------------------------------------------------------------------------
Vector3 Face::ProjectAndTransformToBarycentric(const Vector3& cartesian) const
{
    Vector4 result = transform_ * Vector4(cartesian, 1.0f);
    return Vector3(result.x_, result.y_, result.z_);
}

// ----------------------------------------------------------------------------
Vector3 Face::TransformToCartesian(const Vector3& barycentric) const
{
    return barycentric.x_ * vertex_[0]->position_ +
            barycentric.y_ * vertex_[1]->position_ +
            barycentric.z_ * vertex_[2]->position_;
}

// ----------------------------------------------------------------------------
Vector3 Face::InterpolateGravity(const Vector3& barycentric) const
{
    return (
        vertex_[0]->direction_ * barycentric.x_ +
        vertex_[1]->direction_ * barycentric.y_ +
        vertex_[2]->direction_ * barycentric.z_
    ).Normalized() * (
        vertex_[0]->forceFactor_ * barycentric.x_ +
        vertex_[1]->forceFactor_ * barycentric.y_ +
        vertex_[2]->forceFactor_ * barycentric.z_
    );
}

// ----------------------------------------------------------------------------
Matrix4 Face::CalculateSurfaceProjectionMatrix() const
{
    // This function builds a projection matrix that will project a 3D point
    // onto one of the tetrahedron's triangles (namely the face that doesn't
    // contain any vertices in the infinity mask) before transforming the
    // projected point into barycentric coordinates.
    //
    // See doc/interpolating-values-using-a-tetrahedral-mesh/ for more details
    // on this, or see wikipedia:
    // https://en.wikipedia.org/wiki/Projection_(linear_algebra)#Properties_and_classification

    // Let vertex 0 be our anchor point.
    Vector3 span1 = vertex_[1]->position_ - vertex_[0]->position_;
    Vector3 span2 = vertex_[2]->position_ - vertex_[0]->position_;

    // First calculate (A^T * A)^-1
    Matrix2 B = Matrix2(
        span1.x_*span1.x_ + span1.y_*span1.y_ + span1.z_*span1.z_,
        span1.x_*span2.x_ + span1.y_*span2.y_ + span1.z_*span2.z_,
        span2.x_*span1.x_ + span2.y_*span1.y_ + span2.z_*span1.z_,
        span2.x_*span2.x_ + span2.y_*span2.y_ + span2.z_*span2.z_
    ).Inverse();

    // Sandwich the resulting vector with A * B * A^T. The result is a
    // projection matrix without offset.
    Matrix3 projectOntoTriangle = Matrix3(
        // This is matrix A
        span1.x_, span2.x_, 0,
        span1.y_, span2.y_, 0,
        span1.z_, span2.z_, 0
    ) * Matrix3(
        // Matrix multiplication B * A^T
        B.m00_*span1.x_ + B.m01_*span2.x_,
        B.m00_*span1.y_ + B.m01_*span2.y_,
        B.m00_*span1.z_ + B.m01_*span2.z_,

        B.m10_*span1.x_ + B.m11_*span2.x_,
        B.m10_*span1.y_ + B.m11_*span2.y_,
        B.m10_*span1.z_ + B.m11_*span2.z_,

        0, 0, 0
    );

    // The to-be-transformed position needs to be translated by the anchor
    // point before being projected, then translated back. This is because
    // the tetrahedron very likely isn't located at (0, 0, 0)
    Matrix4 translateToOrigin(
        1, 0, 0, -vertex_[0]->position_.x_,
        0, 1, 0, -vertex_[0]->position_.y_,
        0, 0, 1, -vertex_[0]->position_.z_,
        0, 0, 0, 1
    );

    // Create final matrix. Note that the matrices are applied in reverse order
    // in which they were multiplied.
    return translateToOrigin.Inverse() *    // #3 Restore offset
           Matrix4(projectOntoTriangle) *   // #2 Project position onto one of the tetrahedron's triangles
           translateToOrigin;               // #1 Remove offset to origin
}

// ----------------------------------------------------------------------------
Matrix4 Face::CalculateBarycentricTransformationMatrix() const
{
    // Barycentric transformation matrix
    // https://en.wikipedia.org/wiki/Barycentric_coordinate_system#Conversion_between_barycentric_and_Cartesian_coordinates
    return Matrix4(
        vertex_[0]->position_.x_, vertex_[1]->position_.x_, vertex_[2]->position_.x_, 0,
        vertex_[0]->position_.y_, vertex_[1]->position_.y_, vertex_[2]->position_.y_, 0,
        vertex_[0]->position_.z_, vertex_[1]->position_.z_, vertex_[2]->position_.z_, 0,
        1, 1, 1, 1
    ).Inverse();
}

// ----------------------------------------------------------------------------
void Face::DrawDebugGeometry(DebugRenderer* debug, bool depthTest, const Color& color) const
{
    Vector3 average(Vector3::ZERO);
    for(unsigned i = 0; i != 3; ++i)
    {
        for(unsigned j = i + 1; j != 3; ++j)
            debug->AddLine(vertex_[i]->position_, vertex_[j]->position_, color, depthTest);
        average += vertex_[i]->position_;
    }
    average /= 3.0f;
    debug->AddLine(average, average + normal_, Color::CYAN, depthTest);
}
