#ifndef WIDGET_DEFINES_HPP_INCLUDED
#define WIDGET_DEFINES_HPP_INCLUDED

char const HELP_STRING_SEPARATOR = '|', DEFAULT_ITEM = '*', COLUMN_SEPARATOR = '=',
           IMAGE_PREFIX = '&', IMG_TEXT_SEPARATOR = 17, HEADING_PREFIX = 18;

inline bool is_wml_separator(char c)
{
	switch(c)
	{
	case HELP_STRING_SEPARATOR:
	case DEFAULT_ITEM:
	case COLUMN_SEPARATOR:
	case IMAGE_PREFIX:
	case IMG_TEXT_SEPARATOR:
	case HEADING_PREFIX:
		return true;
	default:
		return false;
	}
}

#endif
