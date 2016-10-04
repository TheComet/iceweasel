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
    Args args;
    for(int i = 0; i != argc; ++i)
    {
        if(strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--editor") == 0)
            args.editor_ = true;

        if(strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--map") == 0)
            if(i + 1 < argc)
                args.mapName_ = argv[i+1];
    }

    SharedPtr<Context> context(new Context);
    SharedPtr<Application> app;
    if(args.editor_)
        app = new InGameEditorApplication(context);
    else
        app = new IceWeasel(context, args.mapName_);

    return app->Run();
}
