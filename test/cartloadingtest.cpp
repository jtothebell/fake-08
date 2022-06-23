#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <stdio.h>
#include <dirent.h>
#include <errno.h>

#include "doctest.h"
#include "../libs/lodepng/lodepng.h"

#include "../source/vm.h"
#include "../source/host.h"
#include "../source/hostVmShared.h"
#include "../source/nibblehelpers.h"
#include "../source/filehelpers.h"


const vector<string> cartsToIgnore ({
    "15666.p8.png",
    "16069.p8.png",
    "16355.p8.png",
    "16507.p8.png",
    "16677.p8.png",
    "16965.p8.png",
    "17616.p8.png",
    "17853.p8.png",
    "17888.p8.png",
    "18124.p8.png",
    "18199.p8.png",
    "18560.p8.png",
    "18813.p8.png",
    "18987.p8.png",
    "20797.p8.png",
    "21114.p8.png", // intermittent
    "21242.p8.png",
    "21415.p8.png",
    "21677.p8.png",
    "21736.p8.png",
    "23013.p8.png",
    "23183.p8.png",
    "23402.p8.png",
    "23801.p8.png",
    "23842.p8.png",
    "23958.p8.png",
    "24007.p8.png",
    "24141.p8.png",
    "24292.p8.png",
    "24418.p8.png",
    "24443.p8.png",
    "24687.p8.png",
    "24695.p8.png",
    "25532.p8.png",
    "25735.p8.png",
    "25919.p8.png",
    "26194.p8.png",
    "26425.p8.png",
    "26646.p8.png", //different logs every time
    "26782.p8.png",
    "27183.p8.png",
    "27672.p8.png",
    "27941.p8.png",
    "28607.p8.png",
    "28611.p8.png",
    "28725.p8.png",
    "28757.p8.png",
    "28912.p8.png",
    "29142.p8.png",
    "29216.p8.png",
    "29283.p8.png",
    "29420.p8.png",
    "29559.p8.png",
    "29562.p8.png",
    "29844.p8.png",
    "29982.p8.png",
    "30091.p8.png",
    "30427.p8.png",
    "30617.p8.png",
    "30625.p8.png",
    "30785.p8.png",
    "30819.p8.png",
    "30960.p8.png",
    "31183.p8.png",
    "31269.p8.png",
    "31458.p8.png",
    "31465.p8.png",
    "31474.p8.png",
    "31503.p8.png",
    "31546.p8.png",
    "32053.p8.png",
    "32485.p8.png",
    "32697.p8.png",
    "33667.p8.png",
    "34502.p8.png",
    "35079.p8.png",
    "36065.p8.png",
    "38099.p8.png",
    "38613.p8.png",
    "38733.p8.png",
    "38748.p8.png",
    "38801.p8.png",
    "38968.p8.png",
    "39488.p8.png",
    "39505.p8.png",
    "40026.p8.png",
    "40152.p8.png",
    "40309.p8.png",
    "40328.p8.png",
    "40832.p8.png",
    "40925.p8.png",
    "41333.p8.png",
    "41737.p8.png",
    "42371.p8.png",
    "42556.p8.png",
    "42594.p8.png",
    "42961.p8.png",
    "43336.p8.png",
    "43869.p8.png",
    "43885.p8.png",
    "44174.p8.png",
    "44176.p8.png",
    "44632.p8.png",
    "45029.p8.png",
    "49042.p8.png",
    "49173.p8.png",
    "49919.p8.png",
    "50458.p8.png",
    "50802.p8.png",
    "51786.p8.png",
    "52377.p8.png",
    "53087.p8.png",
    "53717.p8.png",
    "53887.p8.png",
    "53938.p8.png",
    "53994.p8.png",
    "54184.p8.png",
    "54407.p8.png",
    "54477.p8.png",
    "54654.p8.png",
    "54744.p8.png",
    "54912.p8.png",
    "55036.p8.png",
    "55219.p8.png",
    "55253.p8.png",
    "55422.p8.png",
    "55520.p8.png",
    "55576.p8.png",
    "55585.p8.png",
    "55699.p8.png",
    "55773.p8.png",
    "55781.p8.png",
    "55785.p8.png",
    "55795.p8.png",
    "56003.p8.png",
    "56040.p8.png",
    "56234.p8.png",
    "57097.p8.png",
    "57177.p8.png",
    "57355.p8.png",
    "57362.p8.png",
    "57398.p8.png",
    "57568.p8.png",
    "57748.p8.png",
    "58068.p8.png",
    "58100.p8.png",
    "58429.p8.png",
    "58624.p8.png",
    "58730.p8.png",
    "58876.p8.png",
    "58914.p8.png",
    "58923.p8.png",
    "59036.p8.png",
    "59043.p8.png",
    "59051.p8.png",
    "59078.p8.png",
    "59084.p8.png",
    "59095.p8.png",
    "59103.p8.png",
    "59115.p8.png",
    "59186.p8.png",
    "59190.p8.png",
    "ac_pensate_dw817-0.p8.png",
    "all_32_colors-0.p8.png",
    "amongtweets-0.p8.png",
    "angel_devil-3.p8.png",
    "another_3d_snake-0.p8.png",
    "another_bad_dream-2.p8.png",
    "aquova_pipes-0.p8.png",
    "aquova_simon_says-0.p8.png",
    "astroclerkfonts-1.p8.png",
    "atc1-0.p8.png",
    "atributetomule-0.p8.png",
    "avalanche_02-0.p8.png",
    "axnjaxn_ajml-2.p8.png",
    "axnjaxn_packmap-0.p8.png",
    "bean_pride_2020-0.p8.png",

});

string checkCart(Vm* vm, string cartDirectory, string cart){
    printf("%s\n", cart.c_str());
    vm->LoadCart(cartDirectory + "/" + cart, false);

    if (vm->GetBiosError() != "") {
        return vm->GetBiosError();
    }
    else
    {
        vm->UpdateAndDraw();

        if (vm->GetBiosError() != "") {
            vm->GetBiosError();
        }
    }

    vm->CloseCart();

    return "";
}

string checkCart_wrapper(string cartDirectory, string cart)
{
    std::mutex m;
    std::condition_variable cv;
    string retValue;

    Host* host = new Host();
    Vm* vm = new Vm(host);
    
    std::thread t([&cv, &vm, &cartDirectory, &cart, &retValue]() 
    {
        retValue = checkCart(vm, cartDirectory, cart);
        cv.notify_one();
    });
    
    t.detach();
    
    {
        std::unique_lock<std::mutex> l(m);
        if(cv.wait_for(l, 1s) == std::cv_status::timeout) {
            retValue = "timeout";
            //This leaks host and vm. Not sure of alternative?
            return retValue;
            //vm->CloseCart();
        }
    }

    delete vm;
    delete host;

    return retValue;    
}


TEST_CASE("Loading and running carts") {
    vector<string> carts;
    vector<string> errors;


    //TODO: build library of carts to test- set up compiler flag?
    std::string _cartDirectory = "/Users/jon/p8carts/archive/carts";

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (_cartDirectory.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if (isCartFile(ent->d_name) && std::find(cartsToIgnore.begin(), cartsToIgnore.end(), ent->d_name) == cartsToIgnore.end()){
                carts.push_back(ent->d_name);
            }
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
    }
    sort(carts.begin(), carts.end());

    //sort(cartsToIgnore.begin(), cartsToIgnore.end());
    string lastKnownError = cartsToIgnore[cartsToIgnore.size() - 1];


    Host* host = new Host();
    Vm* vm = new Vm(host);


    for(int i = 0; i < carts.size(); i++) {

        if (carts[i] < lastKnownError) {
            continue;
        }

        try {
            //string result = checkCart_wrapper(_cartDirectory, carts[i]);

            string result = checkCart(vm, _cartDirectory, carts[i]);

            if (result.length() > 0) {
                errors.push_back(carts[i] + ": " + result);
            }
        }
        catch(std::runtime_error& e) {
            errors.push_back(carts[i] + ": Timeout" + e.what());
        }
    }
    
    delete vm;
    delete host;
    
}

