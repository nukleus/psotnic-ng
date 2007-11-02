#include <stdio.h>
#include <string.h>
#include "firedns.h"

int main(int argc, char **argv) {
	struct firedns_mxlist *iter;

	if (argc != 2) {
		fprintf(stderr,"usage: %s <hostname>\n",argv[0]);
		return 2;
	}

	iter = firedns_resolvemxlist(argv[1]);

	if (iter == NULL)
		return 1;

	while (iter != NULL) {
		printf("%7s (%05d) %s:%d\n",firedns_mx_name[iter->protocol],iter->priority,iter->name,firedns_mx_port[iter->protocol]);
		iter = iter->next;
	}

	return 0;
}
