#pragma once

#include "och_memrun.h"

namespace och
{
	och::memrun<char> lf_to_cr(och::string src, och::memrun<char> dst)
	{
		char* w = dst.beg;

		for (const char* r = src.beg; r != src.end && w != dst.end; ++r, w++)
			*w = *r == '\n' ? '\r' : *r;

		return{ dst.beg, w };
	}

	och::memrun<char> lf_to_crlf(och::string src, och::memrun<char> dst)
	{
		char* w = dst.beg;

		for (const char* r = src.beg; r != src.end && w != dst.end; ++r)
		{
			char c = *r;

			if (c == '\n')
				*(w++) = '\r';

			if (w == dst.end)
				return { dst.beg, dst.beg };

			*(w++) = c;
		}

		return{ dst.beg, w };
	}

	och::memrun<char> cr_to_lf(och::string src, och::memrun<char> dst)
	{
		char* w = dst.beg;

		for (const char* r = src.beg; r != src.end && w != dst.end; ++r, w++)
			*w = *r == '\r' ? '\n' : *r;

		return{ dst.beg, w };
	}

	och::memrun<char> cr_to_crlf(och::string src, och::memrun<char> dst)
	{
		char* w = dst.beg;

		for (const char* r = src.beg; r != src.end && w != dst.end; ++r)
		{
			char c = *r;

			*(w++) = c;

			if (c == '\r')
				if (w == dst.end)
					return { dst.beg, dst.beg };
				else
					*(w++) = '\n';
		}

		return{ dst.beg, w };
	}

	och::memrun<char> crlf_to_lf(och::string src, och::memrun<char> dst)
	{
		char* w = dst.beg;


		for (const char* r = src.beg; r != src.end && w != dst.end; ++r)
		{
			if (char c = *r; c != '\r')
				*(w++) = c;
		}

		return{ dst.beg, w };
	}

	och::memrun<char> crlf_to_cr(och::string src, och::memrun<char> dst)
	{
		char* w = dst.beg;


		for (const char* r = src.beg; r != src.end && w != dst.end; ++r)
		{
			if (char c = *r; c != '\n')
				*(w++) = c;
		}

		return{ dst.beg, w };
	}



	och::memrun<char> lf_to_cr(och::memrun<char> src, och::memrun<char> dst)
	{
		return lf_to_cr(och::string(src.beg, src.end), dst);
	}

	och::memrun<char> lf_to_crlf(och::memrun<char> src, och::memrun<char> dst)
	{
		return lf_to_crlf(och::string(src.beg, src.end), dst);
	}

	och::memrun<char> cr_to_lf(och::memrun<char> src, och::memrun<char> dst)
	{
		return cr_to_lf(och::string(src.beg, src.end), dst);
	}

	och::memrun<char> cr_to_crlf(och::memrun<char> src, och::memrun<char> dst)
	{
		return cr_to_crlf(och::string(src.beg, src.end), dst);
	}

	och::memrun<char> crlf_to_lf(och::memrun<char> src, och::memrun<char> dst)
	{
		return crlf_to_lf(och::string(src.beg, src.end), dst);
	}

	och::memrun<char> crlf_to_cr(och::memrun<char> src, och::memrun<char> dst)
	{
		return crlf_to_cr(och::string(src.beg, src.end), dst);
	}

}
