#pragma once

#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Math/Vector4.h>
#include <Urho3D/Math/Matrix4.h>


namespace Urho3D
{

class DebugRenderer;
class Color;


class Tetrahedron
{
public:

    /*!
     * @brief Constructs a tetrahedron from 4 vertex locations in cartesian
     * space.
     */
    Tetrahedron(const Vector3& v0,
                const Vector3& v1,
                const Vector3& v2,
                const Vector3& v3);

    void ExtendIntoInfinity(unsigned vertexID);

    /*!
     * @brief Returns true if the specified 3D point lies inside the
     * tetrahedron.
     */
    bool PointLiesInside(const Vector3& point)
    {
        Vector4 bary = TransformToBarycentric(point);

        return (
            bary.x_ >= 0.0f &&
            bary.y_ >= 0.0f &&
            bary.z_ >= 0.0f &&
            bary.w_ >= 0.0f
        );
    }

    /*!
     * @brief Transforms the specified point from a cartesian coordinate system
     * into the tetrahedron's barycentric coordinate system.
     *
     * This is useful for checking if point lies inside the tetrahedron, or for
     * interpolating values.
     */
    Vector4 TransformToBarycentric(const Vector3& point) const
    {
        return barycentricTransform_ * Vector4(point, 1.0f);
    }

    void DrawDebugGeometry(DebugRenderer* debug, bool depthTest, const Color& color);

private:
    void PrecomputeBarycentricMatrix();

    Vector3 vertices_[4];
    Matrix4 barycentricTransform_;
};

} // namespace Urho3D
