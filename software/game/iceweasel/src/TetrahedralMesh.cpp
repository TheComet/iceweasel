#include "iceweasel/TetrahedralMesh.h"

#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Math/BoundingBox.h>
#include <iceweasel/Math.h>
#include "iceweasel/Tetrahedron.h"


namespace Urho3D
{

namespace internal {


class Vertex : public RefCounted
{
public:
    Vertex(const Vector3& position) : position_(position) {}
    Vector3 position_;
};

class Tetrahedron : public RefCounted
{
public:
    Tetrahedron();
    Tetrahedron(Vertex* v1, Vertex* v2, Vertex* v3, Vertex* v4) { Set(v1, v2, v3, v4); }
    void Set(Vertex* v1, Vertex* v2, Vertex* v3, Vertex* v4) {
        v_[0] = v1; v_[1] = v2; v_[2] = v3; v_[3] = v4;
        circumscibedSphereCenter_ = Math::CircumscribeSphere(v1->position_, v2->position_, v3->position_, v4->position_);
    }

    SharedPtr<Vertex> v_[4];
    Vector3 circumscibedSphereCenter_;
};

} // namespace internal

// ----------------------------------------------------------------------------
static internal::Tetrahedron* ConstructSuperTetrahedron(const Vector<Vector3>& vertexList)
{
    // Compute bounding box
    BoundingBox aabb;
    Vector<Vector3>::ConstIterator it = vertexList.Begin();
    for(; it != vertexList.End(); ++it)
    {
        if(aabb.min_.x_ > it->x_) aabb.min_.x_ = it->x_;
        if(aabb.min_.y_ > it->y_) aabb.min_.y_ = it->y_;
        if(aabb.min_.z_ > it->z_) aabb.min_.z_ = it->z_;
        if(aabb.max_.x_ < it->x_) aabb.max_.x_ = it->x_;
        if(aabb.max_.y_ < it->y_) aabb.max_.y_ = it->y_;
        if(aabb.max_.z_ < it->z_) aabb.max_.z_ = it->z_;
    }

    // This tetrahedron should encompass all vertices in the list
    return new internal::Tetrahedron(
        /*
        new internal::Vertex(aabb.max_ * 100.1f),
        new internal::Vertex(Vector3(aabb.max_.x_ - aabb.min_.x_, 0, 0) * 200 * 1.1f + aabb.min_),
        new internal::Vertex(Vector3(0, aabb.max_.y_ - aabb.min_.y_, 0) * 200 * 1.1f + aabb.min_),
        new internal::Vertex(Vector3(0, 0, aabb.max_.z_ - aabb.min_.z_) * 200 * 1.1f + aabb.min_)*/
        new internal::Vertex(Vector3(0, 50, 0)),
        new internal::Vertex(Vector3(0, -50, 50)),
        new internal::Vertex(Vector3(-50, -50, -50)),
        new internal::Vertex(Vector3(50, -50, -50))
    );
}

// ----------------------------------------------------------------------------
static Vector<SharedPtr<internal::Tetrahedron> >::Iterator
FindTetrahedronContaining(Vector<SharedPtr<internal::Tetrahedron> >& tetrahedrons,
                          const Vector3& point)
{
    Vector<SharedPtr<internal::Tetrahedron> >::Iterator it = tetrahedrons.Begin();
    for(; it != tetrahedrons.End(); ++it)
    {
        internal::Tetrahedron* t = *it;
        if(Urho3D::Tetrahedron(t->v_[0]->position_,
                               t->v_[1]->position_,
                               t->v_[2]->position_,
                               t->v_[3]->position_).PointLiesInside(point))
            return it;
    }

    return tetrahedrons.End();
}

// ----------------------------------------------------------------------------
TetrahedralMesh::TetrahedralMesh(const Vector<Vector3>& vertexList)
{
    Construct(vertexList);
}

// ----------------------------------------------------------------------------
void TetrahedralMesh::Construct(const Vector<Vector3>& vertexList)
{
    // Create triangulation list and add super tetrahedron to it.
    Vector<SharedPtr<internal::Tetrahedron> > triangulation;
    SharedPtr<internal::Tetrahedron> superTetrahedron(ConstructSuperTetrahedron(vertexList));
    triangulation.Push(superTetrahedron);

    Vector<SharedPtr<internal::Tetrahedron>> badTetrahedrons;
    Vector<internal::Vertex*> polyhedron;

    Vector<Vector3>::ConstIterator vertIt = vertexList.Begin();
    for(; vertIt != vertexList.End(); ++vertIt)
    {
        badTetrahedrons.Clear();
        polyhedron.Clear();

        // First find all tetrahedrons that are no longer valid due to the insertion
        Vector<SharedPtr<internal::Tetrahedron> >::Iterator tetrahedron = triangulation.Begin();
        for(; tetrahedron != triangulation.End(); ++tetrahedron)
        {
            internal::Tetrahedron* t = *tetrahedron;

            Vector3 circumsphereCenter = Math::CircumscribeSphere(
                t->v_[0]->position_,
                t->v_[1]->position_,
                t->v_[2]->position_,
                t->v_[3]->position_
            );
            float radiusSquared = (circumsphereCenter - t->v_[0]->position_).LengthSquared();

            // Test if point is inside circumcircle of tetrahedron. Add to bad list
            if((*vertIt - circumsphereCenter).LengthSquared() < radiusSquared)
                badTetrahedrons.Push(SharedPtr<internal::Tetrahedron>(t));

            /* Tests if a point is inside a tetrahedron
            if(Urho3D::Tetrahedron(t->v_[0]->position_,
                                   t->v_[1]->position_,
                                   t->v_[2]->position_,
                                   t->v_[3]->position_).PointLiesInside(*vertIt))
                badTetrahedrons.Push(SharedPtr<internal::Tetrahedron>(t));*/
        }

        // Find the boundary of the hole
        tetrahedron = badTetrahedrons.Begin();
        for(; tetrahedron != badTetrahedrons.End(); ++tetrahedron)
        {
            internal::Tetrahedron* t = *tetrahedron;

            // Stores all vertices that are shared with other tetrahedrons in the bad list
            internal::Vertex* sharedVertices[4];
            internal::Vertex** sharedVerticesPtr = sharedVertices;
            Vector<SharedPtr<internal::Tetrahedron> >::Iterator other = tetrahedron + 1;
            for(; other != badTetrahedrons.End(); ++other)
            {
                for(int i = 0; i != 4; ++i)
                    for(int j = 0; j != 4; ++j)
                        if(t->v_[i] == (*other)->v_[j])
                        {
                            for(int k = 0; k != 4; ++k)
                                if(sharedVertices[k] == t->v_[i])
                                    goto break_already_marked_as_shared;
                            *sharedVerticesPtr++ = t->v_[i].Get();
                            break_already_marked_as_shared: continue;
                        }
            }

            unsigned numSharedVertices = sharedVerticesPtr - sharedVertices;
            assert(numSharedVertices <= 4);
            if(numSharedVertices < 3) // No shared faces
            {
                // Add all four faces of the tetrahedron
                polyhedron.Push(t->v_[0]);
                polyhedron.Push(t->v_[1]);
                polyhedron.Push(t->v_[2]);

                polyhedron.Push(t->v_[0]);
                polyhedron.Push(t->v_[1]);
                polyhedron.Push(t->v_[3]);

                polyhedron.Push(t->v_[1]);
                polyhedron.Push(t->v_[2]);
                polyhedron.Push(t->v_[3]);

                polyhedron.Push(t->v_[2]);
                polyhedron.Push(t->v_[0]);
                polyhedron.Push(t->v_[3]);
            }
            else if(numSharedVertices == 3) // One face is shared
            {
                // Find the vertex that is not part of the face
                internal::Vertex* vertex = NULL;
                for(int i = 0; i != 4; ++i)
                {
                    for(int j = 0; j != numSharedVertices; ++j)
                        if(t->v_[i] == sharedVertices[j])
                            goto break_vertex_is_shared;
                    // Found the vertex
                    vertex = t->v_[i];
                    break;

                    break_vertex_is_shared: continue;
                }

                assert(vertex);

                // Add the three faces
                polyhedron.Push(sharedVertices[0]);
                polyhedron.Push(sharedVertices[1]);
                polyhedron.Push(vertex);

                polyhedron.Push(sharedVertices[1]);
                polyhedron.Push(sharedVertices[2]);
                polyhedron.Push(vertex);

                polyhedron.Push(sharedVertices[2]);
                polyhedron.Push(sharedVertices[0]);
                polyhedron.Push(vertex);
            }
            else
            {
                // All faces are shared, don't add anything
            }
        }

        // Remove bad tetrahedrons from triangulation
        tetrahedron = badTetrahedrons.Begin();
        for(; tetrahedron != badTetrahedrons.End(); ++tetrahedron)
        {
            triangulation.Remove(*tetrahedron);
        }

        // Re-triangulate the hole
        Vector<internal::Vertex*>::Iterator vertex = polyhedron.Begin();
        SharedPtr<internal::Vertex> connectToVertex(new internal::Vertex(*vertIt));
        while(vertex != polyhedron.End())
        {
            internal::Vertex* v1 = *vertex++;
            internal::Vertex* v2 = *vertex++;
            internal::Vertex* v3 = *vertex++;
            triangulation.Push(SharedPtr<internal::Tetrahedron>(
                new internal::Tetrahedron(connectToVertex, v1, v2, v3)
            ));
        }
    }

    Vector<SharedPtr<internal::Tetrahedron> >::ConstIterator tetIt = triangulation.Begin();
    for(; tetIt != triangulation.End(); ++tetIt)
    {
        internal::Tetrahedron* t = *tetIt;
        for(int i = 0; i != 4; ++i)
            for(int j = 0; j != 4; ++j)
                if(t->v_[i] == superTetrahedron->v_[j])
                    goto break_skip;

        tetrahedrons_.Push(Tetrahedron(
            t->v_[0]->position_,
            t->v_[1]->position_,
            t->v_[2]->position_,
            t->v_[3]->position_
        ));

        break_skip: continue;
    }
/*
    tetrahedrons_.Clear();
    tetrahedrons_.Push(Tetrahedron(
        Vector3(-5, 0, 0),Vector3(0, -5, 5),Vector3(5, 0, 0),Vector3(0, 5, 5)
    ));*/
}

// ----------------------------------------------------------------------------
void TetrahedralMesh::DrawDebugGeometry(DebugRenderer* debug, bool depthTest, Vector3 pos)
{
    Vector<Tetrahedron>::Iterator it = tetrahedrons_.Begin();
    for(; it != tetrahedrons_.End(); ++it)
        if(it->PointLiesInside(pos))
            it->DrawDebugGeometry(debug, false, Color::RED);
        else
            it->DrawDebugGeometry(debug, depthTest, Color::WHITE);
}

} // namespace Urho3D
