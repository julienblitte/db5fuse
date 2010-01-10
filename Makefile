CC=gcc
#FLAGS=-Wall -I $(INC) -g
FLAGS=-Wall -I $(INC)
RM=rm
UMOUNT=fusermount -u
FUSE_VER=26
XCP=install -g 0 -o 0 -m 755
DOCUMENT=doxygen > /dev/null

# directories
BIN=bin
INC=inc
SRC=src
DOC=doc

# objects list
obj_fuse=$(SRC)/fuse_main.c $(SRC)/fuse_implementation.c
obj_audio=$(SRC)/mp3_mpeg.c $(SRC)/mp3_id3.c $(SRC)/mp3.c $(SRC)/asf.c $(SRC)/names.c
obj_db5=$(SRC)/db5.c $(SRC)/db5_dat.c $(SRC)/db5_hdr.c $(SRC)/db5_index.c $(SRC)/names.c
obj_common=$(SRC)/crc32.c $(SRC)/wstring.c $(SRC)/file.c $(SRC)/utf8.c $(SRC)/logger.c
obj_fsck=$(SRC)/fsck.c

.PHONY: build install
build: db5fuse fsck

.PHONY: db5fuse fsck
db5fuse: $(BIN)/db5fuse
fsck: $(BIN)/fsck.db5

install: $(BIN)/db5fuse $(BIN)/fsck.db5
	$(XCP) $(BIN)/db5fuse $(BIN)/fsck.db5 /usr/bin && \
	$(XCP) tools/* /usr/bin/

$(BIN)/db5fuse: $(obj_common) $(obj_db5) $(obj_audio) $(obj_fuse)
	$(CC) -o $@ $(FLAGS) $^ -lfuse -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=$(FUSE_VER) -lid3tag

$(BIN)/fsck.db5: $(obj_common) $(obj_db5) $(obj_audio) $(obj_fsck)
	$(CC) -o $@ $(FLAGS) $^ -lid3tag

.PHONY: clean
clean:
	-@$(RM) $(BIN)/*

.PHONY: doc
doc: 
	$(DOCUMENT)

