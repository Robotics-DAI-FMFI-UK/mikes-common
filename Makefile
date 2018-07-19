CC=gcc
CPP=g++
TEST_PQ_SRCS=tests/test_pq.c \
             bites/pq.c
TEST_ASTAR_SRCS=tests/test_astar.c \
                modules/passive/astar.c \
                bites/pq.c
TEST_POSE_SRCS=tests/test_pose.c \
               modules/passive/pose.c \
               modules/passive/mikes_logs.c \
               config/config.c \
               core/config_mikes.c \
               bites/util.c
TEST_BASE_SRCS=tests/test_base_module.c \
               modules/live/base_module.c \
               modules/passive/mikes_logs.c \
               modules/passive/pose.c \
               bites/util.c \
               bites/mikes.c \
               core/config_mikes.c \
               config/config.c
TEST_ASTAR_OBJS=${TEST_ASTAR_SRCS:.c=.o}
TEST_POSE_OBJS=${TEST_POSE_SRCS:.c=.o}
TEST_PQ_OBJS=${TEST_PQ_SRCS:.c=.o}
TEST_BASE_OBJS=${TEST_BASE_SRCS:.c=.o}

TEST_CPPSRCS=
TEST_CPPOBJS=${TEST_CPPSRCS:.cpp=.o}

OPTIMIZE=-O0
DEBUG_FLAGS=-g
CFLAGS=${OPTIMIZE} -std=c11 -D_BSD_SOURCE -D_XOPEN_SOURCE=600 -I. -I/usr/include/cairo -I/usr/local/rplidar/sdk/sdk/include -I/usr/include/librsvg-2.0/librsvg -I/usr/include/glib-2.0 -I/usr/lib/arm-linux-gnueabihf/glib-2.0/include -I/usr/include/libxml2 -I/usr/include/gdk-pixbuf-2.0 -Wall
CPPFLAGS=${OPTIMIZE} ${DEBUG_FLAGS} -D_BSD_SOURCE -D_XOPEN_SOURCE=600 -I/usr/include/cairo -I/usr/local/rplidar/sdk/sdk/include -Wall -Wno-write-strings -I/usr/include/librsvg-2.0/librsvg -I/usr/include/glib-2.0 -I/usr/lib/arm-linux-gnueabihf/glib-2.0/include -I/usr/include/gdk-pixbuf-2.0 
LDFLAGS=${DEBUG_FLAGS} -lpthread -lrt -lcairo -lX11 -lm -lncurses -L/usr/local/rplidar/sdk/output/Linux/Release -lrplidar_sdk -lrsvg-2 -lxml2 -g
PREFIX=/usr/local

all: test

# ${OBJS} ${CPPOBJS} 
#	${CPP} ${OBJS} ${CPPOBJS} -o ${PROG} ${CFLAGS} ${LDFLAGS}

install:

test:	test_pq test_astar test_pose test_base

test_pq: ${TEST_PQ_OBJS}
	${CC} -o test_pq $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_astar: ${TEST_ASTAR_OBJS}
	${CC} -o test_astar $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_pose: ${TEST_POSE_OBJS}
	${CC} -o test_pose $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_base: ${TEST_BASE_OBJS}
	${CC} -o test_base $^ ${LDFLAGS} ${DEBUG_FLAGS}
     
uninstall:

clean:
	rm -f *.o */*.o */*/*.o test_pq test_astar test_pose test_base

