CC=gcc
#FLAGS=-Wall -I $(INC) -g
FLAGS=-Wall -I $(INC)
RM=rm
UMOUNT=fusermount -u
FUSE_VER=26
CP=cp
DOCUMENT=doxygen > /dev/null
COMMIT=svn commit

# directories
BIN=bin
INC=inc
SRC=src
TST=test
DOC=doc

# objects list
obj_fuse=$(SRC)/fuse_main.c $(SRC)/fuse_implementation.c
obj_audio=$(SRC)/mp3_mpeg.c $(SRC)/mp3_id3.c $(SRC)/mp3.c $(SRC)/asf.c $(SRC)/names.c
obj_db5=$(SRC)/db5.c $(SRC)/db5_dat.c $(SRC)/db5_hdr.c $(SRC)/db5_idx.c $(SRC)/names.c
obj_common=$(SRC)/crc32.c $(SRC)/wstring.c $(SRC)/file.c $(SRC)/utf8.c $(SRC)/logger.c
obj_fsck=$(SRC)/fsck.c

.PHONY: build test install
build: db5fuse fsck
test: db5 mp3 asf names utf8 crc32 log truncate

.PHONY: db5 mp3 asf db5fuse fsck utf8 truncate
db5: $(BIN)/db5
mp3: $(BIN)/mp3
asf: $(BIN)/asf 
names: $(BIN)/names 
utf8: $(BIN)/utf8
db5fuse: $(BIN)/db5fuse
crc32: $(BIN)/crc32
fsck: $(BIN)/fsck.db5
log: $(BIN)/log
truncate: $(BIN)/truncate

install: $(BIN)/db5fuse
	$(CP) $(BIN)/db5fuse /usr/bin

$(BIN)/db5fuse: $(obj_common) $(obj_db5) $(obj_audio) $(obj_fuse)
	$(CC) -o $@ $(FLAGS) $^ -lfuse -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=$(FUSE_VER) -lid3tag

$(BIN)/fsck.db5: $(obj_common) $(obj_db5) $(obj_audio) $(obj_fsck)
	$(CC) -o $@ $(FLAGS) $^ -lid3tag

$(BIN)/db5: $(obj_common) $(obj_db5) $(obj_audio) $(TST)/test_db5.c
	$(CC) -o $@ $(FLAGS) $^ -lid3tag

$(BIN)/mp3: $(obj_common) $(obj_db5) $(obj_audio) $(TST)/test_mp3.c
	$(CC) -o $@ $(FLAGS) $^ -lid3tag

$(BIN)/asf: $(obj_common) $(obj_db5) $(obj_audio) $(TST)/test_asf.c
	$(CC) -o $@ $(FLAGS) $^ -lid3tag

$(BIN)/names: $(obj_common) $(obj_db5) $(obj_audio) $(TST)/test_names.c
	$(CC) -o $@ $(FLAGS) $^ -lid3tag

$(BIN)/utf8: $(obj_common) $(TST)/test_utf8.c
	$(CC) -o $@ $(FLAGS) $^

$(BIN)/crc32: $(obj_common) $(TST)/test_crc32.c
	$(CC) -o $@ $(FLAGS) $^

$(BIN)/log: $(obj_common) $(TST)/test_log.c
	$(CC) -o $@ $(FLAGS) $^

$(BIN)/truncate: $(TST)/truncate.c
	$(CC) -o $@ $(FLAGS) $^

.PHONY: clean
clean:
	-@$(RM) $(BIN)/*

.PHONY: doc
doc: 
	$(DOCUMENT)

.PHONY: commit
commit:
	$(COMMIT) .
