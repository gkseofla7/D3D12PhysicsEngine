#include <memory>
#include <windows.h>

#include "ExampleApp.h"
#include "Example_TEMPLATE.h"
#include "DaerimGTA.h"

using namespace std;

int main(int argc, char *argv[]) {

    std::unique_ptr<hlab::AppBase> app;

    if (argc < 2) {
        cout << "Please specify the example number" << endl;
        return -1;
    }

    switch (atoi(argv[1])) {
    case 9999:
        app = make_unique<hlab::DaerimGTA>();
        break;
    default:
        cout << argv[1] << " is not a valid example number" << endl;
    }

    if (!app->Initialize()) {
        cout << "Initialization failed." << endl;
        return -1;
    }

    return app->Run();
}
