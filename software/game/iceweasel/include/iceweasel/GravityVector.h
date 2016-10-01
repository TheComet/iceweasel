#pragma once

#include <Urho3D/Scene/Component.h>


namespace Urho3D {
    class Context;
}

class GravityVector : public Urho3D::Component
{
    URHO3D_OBJECT(GravityVector, Component)

public:

    /*!
     * @brief Constructs a new gravity probe.
     */
    GravityVector(Urho3D::Context* context);

    /*!
     * @brief Destructs the gravity probe.
     */
    virtual ~GravityVector();

    /*!
     * @brief Registers this class as an object factory.
     */
    static void RegisterObject(Urho3D::Context* context);

    /*!
     * @brief Sets the world position of the gravity probe.
     * @param[in] position World location as Vector3.
     */
    void SetPosition(Urho3D::Vector3 position);

    /*!
     * @brief Gets the world position of the gravity probe.
     */
    Urho3D::Vector3 GetPosition() const;

    /*!
     * @brief Sets in which world direction gravity should be applied to
     * objects.
     * @param[in] direction A directional Vector3 in world space. It will be
     * normalized internally..
     */
    void SetDirection(Urho3D::Vector3 direction);

    /*!
     * @brief Returns the normalized direction of the node.
     * @return A normalized Vector3.
     */
    Urho3D::Vector3 GetDirection() const;

    /*!
     * @brief The global gravity is multiplied by this factor. The default
     * value is 1.0.
     * @param[in] factor Multiplication factor to apply to this probe.
     */
    void SetForceFactor(float factor)
            { forceFactor_ = factor; }

    /*!
     * @brief Gets the probe's force factor. This is multiplied with the global
     * gravity setting to calculate the final gravitational force. Defaults to
     * 1.0.
     */
    float GetForceFactor() const
            { return forceFactor_; }

    virtual void DrawDebugGeometry(Urho3D::DebugRenderer* debug, bool depthTest);

private:
    float forceFactor_;
};
