#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

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

TEST_CASE("Loading and running carts") {
    Host* host = new Host();
    Vm* vm = new Vm(host);

    vector<string> carts;
    vector<string> errors;

    //TODO: build library of carts to test- set up compiler flag?
    std::string _cartDirectory = "/Users/jon/p8carts/infinitelooptest";

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (_cartDirectory.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if (isCartFile(ent->d_name)){
                carts.push_back(ent->d_name);
            }
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
    }
    sort(carts.begin(), carts.end());


    for(int i = 0; i < carts.size(); i++) {
        //printf("%s\n", carts[i].c_str());
        vm->LoadCart(_cartDirectory + "/" + carts[i], false);

        if (vm->GetBiosError() != "") {
            errors.push_back(carts[i] + ": " + vm->GetBiosError());
        }
        else
        {
            vm->UpdateAndDraw();

            if (vm->GetBiosError() != "") {
                errors.push_back(carts[i] + vm->GetBiosError());
            }
        }

        vm->CloseCart();
    }
    
    
    delete vm;
    delete host;
}