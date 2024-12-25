/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.c                                             :+:    :+:            */
/*                                                     +:+                    */
/*   By: mgolubev <mgolubev@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2024/10/24 20:33:50 by mgolubev      #+#    #+#                 */
/*   Updated: 2024/12/25 12:36:31 by maria         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <gtk/gtk.h>
#include <adwaita.h>
#include <epoxy/gl.h>

#define CSS ".meoww {border: 3px solid @accent_color; border-radius: 10px;}"

GdkRGBA *f_get_accent_color()
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

void f_set_css_provider(const char *css)
{
	GtkCssProvider *css_provider;
	GdkDisplay *display;

	css_provider = gtk_css_provider_new();
	display = gdk_display_get_default();
	gtk_css_provider_load_from_string(css_provider, css);
	gtk_style_context_add_provider_for_display(display,
		GTK_STYLE_PROVIDER(css_provider),
		GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static void on_accent_color_changed(GObject *object, GParamSpec *pspec, gpointer user_data)
{
	(void)object; (void)pspec;
	gtk_gl_area_queue_render(GTK_GL_AREA(user_data));
}

static void on_gl_area_realize(GtkGLArea *gl_area, gpointer user_data)
{
	(void)user_data;
	gtk_gl_area_make_current(gl_area);
	if (gtk_gl_area_get_error(gl_area) != NULL) {
		g_error("Failed to create OpenGL context");
	}
	glEnable(GL_MULTISAMPLE);
	GLuint shader_program;
	const char *vertex_shader_source = "\n"
		"#version 450 core\n"
		"out vec2 normCoord;\n"
		"out vec2 fragCoord;\n"
		"uniform vec2 resolution;\n"
		"void main() {\n"
		"    vec4 positions[] = {\n"
		"        vec4(-1.0, -1.0, 0.0, 1.0),\n"
		"        vec4( 3.0, -1.0, 0.0, 1.0),\n"
		"        vec4(-1.0,  3.0, 0.0, 1.0)\n"
		"    };\n"
		"    normCoord = vec2(\n"
		"        positions[gl_VertexID].x * resolution.x / resolution.y,\n"
		"        positions[gl_VertexID].y) * vec2(1.3, 1.3);\n"
		"    fragCoord = (positions[gl_VertexID].xy + 1.0) * 0.5 * resolution;\n"
		"    gl_Position = positions[gl_VertexID];\n"
		"}\n";

	const char *fragment_shader_source = "\n"
		"#version 450 core\n"
		"out vec4 fragColor;\n"
		"in vec2 normCoord;\n"
		"in vec2 fragCoord;\n"
		"uniform vec4 accent_color;\n"
		"float rand(vec2 co) {\n"
		"    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);\n"
		"}\n"
		"void main() {\n"
		"    // Нормализуем координаты пикселя в диапазон (-2, 2)\n"
		"    vec2 c = vec2(0.285, 0.01); // Параметр c для фрактала Жюлья\n"
		"    vec2 z = normCoord;\n"
		"    int max_iter = 500;\n"
		"    int iter = 0;\n"
		"    // Итерации для вычисления фрактала\n"
		"    while (length(z) < 4.0 && iter < max_iter) {\n"
		"        float xtmp = z.x * z.x - z.y * z.y;\n"
		"        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;\n"
		"        iter++;\n"
		"    }\n"
		"    float map[8][8] = {\n"
		"        {0.0, 0.5, 0.125, 0.625, 0.03125, 0.53125, 0.15625, 0.65625},\n"
		"        {0.75, 0.25, 0.875, 0.375, 0.78125, 0.28125, 0.90625, 0.40625},\n"
		"        {0.1875, 0.6875, 0.0625, 0.5625, 0.21875, 0.71875, 0.09375, 0.59375},\n"
		"        {0.9375, 0.4375, 0.8125, 0.3125, 0.96875, 0.46875, 0.84375, 0.34375},\n"
		"        {0.046875, 0.546875, 0.171875, 0.671875, 0.015625, 0.515625, 0.140625, 0.640625},\n"
		"        {0.796875, 0.296875, 0.921875, 0.421875, 0.765625, 0.265625, 0.890625, 0.390625},\n"
		"        {0.234375, 0.734375, 0.109375, 0.609375, 0.203125, 0.703125, 0.078125, 0.578125},\n"
		"        {0.984375, 0.484375, 0.859375, 0.359375, 0.953125, 0.453125, 0.828125, 0.328125}\n"
		"    };\n"
		"    // Логарифмическое вычисление интенсивности цвета\n"
		"    float color = 1.0;\n"
		"    if (iter != max_iter) {\n"
		"        float iter = iter + 1 - log(log(length(z))) / log(2);\n"
		"        color = log(float(iter)) / log(float(max_iter));\n"
		"        int x = int(mod(fragCoord.x, 8.0));\n"
		"        int y = int(mod(fragCoord.y, 8.0));\n"
		"        color += map[y][x] * 0.05;\n"
		"    }\n"
		"    // Устанавливаем цвет пикселя\n"
		"    fragColor = mix(vec4(0.0f, 0.0f, 0.0f, 1.0f), accent_color, color);\n"
		"}\n";

	GLuint vertex_shader;
	GLuint fragment_shader;

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);

	GLint success;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char info_log[512];
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		g_error("Vertex shader compilation failed: %s", info_log);
	}

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
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

GtkWidget *f_new_gl_area(void)
{
	GtkWidget *gl_area;

	gl_area = gtk_gl_area_new();
	gtk_gl_area_set_required_version(GTK_GL_AREA(gl_area), 4, 5);
	g_signal_connect(gl_area, "realize", G_CALLBACK(on_gl_area_realize), NULL);
	g_signal_connect(gl_area, "render", G_CALLBACK(on_gl_area_render), NULL);

	AdwStyleManager *style_manager = adw_style_manager_get_default();
	g_signal_connect(style_manager, "notify::accent-color-rgba", G_CALLBACK(on_accent_color_changed), gl_area);

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
	// gtk_widget_add_css_class(GTK_WIDGET(window), "meoww");
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
	// f_set_css_provider(CSS);
	app = adw_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	return status;
}
