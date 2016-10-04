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

class Tetrahedron
{
public:

    Tetrahedron() {} // Required for Vector<GravityTetrahedron>

    /*!
     * @brief Constructs a tetrahedron from 4 vertex locations in cartesian
     * space.
     */
    Tetrahedron(Vertex* v0,
                Vertex* v1,
                Vertex* v2,
                Vertex* v3);

    /*!
     * @brief Returns true if the specified barycentric coordinate lies inside
     * the tetrahedron.
     */
    bool PointLiesInside(const Urho3D::Vector4& bary) const;

    /*!
     * @brief Transforms the specified point from a cartesian coordinate system
     * into the tetrahedron's barycentric coordinate system.
     *
     * This is useful for checking if point lies inside the tetrahedron, or for
     * interpolating values.
     */
    Urho3D::Vector4 TransformToBarycentric(const Urho3D::Vector3& cartesian) const;

    Urho3D::Vector3 TransformToCartesian(const Urho3D::Vector4& barycentric) const;

    Urho3D::Vector3 GetVertexPosition(unsigned char vertexID) const;

    Urho3D::Vector3 InterpolateGravity(const Urho3D::Vector4& barycentric) const;

    void DrawDebugGeometry(Urho3D::DebugRenderer* debug, bool depthTest, const Urho3D::Color& color);

private:
    Urho3D::Matrix4 CalculateBarycentricTransformationMatrix() const;

    Urho3D::SharedPtr<Vertex> vertex_[4];
    Urho3D::Matrix4 transform_;
};

}
