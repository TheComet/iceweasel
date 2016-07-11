#pragma once

#include <Urho3D/Math/Vector3.h>
#include <Urho3D/Container/Ptr.h>
#include "iceweasel/Tetrahedron.h"


namespace Urho3D
{

class TetrahedralMesh : public RefCounted
{
public:

    /*!
     * @brief Triangulates the list of 3D positions into a tetrahedral mesh.
     */
    TetrahedralMesh(const Vector<Vector3>& pointList);

    /*!
     * @brief Queries which tetrahedron a point is located in.
     * @param[in] position The position to query.
     * @param[out] barycentric If this is not NULL, then the barycentric
     * coordinates used for the bounds check are written to this parameter.
     * These can be directly used for interpolation.
     * @return If the search returned successful, the tetrahedron object is
     * returned. If no tetrahedron was found (e.g. the point exists outside of)
     * the mesh's hull), then NULL is returned.
     */
    const Tetrahedron* Query(const Vector3& position, Vector4* barycentric) const;

    void DrawDebugGeometry(DebugRenderer* debug, bool depthTest, Vector3 pos);

private:
    void Construct(const Vector<Vector3>& pointList);

    Vector<Tetrahedron> tetrahedrons_;
};

} // namespace Urho3D
