include Makefile.config

BENCHMARKS = eigenval.exe harris.exe transff.43.exe transsf.43.exe transff.44.exe transsf.44.exe stereobm.exe filter.exe resize.exe moments.exe morph.exe thresh.exe silhouette.exe pyramid.exe
CONC_BENCHMARKS = harris.conc stereobm.conc filter.conc resize.conc moments.conc transsf.conc morph.conc thresh.conc silhouette.conc pyramid.conc

LLVMGCC_COMPILE = $(LLVMGCC_PATH)/bin/llvm-g++ $(CXXFLAGS) -I$(OPENCV_PATH)/include/opencv -I$(KLEE_PATH)/include/klee -I$(OPENCV_BUILD_PATH) -c -emit-llvm

all: $(BENCHMARKS)

conc: $(CONC_BENCHMARKS)

%.bc: %.cpp
	$(LLVMGCC_COMPILE) $< -o $@

%.43.bc: %.cpp
	$(LLVMGCC_COMPILE) -DTRANS_N=4 -DTRANS_NC=3 $< -o $@

%.44.bc: %.cpp
	$(LLVMGCC_COMPILE) -DTRANS_N=4 -DTRANS_NC=4 $< -o $@

%.o: %.cpp
	g++ $(CXXFLAGS) -ggdb3 -D__CONCRETE -I$(OPENCV_PATH)/include/opencv -I$(OPENCV_CONC_BUILD_PATH) -c $< -o $@

%.exe: %.bc
	$(LLVM_BUILD_PATH)/bin/llvm-ld -disable-opt $< $(OPENCV_BUILD_PATH)/lib/libcv.a $(OPENCV_BUILD_PATH)/lib/libcxcore.a -o $@

%.conc: %.o
	g++ $< -L$(OPENCV_CONC_BUILD_PATH)/lib -Wl,-rpath,$(OPENCV_CONC_BUILD_PATH)/lib -lcv -lcxcore -o $@

OpenCV-2.1.0.tar.bz2:
	wget http://downloads.sourceforge.net/project/opencvlibrary/opencv-unix/2.1/OpenCV-2.1.0.tar.bz2

opencv: OpenCV-2.1.0.tar.bz2
	rm -rf $(OPENCV_PATH) $(OPENCV_BUILD_PATH)
	cd $(shell dirname $(OPENCV_PATH)) && \
	  tar xjf $(shell pwd)/OpenCV-2.1.0.tar.bz2 && \
	  if \! test -e $(OPENCV_PATH) ; then \
	    mv OpenCV-2.1.0 $(shell basename $(OPENCV_PATH)) ; \
	  fi
	cd $(OPENCV_PATH) && \
	  for patch in $(shell pwd)/OpenCV-2.1.0-*.patch ; do \
	    patch -p1 < $$patch ; \
	  done
	mkdir -p $(OPENCV_BUILD_PATH)
	cd $(OPENCV_BUILD_PATH) && $(shell pwd)/cmake-opencv $(OPENCV_PATH)
	-make -C$(OPENCV_BUILD_PATH)

clean:
	rm -f $(BENCHMARKS) $(CONC_BENCHMARKS) *.bc
