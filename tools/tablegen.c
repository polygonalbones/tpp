#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <murmur3.h>

#define MAX_KEYWORDS 1024
#define MAX_LINE_LEN 256
#define HASH_SEED 42

typedef struct {
  char raw[MAX_LINE_LEN];
  char enum_name[MAX_LINE_LEN];
  int occupied;
} Slot;

static void to_enum_name(const char *src, char *dst) {
  strcpy(dst, "TOKEN_");
  size_t j = 3;
  for(size_t i = 0; src[i] != '\0'; i++) {
    if(isalnum((unsigned char)src[i]))
      dst[j++] = (char)toupper((unsigned char)src[i]);
    else
      dst[j++] = '_';
  }
  dst[j] = '\0';
}

static size_t next_pow2(size_t n) {
  size_t p = 1;
  while (p < n) p <<= 1;
  return p;
}

int main(int argc, char **argv) {
  FILE *in = fopen("include/keywords.txt", "r");
  if(!in) {
    perror("fopen input");
    return 1;
  }

  char keywords[MAX_KEYWORDS][MAX_LINE_LEN];
  char enums[MAX_KEYWORDS][MAX_LINE_LEN];
  size_t count = 0;

  char line[MAX_LINE_LEN];
  while(fgets(line, sizeof(line), in) && count < MAX_KEYWORDS) {
    line[strcspn(line, "\r\n")] = '\0';
    if (line[0] == '\0') continue;

    strcpy(keywords[count], line);
    to_enum_name(line, enums[count]);
    count++;
  }
  fclose(in);

  size_t table_size = next_pow2(count * 2);
  Slot *table = calloc(table_size, sizeof(Slot));

  for(size_t i = 0; i < count; i++) {
    uint32_t hash;
    MurmurHash3_x86_32(keywords[i], (int)strlen(keywords[i]), HASH_SEED, &hash);
    size_t idx = hash & (table_size - 1);

    while(table[idx].occupied) {
      idx = (idx + 1) & (table_size - 1);
    }
    strcpy(table[idx].raw, keywords[i]);
    strcpy(table[idx].enum_name, enums[i]);
    table[idx].occupied = 1;
  }

  FILE *out = fopen("build/keywords.h", "w");
  if(!out) {
    perror("fopen output");
    return 1;
  }

  fprintf(out, "#ifndef KEYWORDS_H_\n#define KEYWORDS_H_\n\n");
  fprintf(out, "#include <stddef.h>\n#include <stdint.h>\n#include <string.h>\n#include <murmur3.h>\n");

  fprintf(out, "typedef enum {\n    KW_INVALID = -1,\n");
  for(size_t i = 0; i < count; i++) {
    fprintf(out, "    %s = %zu,\n", enums[i], i);
  }
  fprintf(out, "    KW_COUNT = %zu\n} KeywordEnum;\n\n", count);

  fprintf(out, "typedef struct {\n    const char *key;\n    KeywordEnum val;\n} KeywordSlot;\n\n");
  fprintf(out, "static const KeywordSlot KW_TABLE[%zu] = {\n", table_size);
  for(size_t i = 0; i < table_size; i++) {
    if(table[i].occupied)
      fprintf(out, "    [%zu] = { \"%s\", %s },\n", i, table[i].raw, table[i].enum_name);
    else
      fprintf(out, "    [%zu] = { NULL, KW_INVALID },\n", i);
  }
  fprintf(out, "};\n\n");

  fprintf(out, "static inline KeywordEnum keywords_contains(const char *str, size_t len) {\n");
  fprintf(out, "    uint32_t hash;\n");
  fprintf(out, "    MurmurHash3_x86_32(str, (int)len, %d, &hash);\n", HASH_SEED);
  fprintf(out, "    size_t idx = hash & %zu;\n", table_size - 1);
  fprintf(out, "    while (KW_TABLE[idx].key != NULL) {\n");
  fprintf(out, "        if (strncmp(KW_TABLE[idx].key, str, len) == 0 && KW_TABLE[idx].key[len] == '\\0') {\n");
  fprintf(out, "            return KW_TABLE[idx].val;\n");
  fprintf(out, "        }\n");
  fprintf(out, "        idx = (idx + 1) & %zu;\n", table_size - 1);
  fprintf(out, "    }\n");
  fprintf(out, "    return KW_INVALID;\n");
  fprintf(out, "}\n\n#endif // KEYWORDS_H_\n");

  fclose(out);
  free(table);
  return 0;
}
