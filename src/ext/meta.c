#include "meta.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../util/helper.h"
#include "../util/vector.h"
#include "context.h"

static int abs_index = 0;

typedef struct {
	const char *name;
	int (*const func)(uint32_t argc, char **argv, char **command);
} meta;

// Meta comamnds
int meta_ctx(uint32_t argc, char **argv, unused char **command);
int meta_abs(uint32_t argc, char **argv, unused char **command);
int meta_asroot(uint32_t argc, char **argv, char **command);

static const meta registry[] = {
	{ .name = ":ctx", .func = &meta_ctx },
	{ .name = ":abs", .func = &meta_abs },
	{ .name = ":asroot", .func = &meta_asroot }
};
static const size_t registry_length = sizeof(registry) / sizeof(meta);

// Helper functions
const meta *find_meta(const char *name);

int run_meta(str_vec *args, char **command) {
	// Check for standard meta
	char *name = fix_ptr(vec_at(args, 0));
	const meta *result = find_meta(name);
	if (result != NULL) {
		return result->func(args->count, args->raw, command);
	}

	// Check for numeric meta
	char *end;
	uint32_t index = strtoul(name + 1, &end, 10);
	if (*end != '\0') {
		print_error("%s: meta command not found\n", name + 1);
		return -1;
	}

	const char *num_result = abs_index
		? context_get_row_abs(index)
		: context_get_row_rel(index);
	if (num_result == NULL) {
		print_error("no such command in context\n");
		return -1;
	}

	*command = strdup(num_result);
	return 1;
}

/** Meta commands */

int meta_ctx(uint32_t argc, char **argv, unused char **command) {
	if (argc > 2) {
		print_error("too many arguments\n");
		return -1;
	}

	// Switch context
	if (argc == 2) {
		if (context_select(argv[1]) < 0) {
			print_error("context '%s' does not exist\n", argv[1]);
			return -1;
		}

		return 0;
	}

	// Print current context
	const context *ctx = context_get();

	printf("Context name: %s\n\n", ctx->name);
	for (uint32_t i = 0; i < ctx->commands->count; i++) {
		uint32_t index = abs_index ? i : ctx->commands->count - i - 1;
		printf("%u: %s\n", index, (char *)fix_ptr(vec_at(ctx->commands, i)));
	}

	return 0;
}

int meta_abs(uint32_t argc, char **argv, unused char **command) {
	if (argc > 2) {
		print_error("too many arguments\n");
		return -1;
	}

	if (argc == 2) {
		switch (argv[1][0]) {
			case '0':
				abs_index = 0;
				break;
			case '1':
				abs_index = 1;
				break;
			default:
				print_error("invalid argument\n");
				return -1;
		}
	} else {
		abs_index = !abs_index;
	}

	if (abs_index) {
		printf("using absolute indexing\n");
	} else {
		printf("using relative indexing\n");
	}

	return 0;
}

int meta_asroot(uint32_t argc, char **argv, char **command) {
	if (argc > 2) {
		print_error("too many arguments\n");
		return -1;
	}

	uint32_t index = 0;
	if (argc == 2) {
		char *end;
		index = strtoul(argv[1], &end, 10);

		if (*end != '\0') {
			print_error("argument must be a row\n");
			return -1;
		}
	}

	const char *result = abs_index
		? context_get_row_abs(index)
		: context_get_row_rel(index);
	if (result == NULL) {
		print_error("no such command in context\n");
		return -1;
	}

	char buffer[strlen(result) + 5];
	sprintf(buffer, "doas %s", result);

	*command = strdup(buffer);
	return 1;
}

/** Internal commands */

/**
 * @brief Find builtin in the registry.
 *
 * @param[in] name - Search query.
 * @return Builtin entry; NULL on error.
 */
const meta *find_meta(const char *name) {
	for (size_t i = 0; i < registry_length; i++) {
		if (strcmp(registry[i].name, name) == 0) {
			return registry + i;
		}
	}

	return NULL;
}
