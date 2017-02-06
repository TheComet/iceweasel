#pragma once

#include <Urho3D/Scene/Component.h>

namespace Urho3D {
    class Context;
    class DebugRenderer;
}
namespace TetrahedralMesh {
    class Mesh;
    class Hull;
}
class GravityVector;

/*!
 * @brief Provides an interface to query the scene for gravitational data.
 *
 * This class manages all of the gravity vectors in the scene. A vectors
 * defines a (normalized) gravitational direction and a multiplication factor
 * for a specific location in 3D world space. Multiple vectors can be placed
 * in the scene at different locations to change the gravity of physics
 * objects.
 *
 * The final gravitational force is a combination of the global gravitational
 * force (defined by this class) and a resulting interpolated vector from
 * nearby gravity vectors.
 */
class GravityManager : public Urho3D::Component
{
    URHO3D_OBJECT(GravityManager, Urho3D::Component)

public:

    enum Strategy
    {
        TETRAHEDRAL_MESH,
        SHORTEST_DISTANCE,
        DISABLE
    };

    /*!
     * @brief Creates a new gravity component.
     */
    GravityManager(Urho3D::Context* context);

    /*!
     * @brief Destructs the gravity component.
     */
    virtual ~GravityManager();

    /*!
     * @brief Registers this class as an object factory.
     */
    static void RegisterObject(Urho3D::Context* context);

    /*!
     * @brief This is the global gravitational force. It is multiplied with the
     * individual gravity probe forces to yield the final gravitational force.
     * @param[in] acceleration Gravitational constant, in m/s^2.
     */
    void SetGlobalGravity(float acceleration)
            { gravity_ = acceleration; }

    float GetGlobalGravity() const
            { return gravity_; }

    void SetStrategy(Strategy strategy)
            { strategy_ = strategy; }

    Strategy GetStrategy() const
            { return strategy_; }

    /*!
     * @brief Queries all gravity probes and calculates the effective
     * gravitational force at the specified location in world space.
     * @param[in] worldLocation A 3D location in world space.
     * @return Returns the gravitational force at the specified location.
     */
    Urho3D::Vector3 QueryGravity(Urho3D::Vector3 worldLocation);

    void DrawDebugGeometry(Urho3D::DebugRenderer* debug, bool depthTest, Urho3D::Vector3 pos);

private:
    void RebuildTetrahedralMesh();

    /// Triggers a new search for all gravity probe nodes and rebuilds the tetrahedral mesh
    virtual void OnSceneSet(Urho3D::Scene* scene);

    /*!
     * @brief Searches for all gravity probe nodes that are located beneath the
     * specified node and caches them. The specified node is not checked.
     */
    void AddGravityVectorsRecursively(Urho3D::Node* node);

    /*!
     * @brief Searches for all gravity probe nodes that are located on and
     * beneath the specified node.
     */
    void RemoveGravityVectorsRecursively(Urho3D::Node* node);

    void HandleComponentAdded(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleComponentRemoved(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleNodeAdded(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    void HandleNodeRemoved(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    Urho3D::PODVector<GravityVector*> gravityVectors_;
    Urho3D::SharedPtr<TetrahedralMesh::Mesh> gravityMesh_;
    Urho3D::SharedPtr<TetrahedralMesh::Hull> gravityHull_;

    float gravity_;

    Strategy strategy_;
};
