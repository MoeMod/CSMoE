#pragma once

namespace sv {

typedef enum
{
    CLASSNAME
} hash_types_e;

typedef struct hash_item_s
{
    entvars_t *pev;
    struct hash_item_s *next;
    struct hash_item_s *lastHash;
    int pevIndex;
} hash_item_t;

extern CUtlVector<hash_item_t> stringsHashTable;

int CaseInsensitiveHash(const char *string, int iBounds);
void EmptyEntityHashTable(void);
void AddEntityHashValue(entvars_t *pev, const char *value, hash_types_e fieldType);
void RemoveEntityHashValue(entvars_t *pev, const char *value, hash_types_e fieldType);
void printEntities(void);
void loopPerformance(void);

}