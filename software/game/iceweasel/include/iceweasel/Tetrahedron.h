#pragma once

#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Math/Vector4.h>
#include <Urho3D/Math/Matrix3.h>


class Tetrahedron
{
public:

    /*!
     * @brief Constructs a tetrahedron from 4 vertex locations.
     */
    Tetrahedron(const Urho3D::Vector3& v0,
                const Urho3D::Vector3& v1,
                const Urho3D::Vector3& v2,
                const Urho3D::Vector3& v3);

    /*!
     * @brief Returns true if the specified 3D point lies inside the
     * tetrahedron.
     */
    bool PointLiesInside(const Urho3D::Vector3& point)
    {
        Urho3D::Vector4 bary = TransformToBarycentric(point);

        return (
            0.0f <= bary.x_ && bary.x_ <= 1.0f &&
            0.0f <= bary.y_ && bary.y_ <= 1.0f &&
            0.0f <= bary.z_ && bary.z_ <= 1.0f &&
            0.0f <= bary.w_ && bary.w_ <= 1.0f
        );
    }

    /*!
     * @brief Transforms the specified point into the tetrahedron's barycentric
     * coordinate system.
     *
     * This is useful for checking if point lies inside the tetrahedron, or for
     * interpolating values.
     */
    Urho3D::Vector4 TransformToBarycentric(const Urho3D::Vector3& point) const
    {
        Urho3D::Vector3 bary = barycentricTransform_ * (point - vertices_[3]);
        return Urho3D::Vector4(bary, 1.0f - bary.x_ - bary.y_ - bary.z_);
    }

private:
    void PrecomputeBarycentricMatrix();

    Urho3D::Vector3 vertices_[4];
    Urho3D::Matrix3 barycentricTransform_;
};
