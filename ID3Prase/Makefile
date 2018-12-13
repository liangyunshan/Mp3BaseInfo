MODULE = oam
cc = g++
SOURCES = $(wildcard *.cpp)
SOURCES += $(wildcard *.c)
OBJS = $(SOURCES:%.c=%.o)
OBJ_DIR = .obj
CFLAGS = -I./ -DLINUX
all:${MODULE}
${MODULE} : ${OBJS}
	${cc} -DHAVE_CONFIG_H ${CFLAGS} $^ -o $@
	test -d ${OBJ_DIR} || mkdir -p ${OBJ_DIR}
	#mv ${OBJS} ${OBJ_DIR}

.PHONY : clean
clean:
	rm -rf $(addprefix ${OBJ_DIR}/, ${OBJS}



