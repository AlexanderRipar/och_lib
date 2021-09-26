#define OCH_MATMATH_PRESENT

#ifndef OCH_MATMATH_INCLUDE_GUARD
#define OCH_MATMATH_INCLUDE_GUARD

#include <cmath>

namespace och
{
	struct mat2
	{
		float f[4] alignas(8);

		mat2() = default;

		constexpr explicit mat2(float all) noexcept : f{ all, all, all, all } {}

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
		union
		{
			float f[2] alignas(8);

			struct { float x, y; };
		};

		vec2() = default;

		constexpr explicit vec2(float all) : f{ all, all } {}

		constexpr vec2(float x, float y) noexcept : f{ x, y } {}

		constexpr float& operator()(size_t i) noexcept { return f[i]; }

		constexpr float operator()(size_t i) const noexcept { return f[i]; }
	};

	struct mat3
	{
		float f[9];

		mat3() = default;

		constexpr explicit mat3(float all) noexcept : f{ all, all, all, all, all, all, all, all, all } {}

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
		union
		{
			float f[3] alignas(16);

			struct { float x, y, z; };
		};

		vec3() = default;

		constexpr explicit vec3(float all) : f{ all, all, all } {}

		constexpr vec3(float x, float y, float z) noexcept : f{ x, y, z } {}

		constexpr float& operator()(size_t i) noexcept { return f[i]; }

		constexpr float operator()(size_t i) const noexcept { return f[i]; }
	};

	struct mat4
	{
		float f[16] alignas(16);

		mat4() = default;

		constexpr explicit mat4(float all) noexcept : f{ all, all, all, all, all, all, all, all, all, all, all, all, all, all, all, all } {}

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
			return translate(t.x, t.y, t.z);
		}

		inline static mat4 rotate_x(float angle) noexcept
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

		inline static mat4 rotate_y(float angle) noexcept
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

		inline static mat4 rotate_z(float angle) noexcept
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

	constexpr mat2 transpose(const mat2& m) noexcept
	{
		return
		{
			m(0, 0), m(1, 0),
			m(0, 1), m(1, 1),
		};
	}

	constexpr mat3 transpose(const mat3& m) noexcept
	{
		return
		{
			m(0, 0), m(1, 0), m(2, 0),
			m(0, 1), m(1, 1), m(2, 1),
			m(0, 2), m(1, 2), m(2, 2),
		};
	}

	constexpr mat4 transpose(const mat4& m) noexcept
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
		union
		{
			float f[4] alignas(16);
			
			struct alignas(16) { float x, y, z, w; };
		};

		vec4() = default;

		constexpr explicit vec4(float all) : f{ all, all, all, all } {}

		constexpr vec4(float x, float y, float z, float w) noexcept : f{ x, y, z, w } {}

		constexpr float& operator()(size_t i) noexcept { return f[i]; }

		constexpr float operator()(size_t i) const noexcept { return f[i]; }
	};
	


	constexpr mat4 operator-(const mat4& l, const mat4& r) noexcept
	{
		och::mat4 ret;

		for (size_t i = 0; i != 16; ++i)
			ret.f[i] = l.f[i] - r.f[i];

		return ret;
	};

	constexpr mat4 operator+(const mat4& l, const mat4& r) noexcept
	{
		och::mat4 ret;

		for (size_t i = 0; i != 16; ++i)
			ret.f[i] = l.f[i] + r.f[i];

		return ret;
	};

	constexpr mat4 operator*(const mat4& l, const mat4& r) noexcept
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

	constexpr vec4 operator*(const mat4& l, const vec4& r) noexcept
	{
		const float x = l.f[0] * r.f[0] + l.f[1] * r.f[1] + l.f[2] * r.f[2] + l.f[3] * r.f[3];

		const float y = l.f[4] * r.f[0] + l.f[5] * r.f[1] + l.f[6] * r.f[2] + l.f[7] * r.f[3];

		const float z = l.f[8] * r.f[0] + l.f[9] * r.f[1] + l.f[10] * r.f[2] + l.f[11] * r.f[3];

		const float w = l.f[12] * r.f[0] + l.f[13] * r.f[1] + l.f[14] * r.f[2] + l.f[15] * r.f[3];

		return vec4(x, y, z, w);
	}



	constexpr vec4 operator+(const vec4& l, const vec4& r) noexcept
	{
		return
		{
			l.f[0] + r.f[0],
			l.f[1] + r.f[1],
			l.f[2] + r.f[2],
			l.f[3] + r.f[3]
		};
	}

	constexpr vec4 operator-(const vec4& l, const vec4& r) noexcept
	{
		return
		{
			l.f[0] - r.f[0],
			l.f[1] - r.f[1],
			l.f[2] - r.f[2],
			l.f[3] - r.f[3]
		};
	}

	constexpr vec4 operator-(const vec4& v) noexcept
	{
		return
		{
			-v.f[0],
			-v.f[1],
			-v.f[2],
			-v.f[3]
		};
	}

	constexpr vec4& operator+=(vec4& l, const vec4& r) noexcept
	{
		l.f[0] += r.f[0];
		l.f[1] += r.f[1];
		l.f[2] += r.f[2];
		l.f[3] += r.f[3];

		return l;
	}

	constexpr vec4& operator-=(vec4& l, const vec4& r) noexcept
	{
		l.f[0] -= r.f[0];
		l.f[1] -= r.f[1];
		l.f[2] -= r.f[2];
		l.f[3] -= r.f[3];

		return l;
	}

	constexpr vec4 operator*(const vec4& l, float r) noexcept
	{
		return
		{
			l.x * r,
			l.y * r,
			l.z * r,
			l.w * r
		};
	}

	constexpr vec4 operator*(float l, const vec4& r) noexcept
	{
		return
		{
			l * r.x,
			l * r.y,
			l * r.z,
			l * r.w
		};
	}

	constexpr vec4& operator*=(vec4& l, float r) noexcept
	{
		l.x *= r;
		l.y *= r;
		l.z *= r;
		l.w *= r;

		return l;
	}

	constexpr float dot(const vec4& l, const vec4& r) noexcept
	{
		return
			l.f[0] * r.f[0] +
			l.f[1] * r.f[1] +
			l.f[2] * r.f[2] +
			l.f[3] * r.f[3];
	}

	inline float magnitude(const vec4& v) noexcept
	{
		return sqrtf(v.f[0] * v.f[0] + v.f[1] * v.f[1] + v.f[2] * v.f[2] + v.f[3] * v.f[3]);
	}

	inline vec4 normalize(const vec4& v) noexcept
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





	constexpr vec3 operator*(const mat3& l, const vec3& r) noexcept
	{
		const float x = l(0, 0) * r.x + l(1, 0) * r.y + l(2, 0) + r.z;
		const float y = l(0, 1) * r.x + l(1, 1) * r.y + l(2, 1) + r.z;
		const float z = l(0, 2) * r.x + l(1, 2) * r.y + l(2, 2) + r.z;

		return vec3(x, y, z);
	}



	constexpr vec3 operator+(const vec3& l, const vec3& r) noexcept
	{
		return
		{
			l.f[0] + r.f[0],
			l.f[1] + r.f[1],
			l.f[2] + r.f[2]
		};
	}

	constexpr vec3 operator-(const vec3& l, const vec3& r) noexcept
	{
		return
		{
			l.f[0] - r.f[0],
			l.f[1] - r.f[1],
			l.f[2] - r.f[2]
		};
	}

	constexpr vec3 operator-(const vec3& v) noexcept
	{
		return
		{
			-v.f[0],
			-v.f[1],
			-v.f[2]
		};
	}

	constexpr vec3& operator+=(vec3& l, const vec3& r) noexcept
	{
		l.f[0] += r.f[0];
		l.f[1] += r.f[1];
		l.f[2] += r.f[2];

		return l;
	}

	constexpr vec3& operator-=(vec3& l, const vec3& r) noexcept
	{
		l.f[0] -= r.f[0];
		l.f[1] -= r.f[1];
		l.f[2] -= r.f[2];

		return l;
	}

	constexpr vec3 operator*(const vec3& l, float r) noexcept
	{
		return
		{
			l.x * r,
			l.y * r,
			l.z * r,
		};
	}

	constexpr vec3 operator*(float l, const vec3& r) noexcept
	{
		return
		{
			l * r.x,
			l * r.y,
			l * r.z,
		};
	}

	constexpr vec3& operator*=(vec3& l, float r) noexcept
	{
		l.x *= r;
		l.y *= r;
		l.z *= r;

		return l;
	}

	constexpr float dot(const vec3& l, const vec3& r) noexcept
	{
		return
			l.f[0] * r.f[0] +
			l.f[1] * r.f[1] +
			l.f[2] * r.f[2];
	}

	inline float magnitude(const vec3& v) noexcept
	{
		return sqrtf(v.f[0] * v.f[0] + v.f[1] * v.f[1] + v.f[2] * v.f[2]);
	}

	inline vec3 normalize(const vec3& v) noexcept
	{
		float inv_magnitude = 1.0F / magnitude(v);

		return
		{
			v.f[0] * inv_magnitude,
			v.f[1] * inv_magnitude,
			v.f[2] * inv_magnitude
		};
	}

	constexpr vec3 cross(const vec3& l, const vec3& r) noexcept
	{
		return
		{
			l.f[1] * r.f[2] - l.f[2] * r.f[1],
			l.f[2] * r.f[0] - l.f[0] * r.f[2],
			l.f[0] * r.f[1] - l.f[1] * r.f[0]
		};
	}





	constexpr vec2 operator*(const mat2& l, const vec2& r) noexcept
	{
		const float x = l(0, 0) * r.x + l(1, 0) * r.y;
		const float y = l(0, 1) * r.x + l(1, 1) * r.y;

		return vec2(x, y);
	}



	constexpr vec2 operator+(const vec2& l, const vec2& r) noexcept
	{
		return
		{
			l.f[0] + r.f[0],
			l.f[1] + r.f[1]
		};
	}

	constexpr vec2 operator-(const vec2& l, const vec2& r) noexcept
	{
		return
		{
			l.f[0] - r.f[0],
			l.f[1] - r.f[1]
		};
	}

	constexpr vec2 operator-(const vec2& v) noexcept
	{
		return
		{
			-v.f[0],
			-v.f[1]
		};
	}

	constexpr vec2& operator+=(vec2& l, const vec2& r) noexcept
	{
		l.f[0] += r.f[0];
		l.f[1] += r.f[1];

		return l;
	}

	constexpr vec2& operator-=(vec2& l, const vec2& r) noexcept
	{
		l.f[0] -= r.f[0];
		l.f[1] -= r.f[1];

		return l;
	}

	constexpr vec2 operator*(const vec2& l, float r) noexcept
	{
		return
		{
			l.x * r,
			l.y * r,
		};
	}

	constexpr vec2 operator*(float l, const vec2& r) noexcept
	{
		return
		{
			l * r.x,
			l * r.y,
		};
	}

	constexpr vec2& operator*=(vec2& l, float r) noexcept
	{
		l.x *= r;
		l.y *= r;

		return l;
	}

	constexpr float dot(const vec2& l, const vec2& r) noexcept
	{
		return
			l.f[0] * r.f[0] +
			l.f[1] * r.f[1];
	}

	constexpr float squared_magnitude(const vec2& v) noexcept
	{
		return v.x * v.x + v.y * v.y;
	}

	inline float magnitude(const vec2& v) noexcept
	{
		return sqrtf(squared_magnitude(v));
	}

	inline vec2 normalize(const vec2& v) noexcept
	{
		float inv_magnitude = 1.0F / magnitude(v);

		return
		{
			v.f[0] * inv_magnitude,
			v.f[1] * inv_magnitude,
		};
	}

	constexpr float cross(const vec2& l, const vec2& r) noexcept
	{
		return
		{
			l.x * r.y - l.y * r.x
		};
	}





	inline mat4 perspective(float vert_fov, float aspect_ratio, float n, float f) noexcept
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

	inline mat4 look_at(vec3 eye_pos, vec3 center_pos, vec3 up) noexcept
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

#endif // !OCH_MATMATH_INCLUDE_GUARD