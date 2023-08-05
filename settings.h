/* settings.h */


#define SETTINGS_CONFIG_FILENAME "settings.cfg"
#define SETTINGS_CONFIG_TMP_FILENAME "settings.tmp"
#define SETTINGS_CONFIG_OLD_FILENAME "settings.old"

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

/**********************************************************************/

int definesetting(section, keyword, newvalue)
	char	*section;
	char	*keyword;
	char	*newvalue;
{
	int	lensection;
	int	lenkeyword;
	FILE	*settings;
	FILE	*tmpsettings;
	int	insection;
	char	line[MAX_SETTINGS_LINE_LENGTH];
	char	linecopy[MAX_SETTINGS_LINE_LENGTH];
	int	lenline;
	int	changed;

	/* printf("[%s] [%s] [%s]\n", section, keyword, newvalue); */

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

	if ((tmpsettings = fopen(SETTINGS_CONFIG_TMP_FILENAME, "w")) == NULL) {
		return FALSE;
	}

	insection = FALSE;

	changed = FALSE;

	while (fgets(line, MAX_SETTINGS_LINE_LENGTH - sizeof(char), settings) != NULL) {
		/* printf(">1> line: %s", line); */

		lenline = strlen(line);

		if (lenline > 0) {
			if (line[lenline - 1] == '\n') {
				line[lenline - 1] = '\0';
				lenline--;
			}
		}

		if (lenline != 0) {
			if (line[0] != '#') {
				if (sectionmatch(line, section)) {
					insection = TRUE;
				} else if (line[0] == '[') {
					insection = FALSE;
				} else {
					if (insection) {
						if (strncmp(line, keyword, lenkeyword) == 0) {
							if (line[lenkeyword] == '=') {
								strcpy(line + lenkeyword + 1, newvalue);
								changed = TRUE;
							}
						}
					}
				}
			}
		}

		/* printf(">2> line: %s", line); */

		fprintf(tmpsettings, "%s\n", line);
	}

	fflush(tmpsettings);
	fflush(tmpsettings);
	fflush(tmpsettings);
	fclose(tmpsettings);

	fclose(settings);

	if (changed) {
		rename(SETTINGS_CONFIG_FILENAME, SETTINGS_CONFIG_OLD_FILENAME);
		rename(SETTINGS_CONFIG_TMP_FILENAME, SETTINGS_CONFIG_FILENAME);
	}

	return TRUE;
}
		
/* end of file */
