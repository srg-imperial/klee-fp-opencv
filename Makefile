include Makefile.config

all: eigenval.exe harris.exe

%.bc: %.cpp
	$(LLVMGCC_PATH)/bin/llvm-g++ -I$(OPENCV_PATH)/include/opencv -I$(KLEE_PATH)/include/klee -I$(OPENCV_BUILD_PATH) -c -emit-llvm $< -o $@

%.exe: %.bc
	$(LLVM_BUILD_PATH)/bin/llvm-ld -disable-opt $< $(OPENCV_BUILD_PATH)/lib/libcv.a $(OPENCV_BUILD_PATH)/lib/libcxcore.a -o $@

OpenCV-2.1.0.tar.bz2:
	wget http://downloads.sourceforge.net/project/opencvlibrary/opencv-unix/2.1/OpenCV-2.1.0.tar.bz2

opencv: OpenCV-2.1.0.tar.bz2
	rm -rf $(OPENCV_PATH) $(OPENCV_BUILD_PATH)
	cd $(shell dirname $(OPENCV_PATH)) && \
	  tar xjf $(shell pwd)/OpenCV-2.1.0.tar.bz2 && \
	  mv OpenCV-2.1.0 $(shell basename $(OPENCV_PATH))
	cd $(OPENCV_PATH) && \
	  for patch in $(shell pwd)/OpenCV-2.1.0-*.patch ; do \
	    patch -p1 < $$patch ; \
	  done
	mkdir -p $(OPENCV_BUILD_PATH)
	cd $(OPENCV_BUILD_PATH) && $(shell pwd)/cmake-opencv $(OPENCV_PATH)
	-make -C$(OPENCV_BUILD_PATH)
	
clean:
	rm -f eigenval.exe harris.exe *.bc
