#pragma once
#include <vector>

class HudGL {
public:
	HudGL();
	~HudGL();

	void color(float r, float g, float b, float a) const;
	void color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) const;
	void line_width(float width) const;
	void line(const Legacy_Vector2D& start, const Legacy_Vector2D& end) const;
	void circle(const Legacy_Vector2D& center, const std::vector<Legacy_Vector2D>& points) const;
	void rectangle(const Legacy_Vector2D& corner_a, const Legacy_Vector2D& corner_b) const;

	static std::vector<Legacy_Vector2D> compute_circle(float radius);
};
