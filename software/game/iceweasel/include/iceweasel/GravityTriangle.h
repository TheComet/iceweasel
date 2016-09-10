#pragma once

#include "iceweasel/GravityPoint.h"

#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Math/Vector4.h>
#include <Urho3D/Math/Matrix4.h>


namespace Urho3D {
    class Color;
    class DebugRenderer;
}

class GravityPoint;

class GravityTriangle
{
public:
    GravityTriangle() { assert(false); } // Required for Vector<GravityTriangle>

    /*!
     * @brief Constructs a triangle from 4 vertex locations in cartesian
     * space.
     */
    GravityTriangle(const GravityPoint& p0,
                    const GravityPoint& p1,
                    const GravityPoint& p2);

    const GravityPoint& GetVertex(unsigned char vertexID);

    const Urho3D::Vector3& GetNormal() const;

    void FlipNormal();

    /*!
     * @brief Returns true if the specified barycentric coordinate lies inside
     * the triangle.
     */
    bool PointLiesInside(const Vector3& bary) const;

    Urho3D::Vector3 Intersect(const Urho3D::Vector3& origin, const Urho3D::Vector3& direction) const;

    /*!
     * @brief Transforms the specified point from a cartesian coordinate system
     * into the tetrahedron's barycentric coordinate system.
     *
     * This is useful for checking if point lies inside the tetrahedron, or for
     * interpolating values.
     */
    Urho3D::Vector3 ProjectAndTransformToBarycentric(const Urho3D::Vector3& cartesian) const;

    Urho3D::Vector3 TransformToCartesian(const Urho3D::Vector3& barycentric) const;

    Urho3D::Vector3 InterpolateGravity(const Urho3D::Vector3& barycentric) const;

    void DrawDebugGeometry(Urho3D::DebugRenderer* debug, bool depthTest, const Urho3D::Color& color) const;

private:
    Urho3D::Matrix4 CalculateSurfaceProjectionMatrix() const;
    Urho3D::Matrix4 CalculateBarycentricTransformationMatrix() const;

    GravityPoint vertex_[3];
    Urho3D::Vector3 normal_;

    Urho3D::Matrix4 transform_;
};
