#pragma once

#include "och_range.h"

namespace och
{
	struct destructive_tokenizer
	{
		char* text;
		const char* delimiters;

		destructive_tokenizer(char* text, const char* delimiters);

		bool is_delimiter(char c);

		char* operator()();
	};

	struct cstring_tokenizer
	{
		const char* text;
		const char* delimiters;

		cstring_tokenizer(const char* text, const char* delimiters);

		bool is_delimiter(char c);

		och::stringview operator()();
	};

	struct tokenizer
	{
		och::stringview text;
		och::stringview delimiters;

		tokenizer(och::stringview text, och::stringview delimiters);

		bool is_delimiter(char c);

		och::stringview operator()();
	};

	struct matcher
	{
		och::stringview text;
		och::stringview target;
		uint32_t target_idx = 0;
		uint32_t text_idx = 0;

		matcher(och::stringview text, och::stringview target);

		och::stringview operator()();

		uint32_t count();

		void reset();
	};

	struct multi_matcher
	{
		och::stringview text;
		och::range<const och::stringview> targets;
		uint8_t target_idx[8]{ 0, 0, 0, 0, 0, 0, 0, 0 };

		multi_matcher(och::stringview text, och::range<och::stringview> targets);

		och::stringview operator()();

		uint32_t count();
	};
}
