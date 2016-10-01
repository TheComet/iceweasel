#include "iceweasel/TetrahedralMesh_Edge.h"
#include "iceweasel/TetrahedralMesh_Vertex.h"

#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Math/Matrix2.h>

using namespace Urho3D;
using namespace TetrahedralMesh;

// ----------------------------------------------------------------------------
Edge::Edge(Vertex* v0,
           Vertex* v1,
           const Vector3& boundaryNormal0,
           const Vector3& boundaryNormal1)
{
    vertex_[0] = v0;
    vertex_[1] = v1;
    boundaryNormal_[0] = boundaryNormal0;
    boundaryNormal_[1] = boundaryNormal1;

    Matrix4 bt = CalculateBarycentricTransformationMatrix();
    Matrix4 et = CalculateEdgeProjectionMatrix();
    transform_ = bt * et;
                 ;
}

// ----------------------------------------------------------------------------
void Edge::FlipBoundaryCheck()
{
    Vector3 tmp = boundaryNormal_[0];
    boundaryNormal_[0] = boundaryNormal_[1];
    boundaryNormal_[1] = tmp;
}

// ----------------------------------------------------------------------------
bool Edge::PointLiesInside(Vector2 bary) const
{
    return (
        bary.x_ >= 0.0f &&
        bary.y_ >= 0.0f
    );
}

// ----------------------------------------------------------------------------
bool Edge::ProjectionAngleIsInBounds(const Vector3& cartesianTransform,
                                            const Vector3& position) const
{
    Vector3 check = cartesianTransform - position;
    Vector3 cross = boundaryNormal_[0].CrossProduct(check);

    if(cross.DotProduct(check.CrossProduct(boundaryNormal_[1])) > 0 &&
        (vertex_[1]->position_ - vertex_[0]->position_).DotProduct(cross) > 0)
        return true;

    return false;
}

// ----------------------------------------------------------------------------
Vector2 Edge::ProjectAndTransformToBarycentric(const Vector3& cartesian) const
{
    Vector4 result = transform_ * Vector4(cartesian, 1.0f);
    return Vector2(result.x_, result.y_);
}

// ----------------------------------------------------------------------------
Vector3 Edge::TransformToCartesian(const Vector2& barycentric) const
{
    return barycentric.x_ * vertex_[0]->position_ +
            barycentric.y_ * vertex_[1]->position_;
}

// ----------------------------------------------------------------------------
Vector3 Edge::InterpolateGravity(const Vector2& barycentric) const
{
    return (
        vertex_[0]->direction_ * barycentric.x_ +
        vertex_[1]->direction_ * barycentric.y_
    ).Normalized() * (
        vertex_[0]->forceFactor_ * barycentric.x_ +
        vertex_[1]->forceFactor_ * barycentric.y_
    );
}

// ----------------------------------------------------------------------------
Matrix4 Edge::CalculateEdgeProjectionMatrix() const
{
    // This function builds a projection matrix that will project a 3D point
    // onto one of the tetrahedron's edges (namely the edges that doesn't
    // contain any vertices in the infinity mask) before transforming the
    // projected point into barycentric coordinates.
    //
    // See doc/interpolating-values-using-a-tetrahedral-mesh/ for more details
    // on this, or see wikipedia:
    // https://en.wikipedia.org/wiki/Projection_(linear_algebra)#Properties_and_classification

    // Let vertex 0 be our anchor point.
    Vector3 span = vertex_[1]->position_ - vertex_[0]->position_;

    // First calculate (A^T * A)^-1
    float B = span.x_*span.x_ + span.y_*span.y_ + span.z_*span.z_;
    B = 1.0f / B; // Inverse

    // Sandwich the resulting vector with A * B * A^T. The result is a
    // projection matrix without offset.
    Matrix3 projectOntoTriangle = Matrix3(
        // This is matrix A
        span.x_, 0, 0,
        span.y_, 0, 0,
        span.z_, 0, 0
    ) * Matrix3(
        // Matrix multiplication B * A^T
        B*span.x_,
        B*span.y_,
        B*span.z_,

        0, 0, 0,
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
Matrix4 Edge::CalculateBarycentricTransformationMatrix() const
{
    Vector3 fix1 = vertex_[0]->position_.CrossProduct(vertex_[1]->position_);
    Vector3 fix2 = fix1 + vertex_[0]->position_;
    // Barycentric transformation matrix
    // https://en.wikipedia.org/wiki/Barycentric_coordinate_system#Conversion_between_barycentric_and_Cartesian_coordinates
    return Matrix4(
        vertex_[0]->position_.x_, vertex_[1]->position_.x_, fix1.x_, fix2.x_,
        vertex_[0]->position_.y_, vertex_[1]->position_.y_, fix1.y_, fix2.y_,
        vertex_[0]->position_.z_, vertex_[1]->position_.z_, fix1.z_, fix2.z_,
        1, 1, 1, 1
    ).Inverse();
}

// ----------------------------------------------------------------------------
void Edge::DrawDebugGeometry(DebugRenderer* debug, bool depthTest, const Color& color) const
{
    debug->AddLine(vertex_[0]->position_, vertex_[1]->position_, color, depthTest);

    debug->AddLine(vertex_[0]->position_, vertex_[0]->position_ + boundaryNormal_[0], Color::CYAN, depthTest);
    debug->AddLine(vertex_[0]->position_, vertex_[0]->position_ + boundaryNormal_[1], Color::CYAN, depthTest);
    debug->AddLine(vertex_[1]->position_, vertex_[1]->position_ + boundaryNormal_[0], Color::CYAN, depthTest);
    debug->AddLine(vertex_[1]->position_, vertex_[1]->position_ + boundaryNormal_[1], Color::CYAN, depthTest);
}
