#pragma once

#include "och_memrun.h"

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

	struct tokenizer
	{
		const char* text;
		const char* delimiters;

		tokenizer(const char* text, const char* delimiters);

		bool is_delimiter(char c);

		och::string operator()();
	};
}
