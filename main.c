#include "common.h"

extern void insert_symbol (struct symbols *, char *);
extern void print_all_symbols (struct symbols *);
extern struct symbols *create_symbol_table (void);

static struct symbols *root;
static FILE *tmp;

int check_overflow (int offset, int size)
{
	if (offset >= size)
		return OVERFLOW_DETECTED;
	return 0;
}

int is_string (char *s)
{
	return (1 ^ strcmp(STRING_TYPE, s));
}

int is_variable (char c)
{
	return (isalnum(c) || c == '_');
}

int parse_string (char *line, int offset, int line_length, int type)
{
	int state, i, ret;	
	char var_name[32];
	int var_index;
	char assignment[256];
	int assign_index;
	
	char buffer[256];
	int size;
	
	memset(var_name, 0, 32);
	var_index = 0;		
	memset(assignment, 0, 256);
	assign_index = 0;		
	state = ret = 0;

	for (i = offset; i < line_length && state != 3; i++) {
		switch(state) {
		case 0:
			if (isspace(line[i]))
				continue;
			
			if (is_variable(line[i])) {
				var_name[var_index++] = line[i];
				ret = check_overflow(var_index, 32);
			} else {
				if (line[i] == ';')
					state = 3;
				else if (line[i] == '=')
					state = 1;
			}		
			break;
		case 1:
			if (!isspace(line[i])) {
				if (line[i] == '\"')
					state = 2;
			} else
				continue;	
			break;
		case 2:
			if (!isspace(line[i])) {
				if (line[i] == '\"')
					state = 3;
				else {
					assignment[assign_index++] = line[i];	
					ret = check_overflow(assign_index, 256);		
				}	
			} else
				continue;	
			break;	
		default:
			break;
		}
	}	
	if (type == 1 && root != NULL) 
		insert_symbol(root, var_name);

	if (type == 1) {
		size = snprintf(buffer, 256, "\tchar %s = calloc(%d, sizeof(char));\n\0", 
			var_name, assign_index+1);
		fwrite(buffer, size, sizeof(char), tmp);
		
		if (assign_index) {
			size = snprintf(buffer, 256, "\tstrcpy(%s, \"%s\");\n\0", 
				var_name, assignment);
			fwrite(buffer, size, sizeof(char), tmp);
		}
	} else {
		size = snprintf(buffer, 256, "\t%s = realloc(%s, %d);\n\0", 
			var_name, var_name, assign_index+1);
		fwrite(buffer, size, sizeof(char), tmp);
		
		size = snprintf(buffer, 256, "\tstrcpy(%s, \"%s\");\n\0", 
			var_name, (assign_index)? assignment : "0");
		fwrite(buffer, size, sizeof(char), tmp);
	} 
	
	printf("Var_name: %s - Assign: %s Assign_size: %d\n", var_name, assignment, assign_index);	
	return ret;
}

int find_assign(char *line, int offset, int length)
{
	int i;
	
	for (i = offset; i < length; i++) {
		if (isspace(line[i]))
			continue;
		if (line[i] == '=')
			return 1;
		else 
			return 0;
	}
	return 0;
}

int parse_line (char *line, int line_length)
{
	int i, ret, buffer_index;
	char buffer[32];	
	
	memset(buffer, 0, 32);
	buffer_index = ret = 0;
	
	printf("%s", line);
	
	for (i = 0; i < line_length; i++) {		
		if (isspace(line[i]) && !buffer_index)
			continue;
		else if (line[i] == '#') {
			ret = DIRECTIVE_FOUND;
			break;
		} else if (isspace(line[i])) {
			if (is_string(buffer)) 			
				return parse_string(line, i, line_length, 1);
			else if (is_variable(buffer[0]) && find_assign(line, i, line_length)) 
				return parse_string(line, i-1, line_length, 0);				
			else {
				memset(buffer, 0, 32);
				buffer_index = 0;
			}
		} else {
			buffer[buffer_index++] = line[i];
			if (check_overflow(buffer_index, 32))
				return OVERFLOW_DETECTED;
		}
	}
	fwrite(line, strlen(line), sizeof(char), tmp);
	
	return ret;
}

int main (int argc, char **argv)
{
	FILE *stream;
	signed short c;
	char cache_file_ln[256];
	int line_length, line_nr;
	int status;
	
	if (argc != 2)
		fprintf(stderr, "%s: Argc != 2\n", argv[0]);
	
	stream = fopen(argv[1], "r");
	if (stream == NULL)
		fprintf(stderr, "File % cannot be opened\n", argv[1]);
	
	tmp = fopen("tmp", "w");
	if (tmp == NULL)
		fprintf(stderr, "tmp file cannot be opened\n");
	
	
	status = line_length = line_nr = 0;
	memset(cache_file_ln, 0, 256);
	root = (struct symbols *) create_symbol_table();
	printf("root symbol address: %p\n\n", root);
	
	while (status != EOF) {
		c = fgetc(stream);
		if (c != '\n' && c != EOF) {
			cache_file_ln[line_length++] = c;
			if (line_length >= 256)
				fprintf(stderr, "Line (%d) too large!\n", 
					line_nr + 1);
		} else {
			if (c == EOF) 
				status = EOF;
			else if (c == '\n') 
				cache_file_ln[line_length] = c;
			
			if (line_length >= 1) {
				int err = parse_line(cache_file_ln, line_length);
				if (err == OVERFLOW_DETECTED)
					fprintf(stderr, "Overflow!\n");
			}
			
			line_length = 0;
			memset(cache_file_ln, 0, 256);
			line_nr++;
		}		
	}
	puts("\n");
	print_all_symbols(root);
		
	free_symbol_table(root);
	fclose(tmp);
	fclose(stream);
	return 0;
}