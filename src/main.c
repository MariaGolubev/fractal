/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.c                                             :+:    :+:            */
/*                                                     +:+                    */
/*   By: mgolubev <mgolubev@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/24 20:33:50 by mgolubev      #+#    #+#                 */
/*   Updated: 2024/12/25 13:26:54 by maria         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <gtk/gtk.h>
#include <adwaita.h>
#include <epoxy/gl.h>

#include "fragment_shader.h"
#include "vertex_shader.h"

#define CSS ".meoww {border: 3px solid @accent_color; border-radius: 10px;}"

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

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shader_program);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);
	glUseProgram(0);

	return TRUE;
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
	return (gl_area);
}

static void on_activate(GtkApplication *app, gpointer ptr)
{
	AdwApplicationWindow *window;
	GtkWidget *overlay;

	(void)ptr;
	overlay = gtk_overlay_new();
	window = ADW_APPLICATION_WINDOW(adw_application_window_new(app));
	gtk_overlay_add_overlay(GTK_OVERLAY(overlay), f_new_header_bar());
	gtk_overlay_set_child(GTK_OVERLAY(overlay), f_new_gl_area());
	gtk_window_set_title(GTK_WINDOW(window), "Hello Codam!");
	gtk_window_set_default_size(GTK_WINDOW(window), 1080, 920);
	adw_application_window_set_content(window, overlay);
	gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[])
{
	AdwApplication *app;
	int status;

	adw_init();
	app = adw_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	return status;
}
