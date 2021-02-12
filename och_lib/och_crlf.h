#pragma once

#include "och_memrun.h"

namespace och
{
	och::range<char> lf_to_cr(och::stringview src, och::range<char> dst)
	{
		char* w = dst.beg;

		for (const char* r = src.beg; r != src.end && w != dst.end; ++r, w++)
			*w = *r == '\n' ? '\r' : *r;

		return{ dst.beg, w };
	}

	och::range<char> lf_to_crlf(och::stringview src, och::range<char> dst)
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

	och::range<char> cr_to_lf(och::stringview src, och::range<char> dst)
	{
		char* w = dst.beg;

		for (const char* r = src.beg; r != src.end && w != dst.end; ++r, w++)
			*w = *r == '\r' ? '\n' : *r;

		return{ dst.beg, w };
	}

	och::range<char> cr_to_crlf(och::stringview src, och::range<char> dst)
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

	och::range<char> crlf_to_lf(och::stringview src, och::range<char> dst)
	{
		char* w = dst.beg;


		for (const char* r = src.beg; r != src.end && w != dst.end; ++r)
		{
			if (char c = *r; c != '\r')
				*(w++) = c;
		}

		return{ dst.beg, w };
	}

	och::range<char> crlf_to_cr(och::stringview src, och::range<char> dst)
	{
		char* w = dst.beg;


		for (const char* r = src.beg; r != src.end && w != dst.end; ++r)
		{
			if (char c = *r; c != '\n')
				*(w++) = c;
		}

		return{ dst.beg, w };
	}



	och::range<char> lf_to_cr(och::range<char> src, och::range<char> dst)
	{
		return lf_to_cr(och::stringview(src.beg, src.end), dst);
	}

	och::range<char> lf_to_crlf(och::range<char> src, och::range<char> dst)
	{
		return lf_to_crlf(och::stringview(src.beg, src.end), dst);
	}

	och::range<char> cr_to_lf(och::range<char> src, och::range<char> dst)
	{
		return cr_to_lf(och::stringview(src.beg, src.end), dst);
	}

	och::range<char> cr_to_crlf(och::range<char> src, och::range<char> dst)
	{
		return cr_to_crlf(och::stringview(src.beg, src.end), dst);
	}

	och::range<char> crlf_to_lf(och::range<char> src, och::range<char> dst)
	{
		return crlf_to_lf(och::stringview(src.beg, src.end), dst);
	}

	och::range<char> crlf_to_cr(och::range<char> src, och::range<char> dst)
	{
		return crlf_to_cr(och::stringview(src.beg, src.end), dst);
	}
}
