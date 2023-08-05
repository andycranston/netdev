/* settings.h */


#define SETTINGS_CONFIG_FILENAME "settings.cfg"

#define MAX_SETTINGS_LINE_LENGTH 1024

/**********************************************************************/

int sectionmatch(line, section)
	char	*line;
	char	*section;
{
	int	lenline;
	int	lensection;

	lenline = strlen(line);
	lensection = strlen(section);

	if (lenline != (1 + lensection + 1)) {
		return FALSE;
	}

	if (line[0] != '[') {
		return FALSE;
	}

	if (line[lenline - 1] != ']') {
		return FALSE;
	}

	if (strncmp(line + 1, section, lensection) != 0) {
		return FALSE;
	}

	return TRUE;
}

/**********************************************************************/


int lookupsetting(section, keyword, value)
	char	*section;
	char	*keyword;
	char	*value;
{
	int	lensection;
	int	lenkeyword;
	FILE	*settings;
	int	insection;
	char	line[MAX_SETTINGS_LINE_LENGTH];
	int	lenline;

	lensection = strlen(section);

	if (lensection == 0) {
		return FALSE;
	}

	lenkeyword = strlen(keyword);

	if (lenkeyword == 0) {
		return FALSE;
	}

	if ((settings = fopen(SETTINGS_CONFIG_FILENAME, "r")) == NULL) {
		return FALSE;
	}

	insection = FALSE;

	value[0] = '\0';

	while (fgets(line, MAX_SETTINGS_LINE_LENGTH - sizeof(char), settings) != NULL) {
		lenline = strlen(line);

		if (lenline > 0) {
			if (line[lenline - 1] == '\n') {
				line[lenline - 1] = '\0';
				lenline--;
			}
		}

		if (lenline == 0) {
			continue;
		}


		if (line[0] == '#') {
			continue;
		}

		if (sectionmatch(line, section)) {
			insection = TRUE;
			continue;
		}

		if (line[0] == '[') {
			insection = FALSE;
			continue;
		}

		if (insection) {
			if (strncmp(line, keyword, lenkeyword) == 0) {
				if (line[lenkeyword] == '=') {
					strcpy(value, line + lenkeyword + 1);
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

		
/* end of file */
