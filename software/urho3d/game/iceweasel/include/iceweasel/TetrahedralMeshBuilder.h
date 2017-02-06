#pragma once

#include "iceweasel/TetrahedralMesh_Vertex.h"
#include "iceweasel/TetrahedralMesh_Polyhedron.h"

#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Math/Vector3.h>

class GravityVector;

struct TetrahedralMeshBuilder
{
    /*!
     * @brief Construct a tetrahedron by using existing Vertex objects. Allows
     * for multiple tetrahedrons to share the same vertices.
     */
    class CircumscribedTetrahedron : public Urho3D::RefCounted
    {
    public:
        CircumscribedTetrahedron() {}
        CircumscribedTetrahedron(TetrahedralMesh::Vertex* v1, TetrahedralMesh::Vertex* v2,
                                 TetrahedralMesh::Vertex* v3, TetrahedralMesh::Vertex* v4);

        Urho3D::SharedPtr<TetrahedralMesh::Vertex> v_[4];
        Urho3D::Vector3 circumscibedSphereCenter_;
    };

    typedef Urho3D::Vector<Urho3D::SharedPtr<CircumscribedTetrahedron> > CircumscribedTetrahedralMesh;

    /*!
     * @brief Takes a list of gravity vector components and creates a triangulated mesh.
     */
    void Build(const Urho3D::PODVector<GravityVector*>& gravityVectors);

    /*!
     * @brief After building, the resulting mesh can be retrieved with this.
     */
    const CircumscribedTetrahedralMesh& GetTetrahedralMesh() const;

    TetrahedralMesh::Polyhedron* GetHullMesh() const;

private:

    /*!
     * @brief Finds all tetrahedrons who's circumsphere contains a point.
     * @param[out] badTetrahedrons All tetrahedrons containing the point will be
     * stored in this list.
     * @param[in] point The 3D point to test for.
     */
    void FindBadTetrahedrons(CircumscribedTetrahedralMesh* badTetrahedrons, Urho3D::Vector3 point) const;

    /*!
     * @brief Creates a list of triangles are the hull of the specified list of
     * tetrahedrons.
     * @param[out] polyhedron The polyhedron (a list of triangles) is written
     * to this parameter. It will be the hull of the provided list of
     * tetrahedrons.
     * @param[in] tetrahedrons The tetrahedrons to create the hull from.
     */
    static void CreateHullFromTetrahedrons(TetrahedralMesh::Polyhedron* polyhedron,
                                           const CircumscribedTetrahedralMesh& tetrahedrons);

    /*!
     * @brief Removes the specified tetrahedrons from the triangulation result.
     * @param[in] tetrahedrons A list of tetrahedrons to remove.
     */
    void RemoveTetrahedronsFromTriangulation(const CircumscribedTetrahedralMesh& tetrahedrons);

    /*!
     * @brief Creates tetrahedrons by connecting each triangle of a polyhedron
     * to that of the specified gravity vector.
     * @param[in] polyhedron The hull, or polyhedron.
     * @param[in] gravityVector The position, direction and force factor of the
     * gravity vector component is used to create the new vertex to which
     * the faces are connected.
     */
    void ReTriangulateGap(const TetrahedralMesh::Polyhedron& polyhedron, const GravityVector& gravityVector);

    /*!
     * @brief Cleans up the triangulation result such that no more connections
     * exist to the original super tetrahedron.
     */
    void CleanUp(const CircumscribedTetrahedron* superTetrahedron);

    CircumscribedTetrahedralMesh triangulationResult_;
    TetrahedralMesh::Polyhedron* hull_;
};
