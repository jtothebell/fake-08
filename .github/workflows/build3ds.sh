source /etc/profile.d/devkit-env.sh

apt-get update && apt-get install p7zip-full -y

cd /build_dir

#todo: build makerom and bannertool to also build cia
export ctr_ver=16
curl -LJO https://github.com/jakcron/Project_CTR/releases/download/v0."$ctr_ver"/makerom_0"$ctr_ver"_ctrtool.zip
7z e makerom_0"$ctr_ver"_ctrtool.zip -o/usr/bin Ubuntu/makerom
7z e makerom_0"$ctr_ver"_ctrtool.zip -o/usr/bin Ubuntu/ctrtool
chmod +x /usr/bin/makerom && chmod +x /usr/bin/ctrtool

#Build 3dsx
make 3ds

#Build cia
make cia

