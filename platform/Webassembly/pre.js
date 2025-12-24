//current command in ascii decimal
let currentcmd = [0,0,0] 
let currentfile = "";
const sleep = ms => new Promise(r => setTimeout(r,ms));
Module['print'] = function(text){console.log(text);}
Module['preRun'] = function()
{
    
    function stdin(){return 10};
    var stdout = null;
    var stderr = null; 
    FS.init(stdin,stdout,stderr);
    FS.mount(IDBFS,{},"/home/web_user/");
    FS.chdir("/home/web_user");
    
}
Module['noInitialRun'] = true
document.addEventListener('click', (ev) => {
    console.log("event is captured only once.");
    args = []
    document.getElementById("instructions").remove();
    FS.syncfs(true,function(){
        try {
            FS.mkdir("/home/web_user/p8carts")            
        } catch (error) {
            
        }
        try {
            FS.mkdir("/home/web_user/fake08")            
        } catch (error) {
            
        }

        Module.callMain(args);
});
    
  }, { once: true });

  