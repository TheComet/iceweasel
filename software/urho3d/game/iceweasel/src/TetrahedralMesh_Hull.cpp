#include "iceweasel/TetrahedralMesh_Hull.h"
#include "iceweasel/TetrahedralMesh_Polyhedron.h"
#include "iceweasel/TetrahedralMesh_Vertex.h"
#include "iceweasel/TetrahedralMesh_Edge.h"
#include "iceweasel/TetrahedralMesh_Face.h"

#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Math/Vector3.h>

using namespace TetrahedralMesh;

// ----------------------------------------------------------------------------
Hull::Hull()
{
}

// ----------------------------------------------------------------------------
Hull::Hull(Polyhedron* polyhedron)
{
    SetMesh(polyhedron);
}

// ----------------------------------------------------------------------------
void Hull::SetMesh(Polyhedron* polyhedron)
{
    faces_.Clear();
    edges_.Clear();
    hullMesh_ = polyhedron;

    if(hullMesh_->FaceCount() == 0)
        return;

    Urho3D::Vector<Urho3D::Vector3> trianglePositions;

    Polyhedron::ConstIterator vertexIt = hullMesh_->Begin();
    while(vertexIt != hullMesh_->End())
    {
        Vertex* v0 = *vertexIt++;
        Vertex* v1 = *vertexIt++;
        Vertex* v2 = *vertexIt++;

        // Accumulate positions of triangle centres for average position
        trianglePositions.Push(
            (v0->position_ + v1->position_ + v2->position_) / 3.0f
        );

        // Add face
        faces_.Push(Face(v0, v1, v2));
    }

    // Average triangle centres to get centre of hull
    centre_ = Urho3D::Vector3::ZERO;
    for(Urho3D::Vector<Urho3D::Vector3>::ConstIterator it = trianglePositions.Begin();
        it != trianglePositions.End();
        ++it)
    {
        centre_ += *it;
    }
    centre_ /= trianglePositions.Size();

    // Make sure each triangle's normal vector is pointing away from centre of
    // the hull
    for(Urho3D::Vector<Face>::Iterator it = faces_.Begin();
        it != faces_.End();
        ++it)
    {
        Urho3D::Vector3 outwards = it->GetVertex(0)->position_ - centre_;
        if(outwards.DotProduct(it->GetNormal()) < 0)
            it->FlipNormal();
    }

    // With the centre and normals calculated, we can use that information to
    // construct edges. Each edge stores the normal vectors of both triangles
    // it joins in order to calculate if a projected point is projected from
    // the correct angle or not.
    //
    // We iterate the polyhedron and the triangles list simultaneously, because
    // it is more efficient to check for joined edges using the polyhedron data
    // structure than the triangles list. The triangles list is required to get
    // the calculated and adjusted normals.
    vertexIt = hullMesh_->Begin();
    Urho3D::Vector<Face>::ConstIterator triangleIt = faces_.Begin();
    for(; vertexIt != hullMesh_->End(); triangleIt++)
    {
        Vertex* vertex[3];
        vertex[0] = *vertexIt++;
        vertex[1] = *vertexIt++;
        vertex[2] = *vertexIt++;

        // find the three adjacent triangles
        Urho3D::Vector<Face>::ConstIterator triangleIt2 = faces_.Begin();
        Polyhedron::ConstIterator vertexIt2 = hullMesh_->Begin();
        for(; vertexIt2 != hullMesh_->End(); triangleIt2++)
        {
            Vertex* vertex2[3];
            vertex2[0] = *vertexIt2++;
            vertex2[1] = *vertexIt2++;
            vertex2[2] = *vertexIt2++;

            // skip self
            if(vertexIt == vertexIt2)
                continue;

            // joined[2] will contain two Vertex objects if we find an edge
            // that is shared between the two current triangles.
            Vertex* joined[2];
            Vertex** joinedPtr = joined;
            for(unsigned char i = 0; i != 3; ++i)
                for(unsigned char j = 0; j != 3; ++j)
                    if(vertex[i] == vertex2[j])
                        *joinedPtr++ = vertex[i];

            // Calculate number of vertices that were found joined.
            unsigned joinedCount = joinedPtr - joined;
            assert(joinedCount != 3);
            if(joinedCount != 2)
                continue;

            // Found a joined edge, add it
            edges_.Push(Edge(
                joined[0],
                joined[1],
                triangleIt->GetNormal(),
                triangleIt2->GetNormal()
            ));

            // Make sure edge boundary check points outwards from the hull's
            // centre
            Urho3D::Vector2 bary = edges_.Back().ProjectAndTransformToBarycentric(centre_);
            if(edges_.Back().ProjectionAngleIsInBounds(edges_.Back().TransformToCartesian(bary), centre_))
                edges_.Back().FlipBoundaryCheck();
        }
    }
}

// ----------------------------------------------------------------------------
bool Hull::Query(Urho3D::Vector3* gravity, const Urho3D::Vector3& position)
{
    using namespace Urho3D;

    /* NOTE This method works, but was removed because the results were not
     * good enough.
    Vector3 distanceVec = centre_ - position;
    Vector3 direction = distanceVec.Normalized();
    float distanceSquared = distanceVec.LengthSquared();

    // Use a linear search for now. Can optimise later
    Vector<GravityTriangle>::ConstIterator triangle = triangles_.Begin();
    for(; triangle != triangles_.End(); ++triangle)
    {
        Vector3 bary = triangle->Intersect(position, direction);
        if(triangle->PointLiesInside(bary))
        {
            // It's possible we hit a triangle on the other side.
            Vector3 intersectionLocation = triangle->TransformToCartesian(bary);
            if((position - intersectionLocation).LengthSquared() > distanceSquared)
                continue;

            // We found the triangle, return it
            if(barycentric != NULL)
                *barycentric = bary;
            return &(*triangle);
        }
    }*/

    Vector3 distanceVec =  position - centre_;

    // Try all faces first
    for(Vector<Face>::ConstIterator face = faces_.Begin();
        face != faces_.End();
        ++face)
    {
        Vector3 bary = face->ProjectAndTransformToBarycentric(position);
        if(face->PointLiesInside(bary))
        {
            // It's possible we hit a triangle on the other side
            if(distanceVec.DotProduct(face->GetNormal()) < 0)
                continue;

            // We found the triangle, interpolate gravity vector and return
            if(gravity != NULL)
                *gravity = face->InterpolateGravity(bary);
            lastIntersection_ = face->TransformToCartesian(bary);
            return true;
        }
    }

    // Try all edges
    for(Vector<Edge>::ConstIterator edge = edges_.Begin();
        edge != edges_.End();
        ++edge)
    {
        Vector2 bary = edge->ProjectAndTransformToBarycentric(position);
        if(edge->PointLiesInside(bary))
        {
            // It's possible we're not projecting from the correct angle
            if(!edge->ProjectionAngleIsInBounds(edge->TransformToCartesian(bary), position))
                continue;

            // Found the edge, interpolate gravity vector and return
            if(gravity != NULL)
                *gravity = edge->InterpolateGravity(bary);
            lastIntersection_ = edge->TransformToCartesian(bary);
            return true;
        }
    }

    // Find closest vertex as a last resort
    float distanceSquared = M_INFINITY;
    const Vertex* foundPoint = NULL;
    for(Polyhedron::ConstIterator pVertex = hullMesh_->Begin();
        pVertex != hullMesh_->End();
        ++pVertex)
    {
        Vertex* vertex = *pVertex;
        float newDist = (vertex->position_ - position).LengthSquared();
        if(newDist < distanceSquared)
        {
            distanceSquared = newDist;
            foundPoint = &(*vertex);
        }
    }
    if(foundPoint != NULL)
    {
        if(gravity != NULL)
            *gravity = foundPoint->direction_ * foundPoint->forceFactor_;
        lastIntersection_ = foundPoint->position_;
        return true;
    }

    return false;
}

// ----------------------------------------------------------------------------
void Hull::DrawDebugGeometry(Urho3D::DebugRenderer* debug, bool depthTest, Urho3D::Vector3 pos) const
{
    for(Urho3D::Vector<Face>::ConstIterator it = faces_.Begin();
        it != faces_.End();
        ++it)
    {
        it->DrawDebugGeometry(debug, depthTest, Urho3D::Color::WHITE);
    }

    for(Urho3D::Vector<Edge>::ConstIterator it = edges_.Begin();
        it != edges_.End();
        ++it)
    {
        it->DrawDebugGeometry(debug, depthTest, Urho3D::Color::WHITE);
    }

    debug->AddSphere(Urho3D::Sphere(lastIntersection_, 1.0f), Urho3D::Color::RED, depthTest);
}
