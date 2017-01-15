#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "hudgl.h"

int CHudCrosshairs::Init()
{
	m_iFlags = HUD_ACTIVE;

	cl_cross =               CVAR_CREATE("cl_cross", "0", FCVAR_ARCHIVE);
	cl_cross_color =         CVAR_CREATE("cl_cross_color", "0 255 0", FCVAR_ARCHIVE);
	cl_cross_alpha =         CVAR_CREATE("cl_cross_alpha", "200", FCVAR_ARCHIVE);
	cl_cross_thickness =     CVAR_CREATE("cl_cross_thickness", "2", FCVAR_ARCHIVE);
	cl_cross_size =          CVAR_CREATE("cl_cross_size", "10", FCVAR_ARCHIVE);
	cl_cross_gap =           CVAR_CREATE("cl_cross_gap", "3", FCVAR_ARCHIVE);
	cl_cross_circle_radius = CVAR_CREATE("cl_cross_circle_radius", "0", FCVAR_ARCHIVE);
	cl_cross_dot_size =      CVAR_CREATE("cl_cross_dot_size", "0", FCVAR_ARCHIVE);
	
	gHUD.AddHudElem(this);
	return 0;
}

int CHudCrosshairs::VidInit()
{
	return 1;
}

int CHudCrosshairs::Draw(float time)
{
	if (cl_cross->value == 0.0f)
		return 0;

	unsigned char alpha;
	if (sscanf(cl_cross_alpha->string, "%hhu", &alpha) != 1)
		alpha = 200;

	if (alpha == 0)
		return 0;

	unsigned char r, g, b;
	if (sscanf(cl_cross_color->string, "%hhu %hhu %hhu", &r, &g, &b) != 3) {
		r = 0;
		g = 255;
		b = 0;
	}
	
	Vector2D center(ScreenWidth / 2.0f, ScreenHeight / 2.0f);

	HudGL gl;
	gl.color(r, g, b, alpha);

	// Draw the crosshairs.
	if (cl_cross_thickness->value > 0.0f) {
		gl.line_width(cl_cross_thickness->value);

		auto size = cl_cross_size->value;
		auto gap = cl_cross_gap->value;

		gl.line(Vector2D(center.x, center.y - gap - size), Vector2D(center.x, center.y - gap));
		gl.line(Vector2D(center.x, center.y + gap + size), Vector2D(center.x, center.y + gap));
		gl.line(Vector2D(center.x - gap - size, center.y), Vector2D(center.x - gap, center.y));
		gl.line(Vector2D(center.x + gap + size, center.y), Vector2D(center.x + gap, center.y));
	}

	// Draw the circle.
	if (cl_cross_circle_radius->value > 0.0f) {
		gl.line_width(1.0f);

		auto radius = cl_cross_circle_radius->value;
		if (old_circle_radius != radius) {
			// Recompute the circle points.
			circle_points = HudGL::compute_circle(radius);
			old_circle_radius = radius;
		}

		gl.circle(center, circle_points);
	}

	// Draw the dot.
	if (cl_cross_dot_size->value > 0.0f) {
		auto size = cl_cross_dot_size->value;
		auto offset = Vector2D(size / 2.0f, size / 2.0f);

		gl.rectangle(center - offset, center + offset);
	}

	return 0;
}
