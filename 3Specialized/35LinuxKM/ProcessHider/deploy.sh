REMOTE_USER=kiennd19
REMOTE_ADDRESS=192.168.88.147

BUILD_TARGETS="build/ProcessHiderKM.ko build/ProcessHider.o build/meter.o"
scp $BUILD_TARGETS $REMOTE_USER@$REMOTE_ADDRESS:/home/$REMOTE_USER/module_test/