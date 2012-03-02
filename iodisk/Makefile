LDEBUG = y

progs = iodisk

iodisk_src = iodisk.c
local_cflags = -I. -D_GNU_SOURCE
local_ldflags = -lrt -lm

ifeq ($(LDEBUG), y)
    local_cflags += -D_IODISK_DEBUG_ -g
endif

need_post = y
define post_cmd
	$(Q) sudo chown root:root $(1)
	$(Q) sudo chmod +s $(1)
endef


include rules.mk
