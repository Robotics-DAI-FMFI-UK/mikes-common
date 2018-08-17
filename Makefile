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
TEST_NCURSES_SRCS=tests/test_ncurses_control.c \
                 bites/mikes.c \
                 bites/util.c \
                 modules/passive/mikes_logs.c \
                 modules/live/ncurses_control.c \
                 core/config_mikes.c \
                 config/config.c
TEST_TIM571_SRCS=tests/test_tim571.c \
                 modules/live/tim571.c \
                 modules/passive/mikes_logs.c \
                 bites/mikes.c \
                 core/config_mikes.c \
                 bites/util.c \
                 config/config.c
TEST_GUI_SRCS=tests/test_gui.c \
              modules/live/gui.c \
              modules/passive/mikes_logs.c \
              bites/util.c \
              bites/mikes.c \
              core/config_mikes.c \
              config/config.c
TEST_X_TIM571_SRCS=tests/test_x_tim571.c \
                   modules/live/tim571.c \
                   modules/passive/x_tim571.c \
                   modules/live/gui.c \
                   modules/passive/mikes_logs.c \
                   bites/util.c \
                   bites/mikes.c \
                   core/config_mikes.c \
                   config/config.c
TEST_X_BASE_SRCS=tests/test_x_base.c \
                   modules/live/base_module.c \
                   modules/passive/pose.c \
                   modules/passive/x_base.c \
                   modules/live/gui.c \
                   modules/passive/mikes_logs.c \
                   bites/util.c \
                   bites/mikes.c \
                   core/config_mikes.c \
                   config/config.c
TEST_RFID_SRCS=tests/test_rfid.c \
                   modules/live/rfid_sensor.c \
                   modules/passive/mikes_logs.c \
                   bites/util.c \
                   bites/mikes.c \
                   core/config_mikes.c \
                   config/config.c
TEST_UST10LX_SRCS=tests/test_ust10lx.c \
                 modules/live/ust10lx.c \
                 modules/passive/mikes_logs.c \
                 bites/mikes.c \
                 core/config_mikes.c \
                 bites/util.c \
                 config/config.c
TEST_X_UST10LX_SRCS=tests/test_x_ust10lx.c \
                   modules/live/ust10lx.c \
                   modules/passive/x_ust10lx.c \
                   modules/live/gui.c \
                   modules/passive/mikes_logs.c \
                   bites/util.c \
                   bites/mikes.c \
                   core/config_mikes.c \
                   config/config.c
TEST_LIDAR_SRCS=tests/test_rplidar.c \
                 modules/passive/mikes_logs.c \
                 bites/mikes.c \
                 core/config_mikes.c \
                 bites/util.c \
                 config/config.c
TEST_LIDAR_CPPSRCS=modules/live/lidar.cpp
TEST_X_LIDAR_SRCS=tests/test_x_rplidar.c \
                   modules/passive/x_lidar.c \
                   modules/live/gui.c \
                   modules/passive/mikes_logs.c \
                   bites/util.c \
                   bites/mikes.c \
                   core/config_mikes.c \
                   config/config.c
TEST_X_LIDAR_CPPSRCS=modules/live/lidar.cpp

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

TEST_CPPSRCS=
TEST_CPPOBJS=${TEST_CPPSRCS:.cpp=.o}

#OPTIMIZE=-O0 ${SAN}
OPTIMIZE=-O0 
DEBUG_FLAGS=-g
CFLAGS=${OPTIMIZE} -std=c11 -D_BSD_SOURCE -D_XOPEN_SOURCE=600 -I. -I/usr/include/cairo -I/usr/local/rplidar/sdk/sdk/include -I/usr/include/librsvg-2.0/librsvg -I/usr/include/glib-2.0 -I/usr/lib/arm-linux-gnueabihf/glib-2.0/include -I/usr/include/libxml2 -I/usr/include/gdk-pixbuf-2.0 -Wall
CPPFLAGS=${OPTIMIZE} ${DEBUG_FLAGS} -D_BSD_SOURCE -D_XOPEN_SOURCE=600 -I/usr/include/cairo -I/usr/local/rplidar/sdk/sdk/include -Wall -Wno-write-strings -I/usr/include/librsvg-2.0/librsvg -I/usr/include/glib-2.0 -I/usr/lib/arm-linux-gnueabihf/glib-2.0/include -I/usr/include/gdk-pixbuf-2.0 
#LDFLAGS=${DEBUG_FLAGS} -pthread -lrt -lcairo -lX11 -lm -lncurses -L/usr/local/rplidar/sdk/output/Linux/Release -lrplidar_sdk -lrsvg-2 -lxml2 -g -lstdc++ ${SAN} -lubsan
LDFLAGS=${DEBUG_FLAGS} -pthread -lrt -lcairo -lX11 -lm -lncurses -L/usr/local/rplidar/sdk/output/Linux/Release -lrplidar_sdk -lrsvg-2 -lxml2 -g -lstdc++ 
PREFIX=/usr/local

all: test

# ${OBJS} ${CPPOBJS} 
#	${CPP} ${OBJS} ${CPPOBJS} -o ${PROG} ${CFLAGS} ${LDFLAGS}

install:

test:	test_pq test_astar test_pose test_base test_ncurses_control test_tim571 test_gui test_x_tim571 test_x_base test_rfid test_ust10lx test_x_ust10lx test_rplidar test_x_rplidar

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
     
uninstall:

clean:
	rm -f *.o */*.o */*/*.o test_pq test_astar test_pose test_base test_ncurses_control test_tim571 test_gui test_x_tim571 test_x_base test_rfid test_ust10lx test_x_ust10lx test_rplidar test_x_rplidar

