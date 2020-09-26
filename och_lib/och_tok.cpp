#include "och_tok.h"

namespace och
{
	bool destructive_tokenizer::is_delimiter(char c)
	{
		for (int i = 0; delimiters[i]; ++i)
		{
			if (c == delimiters[i])
				return true;
		}

		return false;
	}

	destructive_tokenizer::destructive_tokenizer(char* text, const char* delimiters) : text{ text }, delimiters{ delimiters } {}

	char* destructive_tokenizer::operator()()
	{

		while (is_delimiter(*text))
			++text;

		if (!(*text))
			return nullptr;

		char* ret = text;

		while (!is_delimiter(*text))
		{
			if (!(*text))
				return ret;

			++text;
		}

		*(text++) = '\0';

		return ret;
	}



	tokenizer::tokenizer(const char* text, const char* delimiters) : text{ text }, delimiters{ delimiters } {}

	bool tokenizer::is_delimiter(char c)
	{
		for (int i = 0; delimiters[i]; ++i)
		{
			if (c == delimiters[i])
				return true;
		}

		return false;
	}

	och::string tokenizer::operator()()
	{
		while (is_delimiter(*text))
			++text;

		if (!(*text))
			return { nullptr, nullptr };

		const char* beg = text;

		while (*text && !is_delimiter(*text))
			++text;

		return { beg, text++ };
	}
}