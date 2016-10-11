#include "iceweasel/IceWeasel.h"
#include "iceweasel/Args.h"
#include "iceweasel/InGameEditorApplication.h"

#include <stdio.h>

using namespace Urho3D;

void printHelp(const char* prog_name)
{
    printf("Usage: %s [options]", prog_name);
    printf("  -h, --help                           = Show this help");
    printf("  -r, --resource <Path/To/Resource>    = Add additional paths to resources");
    printf("  -e, --editor                         = Run the editor instead of starting the game");
    printf("  -s, --scene <Scenes/Name.xml>        = Load the specified scene instead of loading the default one");
    printf("  -w, --window                         = Run in windowed mode instead of fullscreen");
    printf("  -v, --vsync                          = Turn on VSync");
    printf("  -m, --multisample <integer>          = Specify multisample value");
    printf("      --server                         = Run in server mode. Clients can connect.");
}

int main(int argc, char** argv)
{
    Urho3D::SharedPtr<Args> args(new Args);
    for(int i = 0; i != argc; ++i)
    {
        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            printHelp(argv[0]);
            return 0;
        }

        if(strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--resuorce") == 0)
            if(i + 1 < argc)
                args->resourcePaths_.Push(argv[i + 1]);
        if(strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--editor") == 0)
            args->editor_ = true;
        if(strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--scene") == 0)
            if(i + 1 < argc)
                args->sceneName_ = argv[i + 1];
        if(strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--window") == 0)
            args->windowed_ = true;
        if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--vsync") == 0)
            args->vsync_ = true;
        if(strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--multisample") == 0)
            if(i + 1 < argc)
                args->multisample_ = atoi(argv[i + 1]);
        if(strcmp(argv[i], "--server") == 0)
            args->headless_ = true;
    }

    SharedPtr<Context> context(new Context);
    SharedPtr<Application> app;
    if(args->editor_)
        app = new InGameEditorApplication(context);
    else
        app = new IceWeasel(context, args);

    return app->Run();
}
