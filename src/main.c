/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.c                                             :+:    :+:            */
/*                                                     +:+                    */
/*   By: mgolubev <mgolubev@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/24 20:33:50 by mgolubev      #+#    #+#                 */
/*   Updated: 2025/01/14 18:10:26 by maria         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <gtk/gtk.h>
#include <adwaita.h>
#include <gtk4-layer-shell.h>
#include <epoxy/gl.h>
#include <sys/time.h>

#include "fragment_shader.h"
#include "vertex_shader.h"

#define CSS ".meoww {border: 3px solid @accent_color; border-radius: 10px;}"

#ifndef M_PI
# define M_PI 3.14159265358979323846
#endif

gboolean	g_is_wallpaper = FALSE;

float g_points[][2] = {
		{0.4, -0.006},
		{0.4, 0.006},
		{0.285, 0.006},
		{0.285, -0.006},
		{0.285, 0.006}
	};
float g_cycle_duration = 50000.0f;
int g_num_points = sizeof(g_points) / sizeof(g_points[0]);

static void mix_complex(float a[], float b[], float t, float result[])
{
	result[0] = a[0] + t * (b[0] - a[0]);
	result[1] = a[1] + t * (b[1] - a[1]);
}

static GdkRGBA *f_get_accent_color()
{
	AdwStyleManager *style_manager;

	style_manager = adw_style_manager_get_default();
	return (adw_style_manager_get_accent_color_rgba(style_manager));
}

static GtkWidget *f_new_header_bar(void)
{
	GtkWidget *header_bar;

	header_bar = adw_header_bar_new();
	gtk_widget_set_hexpand(header_bar, TRUE);
	gtk_widget_set_vexpand(header_bar, FALSE);
	gtk_widget_set_valign(header_bar, GTK_ALIGN_START);
	gtk_widget_add_css_class(header_bar, "flat");
	adw_header_bar_set_show_title(ADW_HEADER_BAR(header_bar), FALSE);
	return header_bar;
}

static void on_accent_color_changed(GObject *object, GParamSpec *pspec, gpointer user_data)
{
	(void)object; (void)pspec;
	gtk_gl_area_queue_render(GTK_GL_AREA(user_data));
}

static void on_gl_area_realize(GtkGLArea *gl_area, gpointer user_data)
{
	const char	*vertex_shader_source = (const char *)src_vertex_shader_glsl;
	const char	*fragment_shader_source = (const char *)src_fragment_shader_glsl;
	GLuint		shader_program;
	GLuint		vertex_shader;
	GLuint		fragment_shader;

	(void)user_data;
	gtk_gl_area_make_current(gl_area);
	if (gtk_gl_area_get_error(gl_area) != NULL) {
		g_error("Failed to create OpenGL context");
	}
	glEnable(GL_MULTISAMPLE);

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, (GLint *)(&src_vertex_shader_glsl_len));
	glCompileShader(vertex_shader);

	GLint success;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char info_log[512];
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		g_error("Vertex shader compilation failed: %s", info_log);
	}

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, (GLint *)&src_fragment_shader_glsl_len);
	glCompileShader(fragment_shader);

	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char info_log[512];
		glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
		g_error("Fragment shader compilation failed: %s", info_log);
	}

	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		char info_log[512];
		glGetProgramInfoLog(shader_program, 512, NULL, info_log);
		g_error("Shader program linking failed: %s", info_log);
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	GLuint vao, vbo;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), NULL, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	g_object_set_data(G_OBJECT(gl_area), "shader_program", GUINT_TO_POINTER(shader_program));
	g_object_set_data(G_OBJECT(gl_area), "vao", GUINT_TO_POINTER(vao));
	g_object_set_data(G_OBJECT(gl_area), "vbo", GUINT_TO_POINTER(vbo));
}

static gboolean on_gl_area_render(GtkGLArea *gl_area, GdkGLContext *context, gpointer user_data)
{
	(void)user_data; (void)context;
	GLuint shader_program = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(gl_area), "shader_program"));
	GLuint vao = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(gl_area), "vao"));

	GLint resolution_loc = glGetUniformLocation(shader_program, "resolution");
	glUseProgram(shader_program);
	glUniform2f(resolution_loc, (float)gtk_widget_get_width(GTK_WIDGET(gl_area)),
		(float)gtk_widget_get_height(GTK_WIDGET(gl_area)));

	GdkRGBA *accent_color = f_get_accent_color();
	GLint accent_color_loc = glGetUniformLocation(shader_program, "accent_color");
	glUseProgram(shader_program);
	glUniform4f(accent_color_loc, accent_color->red, accent_color->green,
		accent_color->blue, accent_color->alpha);
	g_free(accent_color);

	GLint complex_loc = glGetUniformLocation(shader_program, "complex");

	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint64_t milliseconds = tv.tv_sec * 1000 + tv.tv_usec / 1000;
	float time_in_cycle = (float)(milliseconds % (uint64_t)g_cycle_duration);
	float t = time_in_cycle / g_cycle_duration;
	t =  (1 - cos(M_PI * t)) / 2;
	float result[2];
	int current_point = (milliseconds / (int)g_cycle_duration ) % g_num_points;
	mix_complex(g_points[current_point], g_points[(current_point + 1) % g_num_points], t, result);
	glUniform2f(complex_loc, result[0], result[1]);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shader_program);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);
	glUseProgram(0);

	return TRUE;
}

static gboolean on_timeout(gpointer data) {
	GtkWidget *widget = GTK_WIDGET(data);
	if (GTK_IS_WIDGET(widget))
	{
		gtk_widget_queue_draw(widget);
		return (1);
	}
	return (0);
}

static GtkWidget *f_new_gl_area(void)
{
	GtkWidget		*gl_area;
	AdwStyleManager	*style_manager;

	gl_area = gtk_gl_area_new();
	gtk_gl_area_set_required_version(GTK_GL_AREA(gl_area), 4, 5);
	g_signal_connect(gl_area, "realize", G_CALLBACK(on_gl_area_realize), NULL);
	g_signal_connect(gl_area, "render", G_CALLBACK(on_gl_area_render), NULL);
	style_manager = adw_style_manager_get_default();
	g_signal_connect(style_manager, "notify::accent-color-rgba",
			G_CALLBACK(on_accent_color_changed), gl_area);
	g_timeout_add(40, (GSourceFunc)on_timeout, gl_area);
	return (gl_area);
}

static void window(GtkApplication *app)
{
	AdwApplicationWindow *window;
	GtkWidget *overlay;

	overlay = gtk_overlay_new();
	window = ADW_APPLICATION_WINDOW(adw_application_window_new(app));
	gtk_overlay_add_overlay(GTK_OVERLAY(overlay), f_new_header_bar());
	gtk_overlay_set_child(GTK_OVERLAY(overlay), f_new_gl_area());
	gtk_window_set_title(GTK_WINDOW(window), "Hello Codam!");
	gtk_window_set_default_size(GTK_WINDOW(window), 1080, 920);
	adw_application_window_set_content(window, overlay);
	gtk_window_present(GTK_WINDOW(window));
}

static void wallpaper(GtkApplication *app)
{
	AdwApplicationWindow *window;

	window = ADW_APPLICATION_WINDOW(adw_application_window_new(app));

	gtk_layer_init_for_window(GTK_WINDOW(window));
	gtk_layer_set_layer(GTK_WINDOW(window), GTK_LAYER_SHELL_LAYER_BACKGROUND);
	gtk_layer_auto_exclusive_zone_enable(GTK_WINDOW(window));
	gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_LEFT, TRUE);
	gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_RIGHT, TRUE);
	gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_TOP, TRUE);
	gtk_layer_set_anchor(GTK_WINDOW(window), GTK_LAYER_SHELL_EDGE_BOTTOM, TRUE);


	gtk_window_set_title(GTK_WINDOW(window), "Hello Codam!");
	adw_application_window_set_content(window, f_new_gl_area());
	gtk_window_present(GTK_WINDOW(window));
}

static void on_activate(GtkApplication *app, gpointer user_data)
{
	(void)user_data;

	if (g_is_wallpaper)
		wallpaper(app);
	else
		window(app);
}

static gint	on_handle_local_options(GApplication *app, GVariantDict *options, gpointer user_data) {
	(void)app;
	(void)user_data;
	if (g_variant_dict_lookup(options, "wallpaper", "b", NULL))
		g_is_wallpaper = TRUE;
	return (-1);
}

int main(int argc, char *argv[])
{
	AdwApplication *app;
	int status;

	adw_init();
	app = adw_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
	g_application_add_main_option(G_APPLICATION(app), "wallpaper", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, "Set wallpaper mode", NULL);
	g_signal_connect(app, "handle-local-options", G_CALLBACK(on_handle_local_options), NULL);
	g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	return status;
}
