#include "iceweasel/TetrahedralMeshBuilder.h"
#include "iceweasel/TetrahedralMesh_Polyhedron.h"
#include "iceweasel/Math.h"
#include "iceweasel/GravityVector.h"

#include <Urho3D/Math/BoundingBox.h>

using namespace Urho3D;
using namespace TetrahedralMesh;

// ============================================================================
TetrahedralMeshBuilder::
CircumscribedTetrahedron::CircumscribedTetrahedron(TetrahedralMesh::Vertex* v1, TetrahedralMesh::Vertex* v2,
                                                   TetrahedralMesh::Vertex* v3, TetrahedralMesh::Vertex* v4)
{
    v_[0] = v1; v_[1] = v2; v_[2] = v3; v_[3] = v4;
    circumscibedSphereCenter_ = Math::CircumscribeSphere(v1->position_,
                                                         v2->position_,
                                                         v3->position_,
                                                         v4->position_);
}


// ============================================================================
struct Face
{
    Face() {}
    Face(TetrahedralMesh::Vertex* v1,
         TetrahedralMesh::Vertex* v2,
         TetrahedralMesh::Vertex* v3) :
        marked_(true)
    {
        v_[0] = v1;
        v_[1] = v2;
        v_[2] = v3;
    }

    bool FaceIsShared(const Face* other)
    {
        unsigned count = 0;
        for(unsigned i = 0; i != 3; ++i)
            for(unsigned j = 0; j != 3; ++j)
                if(other->v_[i] == v_[j])
                    if(++count == 3)
                        return true;
        return false;
    }

    SharedPtr<TetrahedralMesh::Vertex> v_[3];
    bool marked_;
};


// ============================================================================
static SharedPtr<TetrahedralMeshBuilder::CircumscribedTetrahedron>
ConstructSuperTetrahedron(const PODVector<GravityVector*>& gravityVectors)
{
    // Compute bounding box of all of the gravity vector components
    BoundingBox aabb;
    for(PODVector<GravityVector*>::ConstIterator it = gravityVectors.Begin();
        it != gravityVectors.End();
        ++it)
    {
        const Vector3& pos = (*it)->GetPosition();
        if(aabb.min_.x_ > pos.x_) aabb.min_.x_ = pos.x_;
        if(aabb.min_.y_ > pos.y_) aabb.min_.y_ = pos.y_;
        if(aabb.min_.z_ > pos.z_) aabb.min_.z_ = pos.z_;
        if(aabb.max_.x_ < pos.x_) aabb.max_.x_ = pos.x_;
        if(aabb.max_.y_ < pos.y_) aabb.max_.y_ = pos.y_;
        if(aabb.max_.z_ < pos.z_) aabb.max_.z_ = pos.z_;
    }

    // Expand bounding box by factor 3 plus a small error margin
    aabb.min_.x_ -= (aabb.max_.x_ - aabb.min_.x_) * 2.2f;
    aabb.min_.y_ -= (aabb.max_.y_ - aabb.min_.y_) * 2.2f;
    aabb.min_.z_ -= (aabb.max_.z_ - aabb.min_.z_) * 2.2f;
    aabb.max_.x_ += Abs(aabb.max_.x_ * 0.1f);
    aabb.max_.y_ += Abs(aabb.max_.y_ * 0.1f);
    aabb.max_.z_ += Abs(aabb.max_.z_ * 0.1f);

    // This tetrahedron should encompass all vertices in the list
    return SharedPtr<TetrahedralMeshBuilder::CircumscribedTetrahedron>(new TetrahedralMeshBuilder::CircumscribedTetrahedron(
        new Vertex(aabb.max_, Vector3::DOWN),
        new Vertex(Vector3(aabb.min_.x_, aabb.max_.y_, aabb.max_.z_), Vector3::DOWN),
        new Vertex(Vector3(aabb.max_.x_, aabb.min_.y_, aabb.max_.z_), Vector3::DOWN),
        new Vertex(Vector3(aabb.max_.x_, aabb.max_.y_, aabb.min_.z_), Vector3::DOWN)
    ));
}

// ----------------------------------------------------------------------------
void TetrahedralMeshBuilder::Build(const PODVector<GravityVector*>& gravityVectors)
{
    /*
     * The Bowyer-Watson algorithm is used here to convert a set of 3D points
     * into a mesh of non-overlapping tetrahedrons.
     * https://en.wikipedia.org/wiki/Bowyer%E2%80%93Watson_algorithm
     */

    triangulationResult_.Clear();
    hull_ = new TetrahedralMesh::Polyhedron;

    CircumscribedTetrahedralMesh badTetrahedrons;
    TetrahedralMesh::Polyhedron polyhedron;

    // Add super tetrahedron as the first tetrahedron to the list.
    SharedPtr<CircumscribedTetrahedron> superTetrahedron = ConstructSuperTetrahedron(gravityVectors);
    triangulationResult_.Push(superTetrahedron);

    // Iterate over all gravity vectors, add each as a vertex to the mesh one by one
    for(PODVector<GravityVector*>::ConstIterator gravityVectorsIt = gravityVectors.Begin();
        gravityVectorsIt != gravityVectors.End();
        ++gravityVectorsIt)
    {
        GravityVector* gravityVector = *gravityVectorsIt;

        FindBadTetrahedrons(&badTetrahedrons, gravityVector->GetPosition());
        CreateHullFromTetrahedrons(&polyhedron, badTetrahedrons);
        RemoveTetrahedronsFromTriangulation(badTetrahedrons);
        ReTriangulateGap(polyhedron, *gravityVector);
    }

    CleanUp(superTetrahedron);

    // The mesh is built. We can extract the hull by marking all tetrahedrons
    // in the mesh as "bad" and running it through the face-face comparison
    // code. This will return a list of all triangles that don't touch each
    // other, i.e. the hull.
    CreateHullFromTetrahedrons(hull_, triangulationResult_);
}

// ----------------------------------------------------------------------------
const TetrahedralMeshBuilder::CircumscribedTetrahedralMesh&
TetrahedralMeshBuilder::GetTetrahedralMesh() const
{
    return triangulationResult_;
}

// ----------------------------------------------------------------------------
TetrahedralMesh::Polyhedron* TetrahedralMeshBuilder::GetHullMesh() const
{
    return hull_;
}

// ----------------------------------------------------------------------------
void TetrahedralMeshBuilder::FindBadTetrahedrons(
    CircumscribedTetrahedralMesh* badTetrahedrons,
    Vector3 point) const
{
    badTetrahedrons->Clear();

    // Iterate all tetrahedrons in current triangulation and calculate their
    // circumsphere. If the vertex location (point) we are adding is within the
    // sphere, then we add that tetrahedron to the bad list.
    for(CircumscribedTetrahedralMesh::ConstIterator pTetrahedron = triangulationResult_.Begin();
        pTetrahedron != triangulationResult_.End();
        ++pTetrahedron)
    {
        CircumscribedTetrahedron* tetrahedron = *pTetrahedron;
        Vector3 circumsphereCenter = Math::CircumscribeSphere(
            tetrahedron->v_[0]->position_,
            tetrahedron->v_[1]->position_,
            tetrahedron->v_[2]->position_,
            tetrahedron->v_[3]->position_
        );

        // Test if point is inside circumcircle of tetrahedron. Add to bad list
        float radiusSquared = (circumsphereCenter - tetrahedron->v_[0]->position_).LengthSquared();
        if((point - circumsphereCenter).LengthSquared() < radiusSquared)
            badTetrahedrons->Push(SharedPtr<CircumscribedTetrahedron>(tetrahedron));
    }
}

// ----------------------------------------------------------------------------
void TetrahedralMeshBuilder::CreateHullFromTetrahedrons(
    TetrahedralMesh::Polyhedron* polyhedron,
    const CircumscribedTetrahedralMesh& tetrahedrons)
{
    polyhedron->Clear();

    // Create a list of faces from the bad tetrahedrons. Note that by default
    // all faces are marked initially.
    unsigned numFaces = tetrahedrons.Size() * 4;
    Face* face = new Face[numFaces];
    CircumscribedTetrahedralMesh::ConstIterator tetrahedron = tetrahedrons.Begin();
    for(unsigned i = 0; tetrahedron != tetrahedrons.End(); ++tetrahedron, i += 4)
    {
        CircumscribedTetrahedron* t = *tetrahedron;
        face[i+0] = Face(t->v_[0], t->v_[1], t->v_[2]);
        face[i+1] = Face(t->v_[3], t->v_[0], t->v_[1]);
        face[i+2] = Face(t->v_[3], t->v_[1], t->v_[2]);
        face[i+3] = Face(t->v_[3], t->v_[2], t->v_[0]);
    }

    // Check each face against all of the other faces, see if they are
    // connected. If they are, unmark them.
    for(unsigned i = 0; i != numFaces; ++i)
        for(unsigned j = i+1; j != numFaces; ++j)
            if(face[i].FaceIsShared(&face[j]))
            {
                face[i].marked_ = false;
                face[j].marked_ = false;
            }

    // The remaining marked faces are the hull of the bad tetrahedrons. Add
    // them to the polyhedron so they can be connected with the new vertex.
    for(unsigned i = 0; i != numFaces; ++i)
        if(face[i].marked_)
        {
            polyhedron->Push(face[i].v_[0]);
            polyhedron->Push(face[i].v_[1]);
            polyhedron->Push(face[i].v_[2]);
        }

    delete[] face;
}

// ----------------------------------------------------------------------------
void TetrahedralMeshBuilder::RemoveTetrahedronsFromTriangulation(
    const CircumscribedTetrahedralMesh& tetrahedrons)
{
    for(CircumscribedTetrahedralMesh::ConstIterator tetrahedronIt = tetrahedrons.Begin();
        tetrahedronIt != tetrahedrons.End();
        ++tetrahedronIt)
    {
        triangulationResult_.Remove(*tetrahedronIt);
    }
}

// ----------------------------------------------------------------------------
void TetrahedralMeshBuilder::ReTriangulateGap(const TetrahedralMesh::Polyhedron& polyhedron,
                                          const GravityVector& gravityVector)
{

    // Create an internal Vertex object from the gravity vector component.
    SharedPtr<Vertex> connectToVertex(new Vertex(
        gravityVector.GetPosition(),
        gravityVector.GetDirection(),
        gravityVector.GetForceFactor()
    ));

    // Connect all faces in the polyhedron to the new vertex to form new
    // tetrahedrons.
    TetrahedralMesh::Polyhedron::ConstIterator vertex = polyhedron.Begin();
    while(vertex != polyhedron.End())
    {
        Vertex* v1 = *vertex++;
        Vertex* v2 = *vertex++;
        Vertex* v3 = *vertex++;
        triangulationResult_.Push(SharedPtr<CircumscribedTetrahedron>(
            new CircumscribedTetrahedron(connectToVertex, v1, v2, v3)
        ));
    }
}

// ----------------------------------------------------------------------------
void TetrahedralMeshBuilder::CleanUp(const CircumscribedTetrahedron* superTetrahedron)
{
    CircumscribedTetrahedralMesh::Iterator tetrahedron = triangulationResult_.Begin();
    while(tetrahedron != triangulationResult_.End())
    {
        CircumscribedTetrahedron* t = *tetrahedron;

        for(int i = 0; i != 4; ++i)
            for(int j = 0; j != 4; ++j)
                if(t->v_[i] == superTetrahedron->v_[j])
                {
                    tetrahedron = triangulationResult_.Erase(tetrahedron);
                    goto break_dont_increment_iterator;
                }

        ++tetrahedron;
        break_dont_increment_iterator: continue;
    }
}
