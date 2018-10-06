CC=gcc
CPP=g++
SAN = -fsanitize=undefined               \
      -fsanitize=address                 \
      -fsanitize=leak                    \
      -fsanitize=shift                   \
      -fsanitize=integer-divide-by-zero  \
      -fsanitize=unreachable             \
      -fsanitize=vla-bound               \
      -fsanitize=null
MIKES_BASIC=modules/passive/mikes_logs.c \
            config/config.c \
            core/config_mikes.c \
            bites/mikes.c \
            bites/util.c
TEST_PQ_SRCS=tests/test_pq.c \
             bites/pq.c
TEST_ASTAR_SRCS=tests/test_astar.c \
                modules/passive/astar.c \
                bites/pq.c
TEST_POSE_SRCS=tests/test_pose.c \
               modules/passive/pose.c \
               ${MIKES_BASIC}
TEST_BASE_SRCS=tests/test_base_module.c \
               modules/live/base_module.c \
               modules/passive/pose.c \
               ${MIKES_BASIC}
TEST_NCURSES_SRCS=tests/test_ncurses_control.c \
                 modules/live/ncurses_control.c \
                 ${MIKES_BASIC}
TEST_TIM571_SRCS=tests/test_tim571.c \
                 modules/live/tim571.c \
                 ${MIKES_BASIC}
TEST_GUI_SRCS=tests/test_gui.c \
              modules/live/gui.c \
              ${MIKES_BASIC}
TEST_X_TIM571_SRCS=tests/test_x_tim571.c \
                   modules/live/tim571.c \
                   modules/passive/x_tim571.c \
                   modules/live/gui.c \
                   modules/live/tim_hough_transform.c \
                   modules/live/line_filter.c \
                   modules/live/tim_segment.c \
                   bites/hough.c \
                   bites/filter.c \
                   bites/math_2d.c \
                   bites/segment.c \
                   bites/corner.c \
                   ${MIKES_BASIC}
TEST_X_BASE_SRCS=tests/test_x_base.c \
                   modules/live/base_module.c \
                   modules/passive/pose.c \
                   modules/passive/x_base.c \
                   modules/live/gui.c \
                   ${MIKES_BASIC}
TEST_RFID_SRCS=tests/test_rfid.c \
                   modules/live/rfid_sensor.c \
                   ${MIKES_BASIC}
TEST_UST10LX_SRCS=tests/test_ust10lx.c \
                 modules/live/ust10lx.c \
                 ${MIKES_BASIC}
TEST_X_UST10LX_SRCS=tests/test_x_ust10lx.c \
                   modules/live/ust10lx.c \
                   modules/passive/x_ust10lx.c \
                   modules/live/gui.c \
                   ${MIKES_BASIC}
TEST_LIDAR_SRCS=tests/test_rplidar.c \
                ${MIKES_BASIC}
TEST_LIDAR_CPPSRCS=modules/live/lidar.cpp
TEST_X_LIDAR_SRCS=tests/test_x_rplidar.c \
                   modules/passive/x_lidar.c \
                   modules/live/gui.c \
                   ${MIKES_BASIC}
TEST_X_LIDAR_CPPSRCS=modules/live/lidar.cpp
TEST_PNGWRITER_SRCS=tests/test_pngwriter.c \
                    bites/pngwriter.c
TEST_XTION_SRCS=tests/test_xtion.c \
                ${MIKES_BASIC}
XTION_SRCS=modules/live/xtion/xtion.cpp
XTION_OBJS=modules/live/xtion/Arm-Release/xtion.o
TEST_X_XTION_SRCS=tests/test_x_xtion.c \
                  modules/passive/x_xtion.c \
                  modules/live/gui.c \
                  bites/pngwriter.c \
                  ${MIKES_BASIC}
TEST_X_LINE_MAP_SRCS=tests/test_x_line_map.c \
                     modules/passive/x_line_map.c \
                     modules/live/gui.c \
                     modules/live/base_module.c \
                     modules/passive/pose.c \
                     ${MIKES_BASIC}
TEST_X_LINE_MAP_O_SRCS=tests/test_x_line_map_original.c \
                       modules/passive/x_line_map.c \
                       modules/live/gui.c \
                       ${MIKES_BASIC}
TEST_LINE_MAP_SRCS=tests/test_line_map.c \
                   modules/passive/line_map.c \
                   ${MIKES_BASIC}
TEST_HOUGH_SRCS=tests/test_hough.c \
                tests/hough_tests.c \
                bites/hough.c \
		            bites/math_2d.c
TEST_MATH_SRCS=tests/test_math_2d.c \
               bites/math_2d.c
TEST_NXT_SRCS=tests/test_nxt.c \
              modules/live/nxt.c \
              ${MIKES_BASIC}
TEST_WHEELS_SRCS=tests/test_wheels.c \
                 modules/passive/wheels.c \
                 ${MIKES_BASIC}

TEST_ASTAR_OBJS=${TEST_ASTAR_SRCS:.c=.o}
TEST_POSE_OBJS=${TEST_POSE_SRCS:.c=.o}
TEST_PQ_OBJS=${TEST_PQ_SRCS:.c=.o}
TEST_BASE_OBJS=${TEST_BASE_SRCS:.c=.o}
TEST_NCURSES_OBJS=${TEST_NCURSES_SRCS:.c=.o}
TEST_TIM571_OBJS=${TEST_TIM571_SRCS:.c=.o}
TEST_GUI_OBJS=${TEST_GUI_SRCS:.c=.o}
TEST_X_TIM571_OBJS=${TEST_X_TIM571_SRCS:.c=.o}
TEST_X_BASE_OBJS=${TEST_X_BASE_SRCS:.c=.o}
TEST_RFID_OBJS=${TEST_RFID_SRCS:.c=.o}
TEST_UST10LX_OBJS=${TEST_UST10LX_SRCS:.c=.o}
TEST_X_UST10LX_OBJS=${TEST_X_UST10LX_SRCS:.c=.o}
TEST_LIDAR_OBJS=${TEST_LIDAR_SRCS:.c=.o} ${TEST_LIDAR_CPPSRCS:.cpp=.o}
TEST_X_LIDAR_OBJS=${TEST_X_LIDAR_SRCS:.c=.o} ${TEST_X_LIDAR_CPPSRCS:.cpp=.o}
TEST_PNGWRITER_OBJS=${TEST_PNGWRITER_SRCS:.c=.o} ${TEST_PNGWRITER_SRCS:.c=.o}
TEST_XTION_OBJS=${TEST_XTION_SRCS:.c=.o} ${XTION_OBJS}
TEST_X_XTION_OBJS=${TEST_X_XTION_SRCS:.c=.o} ${XTION_OBJS}
TEST_X_LINE_MAP_OBJS=${TEST_X_LINE_MAP_SRCS:.c=.o}
TEST_X_LINE_MAP_O_OBJS=${TEST_X_LINE_MAP_O_SRCS:.c=.o}
TEST_LINE_MAP_OBJS=${TEST_LINE_MAP_SRCS:.c=.o}
TEST_HOUGH_OBJS=${TEST_HOUGH_SRCS:.c=.o}
TEST_MATH_OBJS=${TEST_MATH_SRCS:.c=.o}
TEST_NXT_OBJS=${TEST_NXT_SRCS:.c=.o}
TEST_WHEELS_OBJS=${TEST_WHEELS_SRCS:.c=.o}

TEST_CPPSRCS=
TEST_CPPOBJS=${TEST_CPPSRCS:.cpp=.o}

#OPTIMIZE=-O0 ${SAN}
OPTIMIZE=-O0
DEBUG_FLAGS=-g
CFLAGS=${OPTIMIZE} ${DEBUG_FLAGS} -std=c11 -D_BSD_SOURCE -D_XOPEN_SOURCE=600 -I. -I/usr/include/cairo -I/usr/local/rplidar/sdk/sdk/include -I/usr/include/librsvg-2.0/librsvg `pkg-config --cflags gio-2.0` -I/usr/include/libxml2 -I/usr/include/gdk-pixbuf-2.0 -Wall
CPPFLAGS=${OPTIMIZE} ${DEBUG_FLAGS} -D_BSD_SOURCE -D_XOPEN_SOURCE=600 -I. -I/usr/include/cairo -I/usr/local/rplidar/sdk/sdk/include -Wall -Wno-write-strings -I/usr/include/librsvg-2.0/librsvg `pkg-config --cflags gio-2.0` -I/usr/include/gdk-pixbuf-2.0
#LDFLAGS=${DEBUG_FLAGS} -pthread -lrt -lcairo -lX11 -lm -lncurses -L/usr/local/rplidar/sdk/output/Linux/Release -lrplidar_sdk -lrsvg-2 -lxml2 -g -lstdc++ ${SAN} -lubsan
LDFLAGS=${DEBUG_FLAGS} -pthread -lrt -lcairo -lX11 -lm -lncurses -L/usr/local/rplidar/sdk/output/Linux/Release -lrplidar_sdk -lrsvg-2 -lxml2 -lpng -lstdc++ `pkg-config --libs gio-2.0`
PREFIX=/usr/local
MIKES_CORE=$(shell pwd)
export MIKES_CORE

all: test

# ${OBJS} ${CPPOBJS}
#	${CPP} ${OBJS} ${CPPOBJS} -o ${PROG} ${CFLAGS} ${LDFLAGS}

install:

test:	test_pq test_astar test_pose test_base test_ncurses_control test_tim571 test_gui test_x_tim571 test_x_base test_rfid test_ust10lx test_x_ust10lx test_rplidar test_x_rplidar test_pngwriter test_xtion test_x_xtion test_x_line_map test_line_map test_hough test_math_2d test_nxt test_wheels test_x_line_map_original

test_pq: ${TEST_PQ_OBJS}
	${CC} -o test_pq $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_astar: ${TEST_ASTAR_OBJS}
	${CC} -o test_astar $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_pose: ${TEST_POSE_OBJS}
	${CC} -o test_pose $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_base: ${TEST_BASE_OBJS}
	${CC} -o test_base $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_ncurses_control: ${TEST_NCURSES_OBJS}
	${CC} -o test_ncurses_control $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_tim571: ${TEST_TIM571_OBJS}
	${CC} -o test_tim571 $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_gui: ${TEST_GUI_OBJS}
	${CC} -o test_gui $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_x_tim571: ${TEST_X_TIM571_OBJS}
	${CC} -o test_x_tim571 $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_x_base: ${TEST_X_BASE_OBJS}
	${CC} -o test_x_base $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_rfid: ${TEST_RFID_OBJS}
	${CC} -o test_rfid $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_ust10lx: ${TEST_UST10LX_OBJS}
	${CC} -o test_ust10lx $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_x_ust10lx: ${TEST_X_UST10LX_OBJS}
	${CC} -o test_x_ust10lx $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_rplidar: ${TEST_LIDAR_OBJS}
	${CPP} -o test_rplidar $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_x_rplidar: ${TEST_X_LIDAR_OBJS}
	${CPP} -o test_x_rplidar $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_pngwriter: ${TEST_PNGWRITER_OBJS}
	${CC} -o test_pngwriter $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_xtion: ${TEST_XTION_OBJS}
	${CPP} -o test_xtion $^ ${LDFLAGS} ${DEBUG_FLAGS} -lOpenNI
${XTION_OBJS}: ${XTION_SRCS}
	$(MAKE) -C modules/live/xtion
test_x_xtion: ${TEST_X_XTION_OBJS}
	${CPP} -o test_x_xtion $^ ${LDFLAGS} ${DEBUG_FLAGS} -lOpenNI
test_x_line_map: ${TEST_X_LINE_MAP_OBJS}
	${CC} -o test_x_line_map $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_x_line_map_original: ${TEST_X_LINE_MAP_O_OBJS}
	${CC} -o test_x_line_map_original $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_line_map: ${TEST_LINE_MAP_OBJS}
	${CC} -o test_line_map $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_hough: ${TEST_HOUGH_OBJS}
	${CC} -o test_hough $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_math_2d: ${TEST_MATH_OBJS}
	${CC} -o test_math_2d $^ ${LDFLAGS} ${DEBUG_FLAGS}
test_nxt: ${TEST_NXT_OBJS} nxt/NXTOperator.cs nxt/tests/TestMonoBrick.cs
	$(MAKE) -C nxt
	${CC} -o test_nxt ${TEST_NXT_OBJS} ${LDFLAGS} ${DEBUG_FLAGS}
test_wheels: ${TEST_WHEELS_OBJS}
	${CC} -o test_wheels $^ ${LDFLAGS} ${DEBUG_FLAGS}

uninstall:

clean:
	rm -f *.o */*.o */*/*.o test_pq test_astar test_pose test_base test_ncurses_control test_tim571 test_gui test_x_tim571 test_x_base test_rfid test_ust10lx test_x_ust10lx test_rplidar test_x_rplidar test_pngwriter test_xtion test_x_xtion test_x_line_map test_line_map test_hough test_math_2d test_nxt test_wheels test_x_line_map_original
	rm -rf modules/live/xtion/Arm-Release
	make -C nxt clean
	rm -f grey_gradient.png rgb_gradient.png
