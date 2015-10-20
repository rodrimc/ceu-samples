CFLAGS = -DCEU_DEBUG #-DCEU_DEBUG_TRAILS
#CFLAGS = -DSDL_MATH

MICRO_MB_CFLAGS = -I/usr/local/include/micromb-0.10 \
									-g -Wall `pkg-config --cflags gstreamer-video-1.0`

MICRO_MB_LDFLAGS = -L/usr/local/lib -lmicromb \
									 `pkg-config --libs gstreamer-video-1.0`

all:
	ceu --cpp-args "-I ." $(CEUFILE)
	gcc -g -Os main.c $(CFLAGS) $(MICRO_MB_CFLAGS) -lpthread -lm \
	$(MICRO_MB_LDFLAGS)	-o $(basename $(CEUFILE))

clean:
	find . -name "*.exe"  | xargs rm -f
	find . -name "_ceu_*" | xargs rm -f

.PHONY: all clean
