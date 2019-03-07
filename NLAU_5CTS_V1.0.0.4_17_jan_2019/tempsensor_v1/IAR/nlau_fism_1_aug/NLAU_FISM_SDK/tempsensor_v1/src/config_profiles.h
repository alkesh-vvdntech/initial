/*
 * config_profiles.h
 *
 *  Created on: May 13, 2016
 *      Author: Ben
 */
 
#ifndef _CONFIG_PROFILES_H_
#define _CONFIG_PROFILES_H_

//config entries are process based on type -end meaning end of profile
typedef enum {
    CONFIG_STRING,
    CONFIG_BIT_FLAG, //uses ini_getbool
    CONFIG_INT,
    CONFIG_FLOAT,
    CONFIG_END
} CONFIG_ENTRY_TYPE;

//config entry struct defaults (def) must be populated based on type:
//STRING - defStr and memSize must be non-zero
//BIT_FLAG - defInt will be used and memSize is bit position
//INT and FLOAT - defInt will be used and memSize 
//
//MANDATORY ENTRIES = section, key and memLoc
//memLoc is always a void pointer to the memory location of which ever
//type is targeted and memSize is always the size of the container
typedef struct {
    const CONFIG_ENTRY_TYPE type;
    const char *section;
    const char *key;
    const char *defStr; //string held in const mem
    const long defInt;  //value type for non-strings
    void *memLoc;
    const int memSize;
} CONFIG_ENTRY;

typedef const CONFIG_ENTRY * ConfigEntry;

typedef const ConfigEntry * ConfigProfile;

extern const ConfigProfile g_DefaultConfigProfile;
extern const ConfigProfile g_ServiceModeConfigProfile;

void config_load_profile(ConfigProfile profile);

#endif /*_CONFIG_PROFILES_H_*/