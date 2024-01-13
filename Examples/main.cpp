#include <memory>
#include <windows.h>

#include "ExampleApp.h"
#include "Example_TEMPLATE.h"

#include "Ex1401_Basic.h"
#include "Ex1402_Blur.h"
#include "Ex1403_MatVecMult.h"
#include "Ex1404_StructuredBuffer.h"
#include "Ex1405_ConsumeAppendBuffer.h"
#include "Ex1406_DensityField.h"
#include "Ex1407_IndirectArguments.h"
#include "Ex1408_BitonicSort.h"
#include "Ex1501_ParticleSystem.h"
#include "Ex1502_SpriteFireEffect.h"
#include "Ex1503_SphWater.h"
#include "Ex1601_StableFluids.h"
#include "Ex1602_CurlNoise.h"
#include "Ex1603_Cloud.h"
#include "Ex1604_RealtimeSmoke.h"
#include "Ex1605_SmokeCpu.h"
#include "Ex1606_HybridWater.h"
#include "Ex1701_SkeletalAnimation.h"
#include "Ex1801_Tree.h"
#include "Ex1802_Grass.h"
#include "Ex1803_Landscape.h"
#include "Ex1901_PhysX.h"
#include "Ex2001_GamePlay.h"
#include "DaerimGTA.h"

using namespace std;

int main(int argc, char *argv[]) {

    std::unique_ptr<hlab::AppBase> app;

    if (argc < 2) {
        cout << "Please specify the example number" << endl;
        return -1;
    }

    switch (atoi(argv[1])) {
    case 0:
        app = make_unique<hlab::ExampleApp>();
        break;
    case 1:
        app = make_unique<hlab::Example_TEMPLATE>();
        break;
    case 1401:
        app = make_unique<hlab::Ex1401_Basic>();
        break;
    case 1402:
        app = make_unique<hlab::Ex1402_Blur>();
        break;
    case 1403:
        app = make_unique<hlab::Ex1403_MatVecMult>();
        break;
    case 1404:
        app = make_unique<hlab::Ex1404_StructuredBuffer>();
        break;
    case 1405:
        app = make_unique<hlab::Ex1405_ConsumeAppendBuffer>();
        break;
    case 1406:
        app = make_unique<hlab::Ex1406_DensityField>();
        break;
    case 1407:
        app = make_unique<hlab::Ex1407_IndirectArguments>();
        break;
    case 1408:
        app = make_unique<hlab::Ex1408_BitonicSort>();
        break;
    case 1501:
        app = make_unique<hlab::Ex1501_ParticleSystem>();
        break;
    case 1502:
        app = make_unique<hlab::Ex1502_SpriteFireEffect>();
        break;
    case 1503:
        app = make_unique<hlab::Ex1503_SphWater>();
        break;
    case 1601:
        app = make_unique<hlab::Ex1601_StableFluids>();
        break;
    case 1602:
        app = make_unique<hlab::Ex1602_CurlNoise>();
        break;
    case 1603:
        app = make_unique<hlab::Ex1603_Cloud>();
        break;
    case 1604:
        app = make_unique<hlab::Ex1604_RealtimeSmoke>();
        break;
    case 1605:
        app = make_unique<hlab::Ex1605_SmokeCpu>();
        break;
    case 1606:
        app = make_unique<hlab::Ex1606_HybridWater>();
        break;
    case 1701:
        app = make_unique<hlab::Ex1701_SkeletalAnimation>();
        break;
    case 1801:
        app = make_unique<hlab::Ex1801_Tree>();
        break;
    case 1802:
        app = make_unique<hlab::Ex1802_Grass>();
        break;
    case 1803:
        app = make_unique<hlab::Ex1803_Landscape>();
        break;
    case 1901:
        app = make_unique<hlab::Ex1901_PhysX>();
        break;
    case 2001:
        app = make_unique<hlab::Ex2001_GamePlay>();
        break;
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
