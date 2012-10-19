#include "common.h"

void insert_symbol (struct symbols *root, char *symbol_name)
{
	struct symbols *new, *p;
	
	assert(root && symbol_name);
	
	new = malloc(sizeof(struct symbols));
	strcpy(new->symbol_name, symbol_name);
	new->next = NULL;
	
	for (p = root; p->next != NULL; p = p->next)
		;
	p->next = new;
}

void print_all_symbols (struct symbols *root)
{
	struct symbols *p;
	
	assert(root);
	
	p = root;
	while (p->next != NULL) {
		p = p->next;
		printf("Addr: %p Symbol_name: %s\n",
		       p, p->symbol_name);
	}
}

struct symbols *create_symbol_table (void)
{
	struct symbols *p;
	p = malloc(sizeof(struct symbols));
	memset(p, 0, sizeof(struct symbols));
	return p;
}

void free_symbol_table (struct symbols *root)
{
	struct symbols *p, *q;
	
	assert(root);
	
	p = root;
	while (p->next != NULL) {
		q = p->next;
		free(p);
		p = q;
	}	
}