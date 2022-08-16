#ifndef MOD_DECRYPTOR_H
#define MOD_DECRYPTOR_H

#include "mod_local.h"


bool Mod_IsModelEncrypted(const char *model_name, const byte *buffer);
void Mod_DecryptModel(const char *model_name, byte *buffer);


#endif