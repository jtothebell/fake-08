#source /etc/profile.d/devkit-env.sh

cd /build_dir

#Build core
cd platform/libretro
make echo platform=libnx
make platform=libnx
