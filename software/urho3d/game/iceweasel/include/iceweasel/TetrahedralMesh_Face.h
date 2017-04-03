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

class Face
{
public:
    Face() {} // Required for Vector<GravityTriangle>

    /*!
     * @brief Constructs a triangle from 3 vertices.
     */
    Face(Vertex* v0,
         Vertex* v1,
         Vertex* v2);

    /*!
     * @param vertexID The ID (0, 1 or 2) of the vertex to get.
     * @return Returns the specified vertex.
     */
    Vertex* GetVertex(unsigned char vertexID);

    /*!
     * @return Retrieves the face's normal vector.
     */
    const Urho3D::Vector3& GetNormal() const;

    /*!
     * Inverts the direction of the face's normal vector.
     */
    void FlipNormal();

    /*!
     * @brief Returns true if the specified barycentric coordinate lies inside
     * the triangle.
     */
    bool PointLiesInside(const Urho3D::Vector3& bary) const;

    /*!
     * Calculates the point of intersection from a ray onto the face.
     * @param origin Where the ray begins.
     * @param direction Normalised vector of the direction of the ray.
     */
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

    Urho3D::SharedPtr<Vertex> vertex_[3];
    Urho3D::Vector3 normal_;

    Urho3D::Matrix4 transform_;
};

}
