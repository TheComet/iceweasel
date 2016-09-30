#pragma once

#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Math/Vector3.h>


class GravityVectorComponent;

struct GravityMeshBuilder
{
    /*!
     * @brief Construct a tetrahedron by using existing Vertex objects. Allows
     * for multiple tetrahedrons to share the same vertices.
     */
    class SharedVertexTetrahedron : public Urho3D::RefCounted
    {
    public:
        SharedVertexTetrahedron() {}
        SharedVertexTetrahedron(SharedVertex* v1, SharedVertex* v2, SharedVertex* v3, SharedVertex* v4);

        Urho3D::SharedPtr<SharedVertex> v_[4];
        Urho3D::Vector3 circumscibedSphereCenter_;
    };

    /// Data type used to represent a mesh of tetrahedrons who's vertices can be referenced more than once.
    typedef Urho3D::Vector<Urho3D::SharedPtr<SharedVertexTetrahedron> > SharedTetrahedralMesh;
    /// Data type to represent a number of triangles. There will always be a multiple of 3 vertices.
    typedef Urho3D::PODVector<SharedVertex*> Polyhedron;

    /*!
     * @brief Takes a list of gravity vector components and creates a triangulated mesh.
     */
    void Build(const Urho3D::PODVector<GravityVectorComponent*>& gravityVectors);

    /*!
     * @brief After building, the resulting mesh can be retrieved with this.
     */
    const SharedTetrahedralMesh& GetSharedTetrahedralMesh() const;

    const Polyhedron& GetHullMesh() const;

private:

    /*!
     * @brief Finds all tetrahedrons who's circumsphere contains a point.
     * @param[out] badTetrahedrons All tetrahedrons containing the point will be
     * stored in this list.
     * @param[in] point The 3D point to test for.
     */
    void FindBadTetrahedrons(SharedTetrahedralMesh* badTetrahedrons, Urho3D::Vector3 point) const;

    /*!
     * @brief Creates a list of triangles are the hull of the specified list of
     * tetrahedrons.
     * @param[out] polyhedron The polyhedron (a list of triangles) is written
     * to this parameter. It will be the hull of the provided list of
     * tetrahedrons.
     * @param[in] tetrahedrons The tetrahedrons to create the hull from.
     */
    static void CreateHullFromTetrahedrons(
        Polyhedron* polyhedron, const SharedTetrahedralMesh& tetrahedrons);

    /*!
     * @brief Removes the specified tetrahedrons from the triangulation result.
     * @param[in] tetrahedrons A list of tetrahedrons to remove.
     */
    void RemoveTetrahedronsFromTriangulation(const SharedTetrahedralMesh& tetrahedrons);

    /*!
     * @brief Creates tetrahedrons by connecting each triangle of a polyhedron
     * to that of the specified gravity vector.
     * @param[in] polyhedron The hull, or polyhedron.
     * @param[in] gravityVector The position, direction and force factor of the
     * gravity vector component is used to create the new vertex to which
     * the faces are connected.
     */
    void ReTriangulateGap(const Polyhedron& polyhedron,const GravityVectorComponent& gravityVector);

    /*!
     * @brief Cleans up the triangulation result such that no more connections
     * exist to the original super tetrahedron.
     */
    void CleanUp(Urho3D::SharedPtr<SharedVertexTetrahedron> superTetrahedron);

    SharedTetrahedralMesh triangulationResult_;
    Polyhedron hull_;
};
