

#DEBUG = n
CFLAGS += $(local_cflags)
LDFLAGS += $(local_ldflags)
OBJ = obj

ifeq ($(DEBUG),y)
    Q = 
else
    Q = @
endif

all:

define compile.prog
	$(Q) $(CC) $(LDFLAGS) -o $$@ $$^ -Wl,-Map,$(OBJ)/$(1).map
endef

define compile.o
	$(Q) $(CC) $(CFLAGS) -c -o $$@ $$<
endef

define object_template
$(OBJ)/$(1:%.c=%.o):$(1) $(wildcard $(1:%.c=%.h))
	$(Q) echo -e "\tCC $$<"
$(call compile.o, $@)
endef

define prog_template
$(1):$($(1)_src:%.c=$(OBJ)/%.o)
	$(Q) echo -e "\tLD $(1)"
$(call compile.prog,$(1))

$(foreach src,$($(1)_src),$(eval $(call object_template,$(src))))
$(1)_clean:
	$(Q)rm -rf $(OBJ) $(1)

clean:$(1)_clean
endef

$(foreach prog,$(progs),$(eval $(call prog_template,$(prog))))

prepare:$(OBJ)
$(OBJ):
	$(Q) mkdir -p $(OBJ)

all:prepare $(progs)
clean:
