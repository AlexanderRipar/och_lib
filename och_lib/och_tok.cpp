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



	cstr_tokenizer::cstr_tokenizer(const char* text, const char* delimiters) : text{ text }, delimiters{ delimiters } {}

	bool cstr_tokenizer::is_delimiter(char c)
	{
		for (int i = 0; delimiters[i]; ++i)
		{
			if (c == delimiters[i])
				return true;
		}

		return false;
	}

	och::string cstr_tokenizer::operator()()
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

	tokenizer::tokenizer(och::string text, och::string delimiters) : text{ text }, delimiters{ delimiters } {}

	bool tokenizer::is_delimiter(char c)
	{
		for (const char& d : delimiters)
		{
			if (c == d)
				return true;
		}

		return false;
	}

	och::string tokenizer::operator()()
	{
		while (is_delimiter(*text.beg))
			++text.beg;

		if (!text.len())
			return { nullptr, nullptr };

		const char* beg = text.beg;

		while (*text.beg && !is_delimiter(*text.beg))
			++text.beg;

		return { beg, text.beg++ };
	}

	matcher::matcher(och::string text, och::string target) : text{ text }, target{ target } {}

	och::string matcher::operator()()
	{
		for (; text_idx != text.len(); ++text_idx)
				if (target[target_idx] == text[text_idx])
				{
					if (++target_idx == target.len())
					{
						target_idx = 0;
						return { text.beg + 1 + text_idx - target.len(), text.beg + 1 + text_idx };
					}
				}
				else
					target_idx = 0;

		return { nullptr, nullptr };
	}

	uint32_t matcher::count()
	{
		uint32_t hits = 0;

		for (; text_idx != text.len(); ++text_idx)
			if (target[target_idx] == text[text_idx])
			{
				if (++target_idx == target.len())
				{
					target_idx = 0;
					++hits;
				}
			}
			else
				target_idx = 0;

		return hits;
	}

	void matcher::reset()
	{
		text_idx = 0;
		target_idx = 0;
	}

	multi_matcher::multi_matcher(och::string text, och::range<och::string> targets) : text{ text }, targets{ targets.beg, targets.end } {}

	och::string multi_matcher::operator()()
	{
		for (char c = *(text.beg++); text.beg <= text.end; c = *(text.beg++))
			for (int i = 0; i != targets.len(); ++i)
				if (targets[i][target_idx[i]] == c)
				{
					if (++target_idx[i] == targets[i].len())
					{
						target_idx[i] = 0;
						return { text.beg - targets[i].len(), text.beg };
					}
				}
				else
					target_idx[i] = 0;

		return { nullptr, nullptr };
	}

	uint32_t multi_matcher::count()
	{
		uint32_t hits = 0;

		for (char c = *(text.beg++); text.beg <= text.end; c = *(text.beg++))
			for (int i = 0; i != targets.len(); ++i)
				if (targets[i][target_idx[i]] == c)
				{
					if (++target_idx[i] == targets[i].len())
					{
						target_idx[i] = 0;
						++hits;
					}
				}
				else
					target_idx[i] = 0;

		return hits;
	}
}
