source /etc/profile.d/devkit-env.sh

apt-get update && apt-get install p7zip-full zip libc6-dev g++ -y

cd /build_dir

#dl makerom
export ctr_ver=16
curl -LJO https://github.com/jakcron/Project_CTR/releases/download/v0."$ctr_ver"/makerom_0"$ctr_ver"_ctrtool.zip
7z e makerom_0"$ctr_ver"_ctrtool.zip -o/usr/bin Ubuntu/makerom

#dl bannertool
curl -LJO https://github.com/diasurgical/bannertool/releases/download/1.2.0/bannertool.zip
7z e bannertool.zip -o/usr/bin linux-x86_64/bannertool

chmod +x /usr/bin/makerom && chmod +x /usr/bin/bannertool

cd /build_dir

#Build 3dsx
make 3ds

#Build cia
make cia

