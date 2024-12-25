NAME = fractal

CC = gcc

CFLAGS_DEBUG = `pkg-config --cflags gtk4 libadwaita-1 epoxy` -Wall -Wextra -Werror -O0 -g
CFLAGS_RELEASE = `pkg-config --cflags gtk4 libadwaita-1 epoxy` -O3 -DNDEBUG

LDFLAGS = `pkg-config --libs gtk4 libadwaita-1 epoxy`

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

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re release debug
