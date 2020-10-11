#define OCH_RTV

#include "och_rtv.h"
#include "och_fio.h"
#include "och_fmt.h"
#include "och_memrun.h"

#ifdef OCH_RTV

namespace och
{
	constexpr size_t max_rtv_cnt = 256;

	enum class rtv_type : uint8_t
	{
		f32,
		f64,
		u8,
		u16,
		u32,
		u64,
		i8,
		i16,
		i32,
		i64
	};

	struct rtv_data
	{
		int registered_var_cnt = 0;

		och::string config_filename;

		void* pointers[och::max_rtv_cnt];

		rtv_type types[och::max_rtv_cnt];

		och::string names[och::max_rtv_cnt];
	};

	rtv_data rtv;

	void rtv_update()
	{
		if (rtv.registered_var_cnt == 0)
			return;

		och::filehandle file(rtv.config_filename, och::fio::access_read, och::fio::open_normal, och::fio::open_fail);

		if(!file.handle)
		{
			och::print("\nCould not open config iohandle for runtime-variables\n");
			return;
		}

		//for (int i = 0; i < rtv.registered_var_cnt; ++i)
		//{
		//	char buf[128];
		//
		//	if (!fgets(buf, sizeof buf, file))
		//	{
		//		och::print("\nCould not read from runtime-configuration-iohandle\n");
		//		return;
		//	}
		//
		//	int c = 0;
		//
		//	while (buf[c] != ':' && c != sizeof buf - 1)
		//		++c;
		//
		//	c += 2;
		//
		//	switch (rtv.types[i])
		//	{
		//	case rtv_type::f32: *reinterpret_cast<float*>(rtv.pointers[i]) = strtof(buf + c, nullptr); break;
		//	case rtv_type::f64: *reinterpret_cast<double*>(rtv.pointers[i]) = strtod(buf + c, nullptr); break;
		//	case rtv_type::u8: *reinterpret_cast<uint8_t*>(rtv.pointers[i]) = atoi(buf + c); break;
		//	case rtv_type::u16: *reinterpret_cast<uint16_t*>(rtv.pointers[i]) = atoi(buf + c); break;
		//	case rtv_type::u32: *reinterpret_cast<uint32_t*>(rtv.pointers[i]) = atoi(buf + c); break;
		//	case rtv_type::u64: *reinterpret_cast<uint64_t*>(rtv.pointers[i]) = atoi(buf + c); break;
		//	case rtv_type::i8: *reinterpret_cast<int8_t*>(rtv.pointers[i]) = atoi(buf + c); break;
		//	case rtv_type::i16: *reinterpret_cast<int16_t*>(rtv.pointers[i]) = atoi(buf + c); break;
		//	case rtv_type::i32: *reinterpret_cast<int32_t*>(rtv.pointers[i]) = atoi(buf + c); break;
		//	case rtv_type::i64: *reinterpret_cast<int64_t*>(rtv.pointers[i]) = atoi(buf + c); break;
		//	}
		//}
	}

	void rtv_init(const char* config_filename)
	{
		if (rtv.registered_var_cnt == 0)
			return;

		FILE* file;

		rtv.config_filename = config_filename;

		if (fopen_s(&file, config_filename, "w"))
		{
			och::print("\nCould not open config iohandle for runtime-variables\n");
			return;
		}

		for (int i = 0; i < rtv.registered_var_cnt; ++i)
		{
			fputs(rtv.names[i].begin(), file);
			fputs(": ", file);

			switch (rtv.types[i])
			{
			case rtv_type::f32: fprintf(file, "%f\n", *reinterpret_cast<float*>(rtv.pointers[i])); break;

			case rtv_type::f64: fprintf(file, "%f\n", *reinterpret_cast<double*>(rtv.pointers[i])); break;

			case rtv_type::u8: fprintf(file, "%u\n", *reinterpret_cast<uint8_t*>(rtv.pointers[i])); break;

			case rtv_type::u16: fprintf(file, "%u\n", *reinterpret_cast<uint16_t*>(rtv.pointers[i])); break;

			case rtv_type::u32: fprintf(file, "%u\n", *reinterpret_cast<uint32_t*>(rtv.pointers[i])); break;

			case rtv_type::u64: fprintf(file, "%llu\n", *reinterpret_cast<uint64_t*>(rtv.pointers[i])); break;

			case rtv_type::i8: fprintf(file, "%i\n", *reinterpret_cast<int8_t*>(rtv.pointers[i])); break;

			case rtv_type::i16: fprintf(file, "%i\n", *reinterpret_cast<int16_t*>(rtv.pointers[i])); break;

			case rtv_type::i32: fprintf(file, "%i\n", *reinterpret_cast<int32_t*>(rtv.pointers[i])); break;

			case rtv_type::i64: fprintf(file, "%lli\n", *reinterpret_cast<int64_t*>(rtv.pointers[i])); break;
			}
		}

		fclose(file);
	}

	void rtv_exit()
	{
		if (och::delete_file(rtv.config_filename))
		{
			och::print("\nCould not remove rtv config-iohandle\n");
		}
	}

	void check_overflow()
	{
		if (rtv.registered_var_cnt == och::max_rtv_cnt - 1)
		{
			och::print("\nToo many runtime variables\n");
			exit(-1);
		}
	}

	namespace internal
	{
		void _register_f32(float* ptr, const char* name)
		{
			check_overflow();

			rtv.pointers[rtv.registered_var_cnt] = reinterpret_cast<void*>(ptr);

			rtv.types[rtv.registered_var_cnt] = rtv_type::f32;

			rtv.names[rtv.registered_var_cnt] = name;

			++rtv.registered_var_cnt;
		}

		void _register_f64(double* ptr, const char* name)
		{
			check_overflow();

			rtv.pointers[rtv.registered_var_cnt] = reinterpret_cast<void*>(ptr);

			rtv.types[rtv.registered_var_cnt] = rtv_type::f64;

			rtv.names[rtv.registered_var_cnt] = name;

			++rtv.registered_var_cnt;
		}

		void _register_u8(uint8_t* ptr, const char* name)
		{
			check_overflow();

			rtv.pointers[rtv.registered_var_cnt] = reinterpret_cast<void*>(ptr);

			rtv.types[rtv.registered_var_cnt] = rtv_type::u8;

			rtv.names[rtv.registered_var_cnt] = name;

			++rtv.registered_var_cnt;
		}

		void _register_u16(uint16_t* ptr, const char* name)
		{
			check_overflow();

			rtv.pointers[rtv.registered_var_cnt] = reinterpret_cast<void*>(ptr);

			rtv.types[rtv.registered_var_cnt] = rtv_type::u16;

			rtv.names[rtv.registered_var_cnt] = name;

			++rtv.registered_var_cnt;
		}

		void _register_u32(uint32_t* ptr, const char* name)
		{
			check_overflow();

			rtv.pointers[rtv.registered_var_cnt] = reinterpret_cast<void*>(ptr);

			rtv.types[rtv.registered_var_cnt] = rtv_type::u32;

			rtv.names[rtv.registered_var_cnt] = name;

			++rtv.registered_var_cnt;
		}

		void _register_u64(uint64_t* ptr, const char* name)
		{
			check_overflow();

			rtv.pointers[rtv.registered_var_cnt] = reinterpret_cast<void*>(ptr);

			rtv.types[rtv.registered_var_cnt] = rtv_type::u64;

			rtv.names[rtv.registered_var_cnt] = name;

			++rtv.registered_var_cnt;
		}

		void _register_i8(int8_t* ptr, const char* name)
		{
			check_overflow();

			rtv.pointers[rtv.registered_var_cnt] = reinterpret_cast<void*>(ptr);

			rtv.types[rtv.registered_var_cnt] = rtv_type::i8;

			rtv.names[rtv.registered_var_cnt] = name;

			++rtv.registered_var_cnt;
		}

		void _register_i16(int16_t* ptr, const char* name)
		{
			check_overflow();

			rtv.pointers[rtv.registered_var_cnt] = reinterpret_cast<void*>(ptr);

			rtv.types[rtv.registered_var_cnt] = rtv_type::i16;

			rtv.names[rtv.registered_var_cnt] = name;

			++rtv.registered_var_cnt;
		}

		void _register_i32(int32_t* ptr, const char* name)
		{
			check_overflow();

			rtv.pointers[rtv.registered_var_cnt] = reinterpret_cast<void*>(ptr);

			rtv.types[rtv.registered_var_cnt] = rtv_type::i32;

			rtv.names[rtv.registered_var_cnt] = name;

			++rtv.registered_var_cnt;
		}

		void _register_i64(int64_t* ptr, const char* name)
		{
			check_overflow();

			rtv.pointers[rtv.registered_var_cnt] = reinterpret_cast<void*>(ptr);

			rtv.types[rtv.registered_var_cnt] = rtv_type::i64;

			rtv.names[rtv.registered_var_cnt] = name;

			++rtv.registered_var_cnt;
		}
	}
}

#else

namespace och
{
	void rtv_update() {}

	void rtv_init(const char* config_filename) {}

	void rtv_exit() {}
}

#endif // OCH_RTV
