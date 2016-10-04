#pragma once

#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Math/Vector4.h>
#include <Urho3D/Math/Matrix4.h>


namespace Urho3D {
    class Color;
    class DebugRenderer;
}

namespace TetrahedralMesh {
class Vertex;

class Edge
{
public:
    Edge() {} // Required for Vector<GravityEdge>

    /*!
     * @brief Constructs a triangle from 4 vertex locations in cartesian
     * space.
     */
    Edge(Vertex* v0,
         Vertex* v1,
         const Urho3D::Vector3& boundaryNormal0,
         const Urho3D::Vector3& boundaryNormal1);

    void FlipBoundaryCheck();

    /*!
     * @brief Returns true if the specified barycentric coordinate lies inside
     * the triangle.
     */
    bool PointLiesInside(Urho3D::Vector2 bary) const;

    bool ProjectionAngleIsInBounds(const Urho3D::Vector3& cartesianTransform,
                                   const Urho3D::Vector3& position) const;

    /*!
     * @brief Transforms the specified point from a cartesian coordinate system
     * into the tetrahedron's barycentric coordinate system.
     *
     * This is useful for checking if point lies inside the tetrahedron, or for
     * interpolating values.
     */
    Urho3D::Vector2 ProjectAndTransformToBarycentric(const Urho3D::Vector3& cartesian) const;

    Urho3D::Vector3 TransformToCartesian(const Urho3D::Vector2& barycentric) const;

    Urho3D::Vector3 InterpolateGravity(const Urho3D::Vector2& barycentric) const;

    void DrawDebugGeometry(Urho3D::DebugRenderer* debug, bool depthTest,
                           const Urho3D::Color& color) const;

private:
    Urho3D::Matrix4 CalculateEdgeProjectionMatrix() const;
    Urho3D::Matrix4 CalculateBarycentricTransformationMatrix() const;

    Urho3D::SharedPtr<Vertex> vertex_[2];
    Urho3D::Vector3 boundaryNormal_[2];

    Urho3D::Matrix4 transform_;
};

}
