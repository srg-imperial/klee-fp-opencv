LLVM_BUILD_PATH = /homes/pcc03/data/src/llvm/llvm-2.7-dbg/Debug
LLVMGCC_PATH = /homes/pcc03/data/src/llvm/llvm-gcc4.2-2.7-x86_64-linux
KLEE_PATH = /homes/pcc03/data/src/llvm/klee
OPENCV_PATH = /data/pcc03/src/OpenCV-2.1.0
OPENCV_BUILD_PATH = /data/pcc03/src/OpenCV-2.1.0-build-llvm

all: eigenval.exe harris.exe

%.bc: %.cpp
	$(LLVMGCC_PATH)/bin/llvm-g++ -I$(OPENCV_PATH)/include/opencv -I$(KLEE_PATH)/include/klee -I$(OPENCV_BUILD_PATH) -c -emit-llvm $< -o $@

%.exe: %.bc
	$(LLVM_BUILD_PATH)/bin/llvm-ld -disable-opt $< $(OPENCV_BUILD_PATH)/lib/libcv.a $(OPENCV_BUILD_PATH)/lib/libcxcore.a -o $@

clean:
	rm -f eigenval.exe harris.exe *.bc
