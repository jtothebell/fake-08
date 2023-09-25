#source /etc/profile.d/devkit-env.sh

cd /build_dir

#Build core
cd platform/libretro
make echo platform=libnx
make platform=libnx

cp fake08_libretro_libnx.a ../../retroarch/libretro_libnx.a

cd ../../retroarch

make -f Makefile.libnx

cp retroarch_switch.nro fake08_libretro_libnx.nro
