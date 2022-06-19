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


const vector<string> cartsToIgore ({
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
            if (isCartFile(ent->d_name) && std::find(cartsToIgore.begin(), cartsToIgore.end(), ent->d_name) == cartsToIgore.end()){
                carts.push_back(ent->d_name);
            }
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
    }
    sort(carts.begin(), carts.end());


    //Host* host = new Host();
    //Vm* vm = new Vm(host);


    for(int i = 0; i < carts.size(); i++) {
        try {
            string result = checkCart_wrapper(_cartDirectory, carts[i]);

            //string result = checkCart(vm, _cartDirectory, carts[i]);

            if (result.length() > 0) {
                errors.push_back(carts[i] + ": " + result);
            }
        }
        catch(std::runtime_error& e) {
            errors.push_back(carts[i] + ": Timeout" + e.what());
        }
    }
    
    //delete vm;
    //delete host;
    
}

