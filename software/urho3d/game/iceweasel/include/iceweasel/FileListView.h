#pragma once

#include <Urho3D/UI/ListView.h>

class FileListView : public Urho3D::ListView
{
public:
    FileListView(Urho3D::Context* context);
    static void RegisterObject(Urho3D::Context* context);
    
};
