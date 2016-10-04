#include "iceweasel/IceWeasel.h"
#include "iceweasel/InGameEditorApplication.h"

using namespace Urho3D;

struct Args
{
    Args() :
        editor_(false)
    {}

    String mapName_;
    bool editor_;
};

int main(int argc, char** argv)
{
    bool editor_ = false;
    StringVector args;
    for(int i = 0; i != argc; ++i)
    {
        if(strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--editor") == 0)
            editor_ = true;

        args.Push(argv[i]);
    }

    SharedPtr<Context> context(new Context);
    SharedPtr<Application> app;
    if(editor_)
        app = new InGameEditorApplication(context);
    else
        app = new IceWeasel(context, args);

    return app->Run();
}
