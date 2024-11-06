################################################################################
# Makefile to make sample application
################################################################################

CC=gcc
CFLAGS=
LDFLAGS=

RM := rm -rf

LIBS := -lzip_ctl -lzip_api -lm

LIB_PATH = -L"../../lib" -L"../../src"

INCLUDE_PATH =

SRC_HEADERS = \
../../include/zip_platform.h ../../include/zip_util.h \
../../include/zip_transport.h ../../include/zip_error.h \
../../include/zip_platform.h ../../include/zip_util.h \
../../include/zip_frame.h ../../include/zip_transport.h \
../../include/zip_tls_server.h \
../../include/zip_session.h ../../include/zip_frame.h \
../../include/zip_application.h ../../include/zip_session.h \
../../include/zip_api.h ../../include/zwave/ZW_transport_api.h \
../../include/zip_nodeid.h \
../../include/zwave/ZW_classcmd.h

TARGET_BIN = hw_int_sen

SRC_OBJS := $(addsuffix .o, $(TARGET_BIN))

ifneq (,$(findstring DUSE_OPENSSL,$(CFLAGS)))
	LIBS += -lssl -lcrypto -ldl -lsqlite3
	LIB_PATH += -L"../../../openssl/install/lib"
	INCLUDE_PATH += -I../../../openssl/install/include/openssl  -I../../../openssl/install/include
endif

ifneq (,$(findstring DOS_MAC_X,$(CFLAGS)))
	LDFLAGS = -arch i386
	LIBS += -lpthread
else ifneq (,$(findstring DOS_ANDROID,$(CFLAGS)))
	LDFLAGS = -pie
	LIBS += -llog
else
	LIBS += -lpthread -lrt
endif

ifneq (,$(findstring CONFIG_DEBUG,$(CFLAGS)))
	COMPILE_OPTION = -g -O0
else
	COMPILE_OPTION = -O2
endif

LIB_FILES_NO_EXT = ../../src/libzip_ctl ../../lib/libzip_api
ifneq (,$(findstring DUSE_OPENSSL,$(CFLAGS)))
	LIB_FILES_NO_EXT += ../../../openssl/install/lib/libssl ../../../openssl/install/lib/libcrypto
endif

ifeq ($(SHARED),1)
#Link with shared libraries
ifeq (,$(findstring DOS_ANDROID,$(CFLAGS)))
	# For Android, don't add -z,now
	LDFLAGS += -Wl,-z,now
endif
	LIB_FILES := $(addsuffix .so, $(LIB_FILES_NO_EXT))
	EXEC_MSG := '*** To run with shared libraries in "$(shell pwd)" directory:\n LD_LIBRARY_PATH="../../lib:../../src:../../../openssl/install/lib" ./$(TARGET_BIN)\n'
else
#Link with static libraries
	LIB_FILES := $(addsuffix .a, $(LIB_FILES_NO_EXT))
	EXEC_MSG :=
endif

# All Target
all: $(TARGET_BIN)

# Compile c source file
%.o: %.c $(SRC_HEADERS)
	@echo 'Compiling file: $<'
	$(CC) $(COMPILE_OPTION) $(INCLUDE_PATH) -DOS_LINUX $(CFLAGS) -c -o"$@" "$<"
	@echo 'Finished compiling: $<'
	@echo ' '

# Tool invocations
$(TARGET_BIN): $(SRC_OBJS) $(LIB_FILES)
	@echo 'Building target: $@'
	$(CC) $(LIB_PATH) $(LDFLAGS) -o $(TARGET_BIN) $(SRC_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	@echo -e $(EXEC_MSG)


# Other Targets
clean:
	-$(RM) *.o
	-$(RM) $(TARGET_BIN)
	-@echo ' '

.PHONY: all clean
