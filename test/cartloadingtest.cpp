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
    "50646.p8.png",
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
    "bo-0.p8.png",
    "boc_roygbiv-0.p8.png",
    "bosegizare-0.p8.png",
    "br-1.p8.png",
    "brw-3.p8.png",
    "catacombae-0.p8.png",
    "catch128-0.p8.png",
    "cds-1.p8.png",
    "ch_func_demo-0.p8.png",
    "char_maker-5.p8.png",
    "chrprinter-2.p8.png",
    "citylimits-0.p8.png",
    "cocreate_demo-0.p8.png",
    "convparade-0.p8.png",
    "conways_garden-0.p8.png",
    "corrupt-4.p8.png",
    "cosmic_painter-0.p8.png",
    "crashlinernd-0.p8.png",
    "crazy_maze-0.p8.png",
    "cs-1.p8.png",
    "cursed_petri-0.p8.png",
    "cursed_sword-14.p8.png", //sigsegv
    "d8ing-0.p8.png",
    "debug-2.p8.png",
    "deep-0.p8.png", //sigsegv
    "deform-0.p8.png",
    "dehobigezu-0.p8.png",
    "demo_realtime_sdf-0.p8.png",
    "diruwopusa-0.p8.png",
    "diwipeboju-0.p8.png",
    "dofinihade-0.p8.png",
    "dt-0.p8.png",
    "dufrasago-1.p8.png",
    "duhozayoda-0.p8.png",
    "dvd-0.p8.png",
    "dvdtweet-0.p8.png",
    "empathy-0.p8.png",
    "esiyc-0.p8.png",
    "evening_train_ride-0.p8.png",
    "eyn_smplsndbx-0.p8.png",
    "f2ba-5.p8.png",
    "fairchild001-10.p8.png",
    "fb-1.p8.png",
    "fesiwimufa-0.p8.png",
    "fireballrebirth-0.p8.png",
    "firroref-0.p8.png",
    "flippingout-0.p8.png",
    "fohekireba-0.p8.png",
    "fohekireba-1.p8.png",
    "fokmawope-0.p8.png",
    "fract-1.p8.png",
    "frlytweetningow-4.p8.png",
    "frostpunk_pico-1.p8.png",
    "frozax_windrose-0.p8.png",
    "fruitlicker-1.p8.png",
    "gajeyabin-0.p8.png",
    "game_and_timer_ball-2.p8.png", //sigsegv
    "game_and_timer_vermin-1.p8.png", //sigsegv
    "gbc_startup-0.p8.png",
    "ghostbuster-0.p8.png",
    "gijenijato-0.p8.png",
    "gimufopapi-0.p8.png",
    "giopacbeta0-0.p8.png",
    "gojefesozi-0.p8.png",
    "golf560-0.p8.png",
    "golfnado_pico_beanborg-0.p8.png",
    "gotchimiya-0.p8.png",
    "gumiradesu-0.p8.png",
    "ha-1.p8.png",
    "hackattack-0.p8.png",
    "hg-6.p8.png",
    "higininufo-0.p8.png",
    "higumakeba-1.p8.png",
    "hitting_the_slopes-1.p8.png",
    "hot_simba-0.p8.png",
    "hs-0.p8.png",
    "humblebumble-3.p8.png",
    "husotekaso-0.p8.png",
    "huwihewesa-0.p8.png",
    "im_a_big_fan-0.p8.png",
    "impossible-1.p8.png",
    "irrigationsim1-18.p8.png",
    "iymp-0.p8.png",
    "jack_o_random-0.p8.png",
    "jammerboard-1.p8.png",
    "jiyiyarodo-0.p8.png",
    "jokenpico-0.p8.png",
    "jotototubo-0.p8.png",
    "jump_flood-1.p8.png",
    "jumugujido-0.p8.png",
    "jutejoguto-0.p8.png",
    "karel_the_robot-0.p8.png",
    "katamar1k-0.p8.png",
    "keep_it_alive-0.p8.png",
    "kesopohn-0.p8.png",
    "kirbyinstarfield-0.p8.png",
    "kirbys_big_bean_blast-0.p8.png",
    "kukafamapa-0.p8.png",
    "kumodot02-0.p8.png",
    "la-0.p8.png",
    "lighthouse-0.p8.png",
    "lonesector-0.p8.png",
    "loveisfunny-0.p8.png",
    "lr280-0.p8.png",
    "lucylovepebs-0.p8.png",
    "lunar_blackout-0.p8.png",
    "luvbomber-0.p8.png",
    "lzw-1.p8.png",
    "mab-0.p8.png",
    "mawonidaba-0.p8.png",
    "microgolf-6.p8.png",
    "midcopy1234-0.p8.png",
    "mifehifiwi-0.p8.png",
    "migukohizo-0.p8.png",
    "mikimekar-2.p8.png",
    "monraibresim-0.p8.png",
    "moon_rises-1.p8.png",
    "mot_dodgeballs-1.p8.png",
    "mujusukip-0.p8.png",
    "navonova_wip1-0.p8.png",
    "ndarwyi-0.p8.png",
    "newfont2021-0.p8.png",
    "nhr-4.p8.png",
    "nilquesteasymode-0.p8.png",
    "nipewihimi-0.p8.png",
    "noclip5_card-0.p8.png",
    "noclipexistential-0.p8.png",
    "nocliplost-1.p8.png",
    "noclipseen-0.p8.png",
    "noggins_tweet-0.p8.png",
    "notabs-0.p8.png",
    "nuhodupiku-0.p8.png",
    "nuzobanija-1.p8.png",
    "obj_import_v01-1.p8.png",
    "oneoffgrid-1.p8.png",
    "p8os_early-1.p8.png",
    "p_harrier-0.p8.png",
    "pandemic-0.p8.png",
    "paziwerize-0.p8.png",
    "pc-0.p8.png",
    "pejawedogu-0.p8.png",
    "pet_the_puppos-0.p8.png",
    "phonemes-0.p8.png",
    "pico8lisp1-7.p8.png",
    "pico_dictator-0.p8.png",
    "pico_dictator101-0.p8.png",
    "pico_punks-0.p8.png",
    "picochallenge-0.p8.png",
    "picoenix-0.p8.png",
    "picoforth-0.p8.png",
    "picolander-0.p8.png", //sigsegv - intermittent?
    "picosolo-4.p8.png",
    "picoteras_1-0.p8.png",
    "picotrainsim-2.p8.png",
    "picozelda-2.p8.png",
    "pinchpoint-0.p8.png",
    "pug_nographicpong-4.p8.png",
    "pushline-0.p8.png",
    "pw_renoiser-2.p8.png",
    "px3_font-0.p8.png",
    "px9-8.p8.png",
    "quadmirrortool-0.p8.png",
    "quantumbullshit-1.p8.png",
    "quasarkid-7.p8.png",
    "rainbow_tunnel-0.p8.png",
    "rajadadaji-0.p8.png",
    "rawepemaze-0.p8.png",
    "reference-0.p8.png",
    "refifomope-0.p8.png",
    "remcode_backbuffer-4.p8.png",
    "renapanehi-0.p8.png",
    "render_speed_dif-0.p8.png",
    "rg_snowytweet-0.p8.png",
    "rg_tweettweet2-0.p8.png",
    "roreguwuwo-0.p8.png",
    "rubaseruzi-0.p8.png",
    "ruzihedana-0.p8.png",
    "s2tb-0.p8.png",
    "sa-0.p8.png",
    "santa1080-4.p8.png",
    "scrap_boy_jam-0.p8.png",
    "sebatudfi-0.p8.png",
    "sepbowka-0.p8.png",
    "shelob-0.p8.png",
    "shelost8bitcontrol-0.p8.png",
    "simple_mouse-0.p8.png",
    "simpleca_040421-3.p8.png",
    "siparefiti-0.p8.png",
    "slingshott-0.p8.png",
    "slushies3_v1-0.p8.png",
    "smal-0.p8.png",
    "smr_beta-0.p8.png",
    "smr_extra_themes-0.p8.png",
    "snowflake_tweetcart-0.p8.png",
    "sonder-0.p8.png",
    "sots-2.p8.png",
    "spaceinvader0-8.p8.png",
    "spaceoban-1.p8.png",
    "spajukuro-0.p8.png",
    "spritetool-1.p8.png",
    "spys_demise-3.p8.png",
    "squiddy-0.p8.png",
    "starlight_saviors-0.p8.png",
    "starmr-0.p8.png",
    "starshot560-0.p8.png",
    "stinkerb06rmg-1.p8.png",
    "str2board-0.p8.png",
    "sudjusiwe-0.p8.png",
    "sufehasede-0.p8.png",
    "sunrise-0.p8.png",
    "super_mario_pico_stars-1.p8.png",
    "superdiscbox-0.p8.png",
    "swatty-0.p8.png",
    "sweet_sip-0.p8.png",
    "swirlypinwheel-0.p8.png",
    "tardis_toolkit-1.p8.png",
    "tater_evader-0.p8.png",
    "td_3ddots-0.p8.png",
    "tempres-0.p8.png",
    "tentropy-0.p8.png",
    "terrainauto-0.p8.png",
    "terraworldviewer-3.p8.png",
    "the_wall-0.p8.png",
    "tinier_mondrian-0.p8.png",
    "tirudomonu-0.p8.png",
    "tline_sprite_rotation-2.p8.png",
    "ttj5_canary-2.p8.png",
    "tuteduhipo-0.p8.png",
    "tweetbubblewrap-1.p8.png",
    "tweetcartgame-1.p8.png",
    "tweetcollectorgame-0.p8.png",
    "tweetpaint-4.p8.png",
    "tweettweetjamsnake-0.p8.png",
    "twtw_bnb_remcode-0.p8.png",
    "ultima_vi_intro_music-0.p8.png",
    "untitled_sfx-0.p8.png",
    "voroterrain-5.p8.png",
    "vowels-0.p8.png",
    "wadesupuso-0.p8.png",
    "wapedabika-0.p8.png",
    "wayofemigu-0.p8.png",
    "weaver-0.p8.png",
    "whereswilly-2.p8.png",
    "whiletrue-0.p8.png",
    "wijagabufi-0.p8.png",
    "windstock-0.p8.png",
    "wurorogoga-0.p8.png",
    "xor_flower-0.p8.png",
    "yijebofuse-0.p8.png",
    "yizokigaga-0.p8.png",
    "yugoyigabo-1.p8.png",
    "ywosahuru-3.p8.png",
    "zamidenamu-0.p8.png",
    "zegegawido-0.p8.png",
    "zehomimugi-0.p8.png",
    "zodajeph-0.p8.png",
    "zoyifizotu-0.p8.png",

    //newly introduced 20220710 -- looks like from fixing exponent parsing
    "39807.p8.png", // ?? this one might be intermittent
    "59251.p8.png",
    "amongthestars560-3.p8.png",
    "avoid_the_worms-0.p8.png",
    "begi_ideocart-0.p8.png",
    "bitdraw-3.p8.png",
    "blockbop560-1.p8.png",
    "borecode-0.p8.png",
    "brake_machine_broke_ld49-1.p8.png",
    "bubblecat-0.p8.png",
    "bubblecat-2.p8.png",
    "bubblegum-0.p8.png",
    "chico8-2.p8.png",
    "christmas_present_panic-1.p8.png",
    "circleattack560-0.p8.png",
    "cursedconsole-0.p8.png",
    "curve_dasher-0.p8.png",
    "ebeamanime2-0.p8.png",
    "endlessrunner_tweet-0.p8.png",
    "fishy-0.p8.png",
    "flappy_tweet-1.p8.png",
    "footy560-0.p8.png",
    "gt-1.p8.png",
    "inside_the_hole-0.p8.png",
    "isidore-0.p8.png",
    "jetpackcollectorgame-1.p8.png", //crashes, doesn't hang
    "ketabarihu-0.p8.png",
    "kinezapubo-0.p8.png",
    "lemniscate_redux-0.p8.png",
    "manvirusreal-0.p8.png",
    "mmo_moon_tut_ver-0.p8.png",
    "munro_tweetcarts-5.p8.png",
    //"nayadunaze-1.p8.png", //just long load
    "nintweetasketch-0.p8.png",
    "p8r-4.p8.png",
    "p8sciiviz560-0.p8.png",
    "petri-0.p8.png",
    "pico_1k_jam_invitation-0.p8.png",
    "picoforth-4.p8.png",
    "pigeo-0.p8.png",
    "r56oh-0.p8.png",
    "racenosun_560-0.p8.png",
    "redblocc-0.p8.png",
    "rg_discord_swarm-0.p8.png",
    "si-1.p8.png",
    "somuneyetu-0.p8.png",
    "spirograph-1.p8.png",
    "suziwagiwo-0.p8.png",
    "t3dsd-0.p8.png",
    "thepicomermaid-0.p8.png",
    "totafohese-0.p8.png",
    "tweet_invaders-0.p8.png",
    "tweetgario-0.p8.png",
    "tweetormergame-0.p8.png",
    "twrogue-2.p8.png",
    "void_roller-0.p8.png",
    "vvm-6.p8.png",
    "wall_jump_platformer-4.p8.png",
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
/*
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
*/


TEST_CASE("Loading and running carts") {
    vector<string> carts;
    vector<string> errors;


    //TODO: build library of carts to test- set up compiler flag?
    std::string _cartDirectory = "";

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (_cartDirectory.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if (isCartFile(ent->d_name) 
                && ! isCPostFile(ent->d_name)
                && std::find(cartsToIgnore.begin(), cartsToIgnore.end(), ent->d_name) == cartsToIgnore.end()) {
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

        //if (carts[i] < lastKnownError) {
        //    continue;
        //}

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

    printf("\n");
    printf("ERRORS:\n");
    for (int i = 0; i < errors.size(); i++) {
        printf("%s\n", errors[i].c_str());
    }
    
    //TODO: compile known errors, make sure none are introduced

    delete vm;
    delete host;
    
}

