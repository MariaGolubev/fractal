NAME = fractal

CC = gcc

CFLAGS_DEBUG = `pkg-config --cflags gtk4 libadwaita-1 epoxy  gtk4-layer-shell-0` -Wall -Wextra -Werror -O0 -g
CFLAGS_RELEASE = `pkg-config --cflags gtk4 libadwaita-1 epoxy  gtk4-layer-shell-0` -O3 -DNDEBUG

LDFLAGS = `pkg-config --libs gtk4 libadwaita-1 epoxy  gtk4-layer-shell-0` -lm

INCLUDE = include
HEADERSFILE = vertex_shader.h fragment_shader.h

HEADERS = $(HEADERSFILE:%=$(INCLUDE)/%)

SRCDIR = src
OBJDIR = obj

SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: release

release: CFLAGS += $(CFLAGS_RELEASE)
release: $(NAME)

debug: CFLAGS += $(CFLAGS_DEBUG)
debug: $(NAME)

$(NAME): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(OBJDIR)
	$(CC) -I$(INCLUDE) -c $< -o $@ $(CFLAGS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(INCLUDE)/vertex_shader.h: $(SRCDIR)/vertex_shader.glsl | $(INCLUDE)
	xxd -i $< > $@

$(INCLUDE)/fragment_shader.h: $(SRCDIR)/fragment_shader.glsl | $(INCLUDE)
	xxd -i $< > $@

$(INCLUDE):
	mkdir -p $(INCLUDE)

clean:
	rm -rf $(OBJDIR) $(INCLUDE)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re release debug
