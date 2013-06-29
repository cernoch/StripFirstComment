#include <stdio.h>
#include <unistd.h>

int main(int argc, char** args) {

	if (argc < 2) {
		printf("Usage: %s source.java [source2.java ...]\n", args[0]);
		return 1;
	}


	for (int i=1; i<argc; i++) {
		FILE* f = fopen(args[i], "r");
		if (f == NULL) {
			fprintf(stderr, "File %s not found, skipping.\n", args[i]);
			continue;
		}

		// Read the first byte
		int last = fgetc(f);
		if (last == EOF) {
			fprintf(stderr, "File %s empty or unreadable.\n", args[i]);
			fclose(f);
			continue;
		}

		// Delete the file (but keep the 'f' opened)
		if (unlink(args[i]) != 0) {
			fprintf(stderr, "File %s cannot be recreated.\n", args[i]);
			fclose(f);
			return 2;
		}

		// Recreate the file
		FILE* w = fopen(args[i], "w");
		if (w == NULL) {
			fprintf(stderr, "File %s cannot be recreated. File lost, sorry.\n", args[i]);
			fclose(f);
			fclose(w);
			return 3;
		}

		// 1) Read white space at the beginning of the file
		while (last == ' '
		|| 	last == '\t'
		||	last == '\n'
		||	last == '\r') {

			if (fputc(last, w) == EOF) {
				fprintf(stderr, "File %s cannot be written to. File lost, sorry.\n", args[i]);
				fclose(f);
				fclose(w);
				return 3;
			}
			last = fgetc(f);
		}

		if (last == EOF) {
			// File has only whitespace
			goto finale;
		}

		// 2) Do we meet a comment starting? 		
		int curr = fgetc(f);
		if (curr == EOF) {
			if (fputc(last,w) == EOF) {
				fprintf(stderr, "Last byte of %s was not written. The byte was lost, sorry.", args[i]);
				fclose(f);
				fclose(w);
				return 3;
			}
			goto finale;
		}

		if (last == '/' && curr == '*') {
			// 2a) YES

			curr = 0; // Avoid /*/ being a comment
			do {
				last = curr;
				if ((curr = fgetc(f)) == EOF) {
					goto finale;
				}
			} while (!(last == '*' && curr == '/'));

			// Read white-space till EOL
			do {
				if ((curr = fgetc(f)) == EOF) {
					goto finale;
				}
			} while (curr == ' '
				|| curr == '\t'
				|| curr == '\r'
				|| curr == '\n');
		} else {
			// 2b) NO
			fputc(last,w);
		}
		
		// 3) Write till the end of file
		while (curr != EOF) {
			if (fputc(curr, w) == EOF) {
				fprintf(stderr, "File %s cannot be written to. File lost, sorry.\n", args[i]);
				fclose(f);
				fclose(w);
				return 3;
			}
			curr = fgetc(f);
		}

		finale:
		fclose(f);
		fclose(w);
	}
}

