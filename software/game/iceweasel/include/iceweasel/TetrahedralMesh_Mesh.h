#pragma once

#include "iceweasel/TetrahedralMeshBuilder.h"
#include "iceweasel/TetrahedralMesh_Tetrahedron.h"

#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Container/Ptr.h>

class GravityVector;
namespace Urho3D {
    class DebugRenderer;
}
namespace TetrahedralMesh {
class Tetrahedron;

class Mesh : public Urho3D::RefCounted
{
public:
    Mesh();

    /*!
     * @brief Creates the gravity mesh from a shared vertex mesh (provided by
     * GravityMeshBuilder).
     *
     * The mesh is split into individual tetrahedron objects.
     */
    Mesh(const TetrahedralMeshBuilder::CircumscribedTetrahedralMesh& sharedVertexMesh);

    /*!
     * @brief Queries which tetrahedron a point is located in.
     * @param[out] barycentric If this is not NULL, then the barycentric
     * coordinates used for the bounds check are written to this parameter.
     * These can be directly used for interpolation.
     * @return If the search returned successful, the tetrahedron object is
     * returned. If no tetrahedron was found (e.g. the point exists outside of)
     * the mesh's hull), then NULL is returned.
     * @param[in] position The position to query.
     */
    bool Query(Urho3D::Vector3* gravity, const Urho3D::Vector3& position) const;

    /*!
     * @brief Replaces the existing gravity mesh (if any) with a shared vertex
     * mesh (provided by GravityMeshBuilder).
     *
     * The mesh is split into individual tetrahedron objects.
     */
    void SetMesh(const TetrahedralMeshBuilder::CircumscribedTetrahedralMesh& sharedVertexMesh);

    void DrawDebugGeometry(Urho3D::DebugRenderer* debug, bool depthTest, Urho3D::Vector3 pos);

private:
    typedef Urho3D::Vector<Tetrahedron> ContainerType;
    ContainerType tetrahedrons_;
};

}
