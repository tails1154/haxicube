CC = cc
CFLAGS = -I.. -I../src -shared -fPIC
PLUGIN_DIR = ../plugins

MODS = coord_mod toggle_mod auto_msg

.PHONY: all clean

all: $(MODS)

$(PLUGIN_DIR):
	mkdir -p $(PLUGIN_DIR)

%: %.c $(PLUGIN_DIR)
	$(CC) $(CFLAGS) -o $(PLUGIN_DIR)/$*.so $<

clean:
	rm -f $(MODS:%=../plugins/%.so)
