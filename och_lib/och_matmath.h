#pragma once
#pragma once

#include <cmath>

namespace och
{
	struct mat2
	{
		float f[4] alignas(8);

		mat2() = default;

		constexpr mat2(float all) noexcept : f{ all, all, all, all } {}

		constexpr mat2(float xx, float xy, float yx, float yy) noexcept : f{ xx, xy, yx, yy } {}

		constexpr float& operator()(size_t i) noexcept { return f[i]; }

		constexpr float operator()(size_t i) const noexcept { return f[i]; }

		constexpr float& operator()(size_t x, size_t y) noexcept { return f[x + 2 * y]; }

		constexpr float operator()(size_t x, size_t y) const noexcept { return f[x + 2 * y]; }

		constexpr static mat2 identity() noexcept
		{
			return
			{
				1.0F, 0.0F,
				0.0F, 1.0F,
			};
		}
	};

	struct vec2
	{
		float f[2] alignas(8);

		vec2() = default;

		constexpr vec2(float all) : f{ all, all } {}

		constexpr vec2(float x, float y) noexcept : f{ x, y } {}

		float& operator()(size_t i) noexcept { return f[i]; }

		float operator()(size_t i) const noexcept { return f[i]; }
	};

	struct mat3
	{
		float f[9];

		mat3() = default;

		constexpr mat3(float all) noexcept : f{ all, all, all, all, all, all, all, all, all } {}

		constexpr mat3(float xx, float xy, float xz, float yx, float yy, float yz, float zx, float zy, float zz) noexcept : f{ xx, xy, xz, yx, yy, yz, zx, zy, zz } {}

		constexpr float& operator()(size_t i) noexcept { return f[i]; }

		constexpr float operator()(size_t i) const noexcept { return f[i]; }

		constexpr float& operator()(size_t x, size_t y) noexcept { return f[x + 4 * y]; }

		constexpr float operator()(size_t x, size_t y) const noexcept { return f[x + 4 * y]; }

		constexpr static mat3 identity() noexcept
		{
			return
			{
				1.0F, 0.0F, 0.0F,
				0.0F, 1.0F, 0.0F,
				0.0F, 0.0F, 1.0F,
			};
		}
	};

	struct vec3
	{
		float f[3] alignas(16);

		vec3() = default;

		constexpr vec3(float all) : f{ all, all, all } {}

		constexpr vec3(float x, float y, float z) noexcept : f{ x, y, z } {}

		float& operator()(size_t i) noexcept { return f[i]; }

		float operator()(size_t i) const noexcept { return f[i]; }
	};

	struct mat4
	{
		float f[16] alignas(16);

		mat4() = default;

		constexpr mat4(float all) noexcept : f{ all, all, all, all, all, all, all, all, all, all, all, all, all, all, all, all } {}

		constexpr mat4(float xx, float xy, float xz, float xw, float yx, float yy, float yz, float yw, float zx, float zy, float zz, float zw, float wx, float wy, float wz, float ww) noexcept
			: f{ xx, xy, xz, xw, yx, yy, yz, yw, zx, zy, zz, zw, wx, wy, wz, ww } {}

		constexpr float& operator()(size_t i) noexcept { return f[i]; }

		constexpr float operator()(size_t i) const noexcept { return f[i]; }

		constexpr float& operator()(size_t x, size_t y) noexcept { return f[4 * x + y]; }

		constexpr float operator()(size_t x, size_t y) const noexcept { return f[4 * x + y]; }

		constexpr static mat4 identity() noexcept
		{
			return
			{
				1.0F, 0.0F, 0.0F, 0.0F,
				0.0F, 1.0F, 0.0F, 0.0F,
				0.0F, 0.0F, 1.0F, 0.0F,
				0.0F, 0.0F, 0.0F, 1.0F
			};
		}

		constexpr static mat4 scale(float sx, float sy, float sz) noexcept
		{
			return
			{
				  sx, 0.0F, 0.0F, 0.0F,
				0.0F,   sy, 0.0F, 0.0F,
				0.0F, 0.0F,   sz, 0.0F,
				0.0F, 0.0F, 0.0F, 1.0F
			};
		}

		constexpr static mat4 translate(float tx, float ty, float tz) noexcept
		{
			return
			{
				1.0F, 0.0F, 0.0F, 0.0F,
				0.0F, 1.0F, 0.0F, 0.0F,
				0.0F, 0.0F, 1.0F, 0.0F,
				  tx,   ty,   tz, 1.0F
			};
		}

		constexpr static mat4 translate(const vec3& t)
		{
			return translate(t.f[0], t.f[1], t.f[2]);
		}

		static mat4 rotate_x(float angle) noexcept
		{
			float cv = cosf(angle);

			float sv = sinf(angle);

			return
			{
				1.0F, 0.0F, 0.0F, 0.0F,
				0.0F,   cv,   sv, 0.0F,
				0.0F,  -sv,   cv, 0.0F,
				0.0F, 0.0F, 0.0F, 1.0F,
			};
		}

		static mat4 rotate_y(float angle) noexcept
		{
			float cv = cosf(angle);

			float sv = sinf(angle);

			return
			{
				  cv, 0.0F,  -sv, 0.0F,
				0.0F, 1.0F, 0.0F, 0.0F,
				  sv, 0.0F,   cv, 0.0F,
				0.0F, 0.0F, 0.0F, 1.0F
			};
		}

		static mat4 rotate_z(float angle) noexcept
		{
			float cv = cosf(angle);

			float sv = sinf(angle);

			return
			{
				  cv,   sv, 0.0F, 0.0F,
				 -sv,   cv, 0.0F, 0.0F,
				0.0F, 0.0F, 1.0F, 0.0F,
				0.0F, 0.0F, 0.0F, 1.0F
			};
		}

		constexpr static mat4 mirror_yz() noexcept
		{
			return
			{
				-1.0F, 0.0F, 0.0F, 0.0F,
				 0.0F, 1.0F, 0.0F, 0.0F,
				 0.0F, 0.0F, 1.0F, 0.0F,
				 0.0F, 0.0F, 0.0F, 1.0F
			};
		}

		constexpr static mat4 mirror_xz() noexcept
		{
			return
			{
				1.0F,  0.0F, 0.0F, 0.0F,
				0.0F, -1.0F, 0.0F, 0.0F,
				0.0F,  0.0F, 1.0F, 0.0F,
				0.0F,  0.0F, 0.0F, 1.0F
			};
		}

		constexpr static mat4 mirror_xy() noexcept
		{
			return
			{
				1.0F, 0.0F,  0.0F, 0.0F,
				0.0F, 1.0F,  0.0F, 0.0F,
				0.0F, 0.0F, -1.0F, 0.0F,
				0.0F, 0.0F,  0.0F, 1.0F
			};
		}
	};

	constexpr mat2 transpose(const mat2& m)
	{
		return
		{
			m(0, 0), m(1, 0),
			m(0, 1), m(1, 1),
		};
	}

	constexpr mat3 transpose(const mat3& m)
	{
		return
		{
			m(0, 0), m(1, 0), m(2, 0),
			m(0, 1), m(1, 1), m(2, 1),
			m(0, 2), m(1, 2), m(2, 2),
		};
	}

	constexpr mat4 transpose(const mat4& m)
	{
		return
		{
			m(0, 0), m(1, 0), m(2, 0), m(3, 0),
			m(0, 1), m(1, 1), m(2, 1), m(3, 1),
			m(0, 2), m(1, 2), m(2, 2), m(3, 2),
			m(0, 3), m(1, 3), m(2, 3), m(3, 3),
		};
	}

	struct vec4
	{
		float f[4] alignas(16);

		vec4() = default;

		constexpr vec4(float all) : f{ all, all, all, all } {}

		constexpr vec4(float x, float y, float z, float w) noexcept : f{ x, y, z, w } {}

		constexpr float& operator()(size_t i) noexcept { return f[i]; }

		constexpr float operator()(size_t i) const noexcept { return f[i]; }
	};
	
	mat4 operator*(const mat4& l, const mat4& r)
	{
		mat4 product;

		product(0, 0) = l(0, 0) * r(0, 0) + l(1, 0) * r(0, 1) + l(2, 0) * r(0, 2) + l(3, 0) * r(0, 3);
		product(1, 0) = l(0, 0) * r(1, 0) + l(1, 0) * r(1, 1) + l(2, 0) * r(1, 2) + l(3, 0) * r(1, 3);
		product(2, 0) = l(0, 0) * r(2, 0) + l(1, 0) * r(2, 1) + l(2, 0) * r(2, 2) + l(3, 0) * r(2, 3);
		product(3, 0) = l(0, 0) * r(3, 0) + l(1, 0) * r(3, 1) + l(2, 0) * r(3, 2) + l(3, 0) * r(3, 3);

		product(0, 1) = l(0, 1) * r(0, 0) + l(1, 1) * r(0, 1) + l(2, 1) * r(0, 2) + l(3, 1) * r(0, 3);
		product(1, 1) = l(0, 1) * r(1, 0) + l(1, 1) * r(1, 1) + l(2, 1) * r(1, 2) + l(3, 1) * r(1, 3);
		product(2, 1) = l(0, 1) * r(2, 0) + l(1, 1) * r(2, 1) + l(2, 1) * r(2, 2) + l(3, 1) * r(2, 3);
		product(3, 1) = l(0, 1) * r(3, 0) + l(1, 1) * r(3, 1) + l(2, 1) * r(3, 2) + l(3, 1) * r(3, 3);
		
		product(0, 2) = l(0, 2) * r(0, 0) + l(1, 2) * r(0, 1) + l(2, 2) * r(0, 2) + l(3, 2) * r(0, 3);
		product(1, 2) = l(0, 2) * r(1, 0) + l(1, 2) * r(1, 1) + l(2, 2) * r(1, 2) + l(3, 2) * r(1, 3);
		product(2, 2) = l(0, 2) * r(2, 0) + l(1, 2) * r(2, 1) + l(2, 2) * r(2, 2) + l(3, 2) * r(2, 3);
		product(3, 2) = l(0, 2) * r(3, 0) + l(1, 2) * r(3, 1) + l(2, 2) * r(3, 2) + l(3, 2) * r(3, 3);
		
		product(0, 3) = l(0, 3) * r(0, 0) + l(1, 3) * r(0, 1) + l(2, 3) * r(0, 2) + l(3, 3) * r(0, 3);
		product(1, 3) = l(0, 3) * r(1, 0) + l(1, 3) * r(1, 1) + l(2, 3) * r(1, 2) + l(3, 3) * r(1, 3);
		product(2, 3) = l(0, 3) * r(2, 0) + l(1, 3) * r(2, 1) + l(2, 3) * r(2, 2) + l(3, 3) * r(2, 3);
		product(3, 3) = l(0, 3) * r(3, 0) + l(1, 3) * r(3, 1) + l(2, 3) * r(3, 2) + l(3, 3) * r(3, 3);

		return product;
	}

	vec4 operator*(const mat4& l, const vec4& r)
	{
		vec4 product;

		product.f[0] = l.f[0] * r.f[0] + l.f[1] * r.f[1] + l.f[2] * r.f[2] + l.f[3] * r.f[3];

		product.f[1] = l.f[4] * r.f[0] + l.f[5] * r.f[1] + l.f[6] * r.f[2] + l.f[7] * r.f[3];

		product.f[2] = l.f[8] * r.f[0] + l.f[9] * r.f[1] + l.f[10] * r.f[2] + l.f[11] * r.f[3];

		product.f[3] = l.f[12] * r.f[0] + l.f[13] * r.f[1] + l.f[14] * r.f[2] + l.f[15] * r.f[3];

		return product;
	}



	vec4 operator+(const vec4& l, const vec4& r) noexcept
	{
		return
		{
			l.f[0] + r.f[0],
			l.f[1] + r.f[1],
			l.f[2] + r.f[2],
			l.f[3] + r.f[3]
		};
	}

	vec4 operator-(const vec4& l, const vec4& r) noexcept
	{
		return
		{
			l.f[0] - r.f[0],
			l.f[1] - r.f[1],
			l.f[2] - r.f[2],
			l.f[3] - r.f[3]
		};
	}

	vec4 operator-(const vec4& v)
	{
		return
		{
			-v.f[0],
			-v.f[1],
			-v.f[2],
			-v.f[3]
		};
	}

	vec4& operator+=(vec4& l, vec4& r) noexcept
	{
		l.f[0] += r.f[0];
		l.f[1] += r.f[1];
		l.f[2] += r.f[2];
		l.f[3] += r.f[3];

		return l;
	}

	vec4& operator-=(vec4& l, vec4& r) noexcept
	{
		l.f[0] -= r.f[0];
		l.f[1] -= r.f[1];
		l.f[2] -= r.f[2];
		l.f[3] -= r.f[3];

		return l;
	}

	float dot(const vec4& l, const vec4& r) noexcept
	{
		return
			l.f[0] * r.f[0] +
			l.f[1] * r.f[1] +
			l.f[2] * r.f[2] +
			l.f[3] * r.f[3];
	}

	float magnitude(const vec4& v) noexcept
	{
		return sqrtf(v.f[0] * v.f[0] + v.f[1] * v.f[1] + v.f[2] * v.f[2] + v.f[3] * v.f[3]);
	}

	vec4 normalize(const vec4& v) noexcept
	{
		float inv_magnitude = 1.0F / magnitude(v);

		return
		{
			v.f[0] * inv_magnitude,
			v.f[1] * inv_magnitude,
			v.f[2] * inv_magnitude,
			v.f[3] * inv_magnitude
		};
	}



	vec3 operator+(const vec3& l, const vec3& r) noexcept
	{
		return
		{
			l.f[0] + r.f[0],
			l.f[1] + r.f[1],
			l.f[2] + r.f[2]
		};
	}

	vec3 operator-(const vec3& l, const vec3& r) noexcept
	{
		return
		{
			l.f[0] - r.f[0],
			l.f[1] - r.f[1],
			l.f[2] - r.f[2]
		};
	}

	vec3 operator-(const vec3& v)
	{
		return
		{
			-v.f[0],
			-v.f[1],
			-v.f[2]
		};
	}

	vec3& operator+=(vec3& l, vec3& r) noexcept
	{
		l.f[0] += r.f[0];
		l.f[1] += r.f[1];
		l.f[2] += r.f[2];

		return l;
	}

	vec3& operator-=(vec3& l, vec3& r) noexcept
	{
		l.f[0] -= r.f[0];
		l.f[1] -= r.f[1];
		l.f[2] -= r.f[2];

		return l;
	}

	float dot(const vec3& l, const vec3& r) noexcept
	{
		return
			l.f[0] * r.f[0] +
			l.f[1] * r.f[1] +
			l.f[2] * r.f[2];
	}

	float magnitude(const vec3& v) noexcept
	{
		return sqrtf(v.f[0] * v.f[0] + v.f[1] * v.f[1] + v.f[2] * v.f[2]);
	}

	vec3 normalize(const vec3& v) noexcept
	{
		float inv_magnitude = 1.0F / magnitude(v);

		return
		{
			v.f[0] * inv_magnitude,
			v.f[1] * inv_magnitude,
			v.f[2] * inv_magnitude
		};
	}

	vec3 cross(const vec3& l, const vec3& r)
	{
		return
		{
			l.f[1] * r.f[2] - l.f[2] * r.f[1],
			l.f[2] * r.f[0] - l.f[0] * r.f[2],
			l.f[0] * r.f[1] - l.f[1] * r.f[0]
		};
	}






	mat4 perspective(float vert_fov, float aspect_ratio, float n, float f) noexcept
	{
		float rad = vert_fov;
		float tanHalfFovy = tan(rad / 2.0F);

		mat4 rst(0.0F);

		rst(0, 0) = 1.0F / (aspect_ratio * tanHalfFovy);
		rst(1, 1) = -1.0F / tanHalfFovy;
		rst(2, 2) = f / (n - f);
		rst(2, 3) = -1.0F;
		rst(3, 2) = -(1.0F * f * n) / (f - n);

		return rst;
	}

	mat4 look_at(vec3 eye_pos, vec3 center_pos, vec3 up)
	{
		// View direction
		vec3 z = normalize(center_pos - eye_pos);

		vec3 x = normalize(cross(z, up));

		vec3 y = normalize(cross(x, z));

		mat4 m
		{
			x(0), y(0), -z(0), 0.0F,
			x(1), y(1), -z(1), 0.0F,
			x(2), y(2), -z(2), 0.0F,
			0.0F, 0.0F,  0.0F, 1.0F
		};

		return m * mat4::translate(-eye_pos);
	}
}
