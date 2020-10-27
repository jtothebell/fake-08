source /etc/profile.d/devkit-env.sh

cd /build_dir

#todo: build makerom and bannertool to also build cia

#Build 3dsx
make 3ds

#todo: copy artifacts for uploading
