#include "iceweasel/FileListView.h"
#include "iceweasel/IceWeasel.h"

// ----------------------------------------------------------------------------
FileListView::FileListView(Urho3D::Context* context) :
    ListView(context)
{
}

// ----------------------------------------------------------------------------
void FileListView::RegisterObject(Urho3D::Context* context)
{
    context->RegisterFactory<FileListView>(ICEWEASEL_CATEGORY);
}
