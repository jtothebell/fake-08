source /etc/profile.d/devkit-env.sh

cd /build_dir

#Build miyoomini core
cd platform/libretro
make platform=libnx
