/*
 * config_profiles.c
 *
 *  Created on: May 13, 2016
 *      Author: Ben
 */

#include "config_profiles.h"
#include "config.h"
#ifdef USE_MININI
#include "minIni.h"
#endif

static const char tagSMS[] = "SMS";
static const char tagSystem[] = "SYSTEM";
static const char tagServer[] = "SERVER";
static const char tagIntervals[] = "INTERVALS";
static const char tagSIM1[]     = "SIM1";
static const char tagSIM2[]     = "SIM2";
static const char tagPower[] = "POWER";
static const char tagInvalid[] = "INVALID"; //use this for parameters that shouldn't
                                            //be entered in the ini file

/*********************** BEGIN SECTION CONFIG PROFILES ***********************/

/* Individual Configuration Entries Below */

/* SYSTEM Entries Start */
#ifndef _EVIN_BUILD_
static const CONFIG_ENTRY g_cfgEntry_ConfigVersionString = {
    CONFIG_STRING,
    tagSystem,
    "AppVersion",
    DEFAULT_APP_MODE_STRING, 
    0,
    (void *)g_ConfigDevice.cfgVersion,
    sizeof(g_ConfigDevice.cfgVersion)
};
#endif

//
static const CONFIG_ENTRY g_cfgEntry_TextPersistFlag = {
    CONFIG_BIT_FLAG,
    tagSystem,
    "Text_Persist",
    "", 
    LCD_TEXT_PERSIST,
    (void *)&g_ConfigDevice.cfg.status, //this is the base address of cfg.logs
    5 //bit position
};

static const CONFIG_ENTRY g_cfgEntry_BuzzerDisableFlag = {
    CONFIG_BIT_FLAG,
    tagSystem,
    "Buzzer_Off",
    "", 
    BUZZER_DISABLE,
    (void *)&g_ConfigDevice.cfg.status, //this is the base address of cfg.logs
    6 //bit position
};

static const CONFIG_ENTRY g_cfgEntry_BuzzerToggleFlag = {
    CONFIG_BIT_FLAG,
    tagSystem,
    "Buzzer_Toggle",
    "", 
    BUZZER_TOGGLE,
    (void *)&g_ConfigDevice.cfg.status, //this is the base address of cfg.logs
    7 //bit position
};

//static const CONFIG_ENTRY g_cfgEntry_ServerConfigLogFlag = { //no ini entry
//    CONFIG_BIT_FLAG,
//    tagInvalid,
//    "",
//    "", 
//    LOG_DEFAULT_CONFIG,
//    (void *)&g_ConfigDevice.cfg.status, //this is the base address of cfg.logs
//    2 //bit position
//};

static const CONFIG_ENTRY g_cfgEntry_TimeZoneInt = {
    CONFIG_INT,
    tagSystem,
    "TimeZone",
    "", 
    SYSTEM_TIMEZONE,
    (void *)&g_ConfigDevice.cfgTimeZone,
    sizeof(g_ConfigDevice.cfgTimeZone)
};
/* SYSTEM Entries End */

/* POWER Entries Start */
static const CONFIG_ENTRY g_cfgEntry_PowerStateForcedUploadFlag = {
    CONFIG_BIT_FLAG,
    tagPower,
    "StateForcedUpload",
    "", 
    POWER_CHANGE_FORCE_UPLOAD_ENABLE,
    (void *)&g_ConfigDevice.stBattPowerAlertParam.flags.byte,
    1 //bit position
};
/* POWER Entries End */

static const CONFIG_ENTRY g_cfgEntry_LowTempThreshA = { //testing for float defaults
    CONFIG_FLOAT,
    tagInvalid,
    "",
    "", 
    LOW_TEMP_THRESHOLD,
    (void *)&g_ConfigDevice.stTempAlertParams[0].threshCold,
    sizeof(g_ConfigDevice.stTempAlertParams[0].threshCold)
};

/* SERVER Entries Start */

//These are the default server keys
static const CONFIG_ENTRY g_cfgEntry_GatewayKeyString = {
    CONFIG_STRING,
    tagServer,
    "GatewayKey",
    "", 
    0,
    (void *)g_ConfigDevice.cfgGatewayKey,
    sizeof(g_ConfigDevice.cfgGatewayKey)
};

static const CONFIG_ENTRY g_cfgEntry_GatewaySMSString = {
    CONFIG_STRING,
    tagServer,
    "GatewaySMS",
    NEXLEAF_SMS_GATEWAY, 
    0,
    (void *)g_ConfigDevice.cfgGatewaySMS,
    sizeof(g_ConfigDevice.cfgGatewaySMS)
};

static const CONFIG_ENTRY g_cfgEntry_GatewayIPString = {
    CONFIG_STRING,
    tagServer,
    "GatewayIP",
    NEXLEAF_DEFAULT_SERVER_IP, 
    0,
    (void *)g_ConfigDevice.cfgGatewayIP,
    sizeof(g_ConfigDevice.cfgGatewayIP)
};

static const CONFIG_ENTRY g_cfgEntry_UploadURLString = {
    CONFIG_STRING,
    tagServer,
    "Upload_URL",
    DATA_UPLOAD_URL_PATH, 
    0,
    (void *)g_ConfigDevice.cfgUpload_URL,
    sizeof(g_ConfigDevice.cfgUpload_URL)
};

static const CONFIG_ENTRY g_cfgEntry_DeviceAlarmURLString = {
    CONFIG_STRING,
    tagServer,
    "DeviceAlarm_URL",
    DEVICE_ALARM_URL_PATH, 
    0,
    (void *)g_ConfigDevice.cfgDeviceAlarm_URL,
    sizeof(g_ConfigDevice.cfgDeviceAlarm_URL)
};

static const CONFIG_ENTRY g_cfgEntry_DeviceReadyURLString = {
    CONFIG_STRING,
    tagServer,
    "DeviceReady_URL",
    DEVICE_READY_URL_PATH, 
    0,
    (void *)g_ConfigDevice.cfgDeviceReady_URL,
    sizeof(g_ConfigDevice.cfgDeviceReady_URL)
};

static const CONFIG_ENTRY g_cfgEntry_ConfigURLString = {
    CONFIG_STRING,
    tagServer,
    "Config_URL",
    CONFIGURATION_URL_PATH, 
    0,
    (void *)g_ConfigDevice.cfgConfig_URL,
    sizeof(g_ConfigDevice.cfgConfig_URL)
};

//These are the service mode server keys
static const CONFIG_ENTRY g_cfgEntry_GatewayKeyString_svcMode = {
    CONFIG_STRING,
    tagServer,
    "GatewayKey_NL",
    "cldt", 
    0,
    (void *)g_ConfigDevice.cfgGatewayKey,
    sizeof(g_ConfigDevice.cfgGatewayKey)
};

static const CONFIG_ENTRY g_cfgEntry_GatewaySMSString_svcMode = {
    CONFIG_STRING,
    tagServer,
    "GatewaySMS_NL",
    NEXLEAF_SMS_GATEWAY, 
    0,
    (void *)g_ConfigDevice.cfgGatewaySMS,
    sizeof(g_ConfigDevice.cfgGatewaySMS)
};

static const CONFIG_ENTRY g_cfgEntry_GatewayIPString_svcMode = {
    CONFIG_STRING,
    tagServer,
    "GatewayIP_NL",
    NEXLEAF_DEFAULT_SERVER_IP, 
    0,
    (void *)g_ConfigDevice.cfgGatewayIP,
    sizeof(g_ConfigDevice.cfgGatewayIP)
};

static const CONFIG_ENTRY g_cfgEntry_UploadURLString_svcMode = {
    CONFIG_STRING,
    tagServer,
    "Upload_URL_NL",
    DATA_UPLOAD_URL_PATH, 
    0,
    (void *)g_ConfigDevice.cfgUpload_URL,
    sizeof(g_ConfigDevice.cfgUpload_URL)
};

static const CONFIG_ENTRY g_cfgEntry_DeviceAlarmURLString_svcMode = {
    CONFIG_STRING,
    tagServer,
    "DeviceAlarm_URL_NL",
    DEVICE_ALARM_URL_PATH, 
    0,
    (void *)g_ConfigDevice.cfgDeviceAlarm_URL,
    sizeof(g_ConfigDevice.cfgDeviceAlarm_URL)
};

static const CONFIG_ENTRY g_cfgEntry_DeviceReadyURLString_svcMode = {
    CONFIG_STRING,
    tagServer,
    "DeviceReady_URL_NL",
    DEVICE_READY_URL_PATH, 
    0,
    (void *)g_ConfigDevice.cfgDeviceReady_URL,
    sizeof(g_ConfigDevice.cfgDeviceReady_URL)
};

static const CONFIG_ENTRY g_cfgEntry_ConfigURLString_svcMode = {
    CONFIG_STRING,
    tagServer,
    "Config_URL_NL",
    CONFIGURATION_URL_PATH, 
    0,
    (void *)g_ConfigDevice.cfgConfig_URL,
    sizeof(g_ConfigDevice.cfgConfig_URL)
};

static const CONFIG_ENTRY g_cfgEntry_GSMModeEnabledFlag = {
    CONFIG_BIT_FLAG,
    tagServer,
    "GSMModeEnabled",
    "", 
    GSM_MODE_DEFAULT,
    (void *)&g_ConfigDevice.cfgUploadMode,
    0 //bit position
};

static const CONFIG_ENTRY g_cfgEntry_GPRSModeEnabledFlag = {
    CONFIG_BIT_FLAG,
    tagServer,
    "GPRSModeEnabled",
    "", 
    GRPS_MODE_DEFAULT,
    (void *)&g_ConfigDevice.cfgUploadMode,
    1 //bit position
};
/* SERVER Entries End */

/* SMS Entries Begin */
static const CONFIG_ENTRY g_cfgEntry_SMSModeDevReadyFlag = {
    CONFIG_BIT_FLAG,
    tagSMS,
    "DeviceReadyOnBoot",
    "", 
    SMS_MODE_DEVRDY_DEFAULT,
    (void *)&g_ConfigDevice.cfgSMSmode.byte,
    0 //bit position
};

static const CONFIG_ENTRY g_cfgEntry_SMSModeFailoverFlag = {
    CONFIG_BIT_FLAG,
    tagSMS,
    "API_Failover",
    "", 
    SMS_MODE_FAILOVER_DEFAULT,
    (void *)&g_ConfigDevice.cfgSMSmode.byte,
    1 //bit position
};
/* SMS Entries End */

//SIM1 APN
static const CONFIG_ENTRY g_cfgEntry_SIM1APNString = {
    CONFIG_STRING,
    tagSIM1,
    "APN",
    g_ConfigDevice.SIM[0].cfgAPN, 
    0,
    (void *)g_ConfigDevice.SIM[0].cfgAPN,
    sizeof(g_ConfigDevice.SIM[0].cfgAPN)
};

//SIM1 APN
static const CONFIG_ENTRY g_cfgEntry_SIM2APNString = {
    CONFIG_STRING,
    tagSIM2,
    "APN",
    g_ConfigDevice.SIM[1].cfgAPN, 
    0,
    (void *)g_ConfigDevice.SIM[1].cfgAPN,
    sizeof(g_ConfigDevice.SIM[1].cfgAPN)
};

/* INTERVALS Entries Begin */

static const CONFIG_ENTRY g_cfgEntry_SamplingInt = {
    CONFIG_INT,
    tagIntervals,
    "Sampling",
    "", 
    PERIOD_SAMPLING,
    (void *)&g_ConfigDevice.sIntervalsMins.sampling,
    sizeof(g_ConfigDevice.sIntervalsMins.sampling)
};

static const CONFIG_ENTRY g_cfgEntry_UploadInt = {
    CONFIG_INT,
    tagIntervals,
    "Upload",
    "", 
    PERIOD_UPLOAD,
    (void *)&g_ConfigDevice.sIntervalsMins.upload,
    sizeof(g_ConfigDevice.sIntervalsMins.upload)
};

static const CONFIG_ENTRY g_cfgEntry_LCDoffInt = {
    CONFIG_INT,
    tagIntervals,
    "LCDoff",
    "", 
    PERIOD_LCD_OFF,
    (void *)&g_ConfigDevice.sIntervalsMins.lcdOff,
    sizeof(g_ConfigDevice.sIntervalsMins.lcdOff)
};

/* INTERVALS Entries End */

static const CONFIG_ENTRY g_structConfigEnd = { CONFIG_END, 0, 0, 0, 0, 0, 0 };

//default only one profile for now
static const ConfigEntry g_DefaultConfigEntries[] = {
#ifndef _EVIN_BUILD_
    &g_cfgEntry_ConfigVersionString,
#endif
    &g_cfgEntry_LowTempThreshA,
    &g_cfgEntry_TextPersistFlag,
    &g_cfgEntry_BuzzerDisableFlag,
    &g_cfgEntry_BuzzerToggleFlag,
    &g_cfgEntry_TimeZoneInt,
    &g_cfgEntry_PowerStateForcedUploadFlag,
    &g_cfgEntry_GatewayKeyString,
    &g_cfgEntry_GatewaySMSString,
    &g_cfgEntry_GatewayIPString,
    &g_cfgEntry_UploadURLString,
    &g_cfgEntry_DeviceAlarmURLString,
    &g_cfgEntry_DeviceReadyURLString,
    &g_cfgEntry_ConfigURLString,
    &g_cfgEntry_GSMModeEnabledFlag,
    &g_cfgEntry_GPRSModeEnabledFlag,
    &g_cfgEntry_SIM1APNString,
    &g_cfgEntry_SIM2APNString,
    &g_cfgEntry_SamplingInt,
    &g_cfgEntry_UploadInt,
    &g_cfgEntry_LCDoffInt,
    &g_cfgEntry_SMSModeFailoverFlag,
    &g_cfgEntry_SMSModeDevReadyFlag,
    &g_structConfigEnd //this must always be the last entry in a profile!
};

const ConfigProfile g_DefaultConfigProfile = g_DefaultConfigEntries;

static const ConfigEntry g_ServiceModeConfigEntries[] = {
#ifndef _EVIN_BUILD_
    &g_cfgEntry_ConfigVersionString,
#endif
    &g_cfgEntry_LowTempThreshA,
    &g_cfgEntry_TextPersistFlag,
    &g_cfgEntry_BuzzerDisableFlag,
    &g_cfgEntry_BuzzerToggleFlag,
    &g_cfgEntry_TimeZoneInt,
    &g_cfgEntry_PowerStateForcedUploadFlag,
    &g_cfgEntry_GatewayKeyString_svcMode,
    &g_cfgEntry_GatewaySMSString_svcMode,
    &g_cfgEntry_GatewayIPString_svcMode,
    &g_cfgEntry_UploadURLString_svcMode,
    &g_cfgEntry_DeviceAlarmURLString_svcMode,
    &g_cfgEntry_DeviceReadyURLString_svcMode,
    &g_cfgEntry_ConfigURLString_svcMode,
    &g_cfgEntry_GSMModeEnabledFlag,
    &g_cfgEntry_GPRSModeEnabledFlag,
    &g_cfgEntry_SIM1APNString,
    &g_cfgEntry_SIM2APNString,
    &g_cfgEntry_SamplingInt,
    &g_cfgEntry_UploadInt,
    &g_cfgEntry_LCDoffInt,
    &g_cfgEntry_SMSModeFailoverFlag,
    &g_cfgEntry_SMSModeDevReadyFlag,
    &g_structConfigEnd //this must always be the last entry in a profile!
};

const ConfigProfile g_ServiceModeConfigProfile = g_ServiceModeConfigEntries;

/************************ END SECTION CONFIG PROFILES ************************/

static void config_load_entry(ConfigEntry entry) {
    
    switch(entry->type) 
    {
       case CONFIG_STRING:
        {
            ini_gets(entry->section,    //param Section     the name of the section to search for
                     entry->key,        //param Key         the name of the entry to find the value of
                     entry->defStr,     //param DefValue    default string in the event of a failed read
                     entry->memLoc,     //param Buffer      a pointer to the buffer to copy into
                     entry->memSize,    //param BufferSize  the maximum number of characters to copy
                     CONFIG_INI_FILE);  //param Filename    the name and full path of the .ini file to read from
        } break;
       case CONFIG_BIT_FLAG: //bit field flags used by config.h - entry creator must take caution to set correct offset
        {
            int flag = ini_getbool(
                     entry->section,    //param Section     the name of the section to search for
                     entry->key,        //param Key         the name of the entry to find the value of
                     0x1 & entry->defInt, //param DefValue  default value in the event of a failed read; it should
                                        //                  be zero (0) or one (1). And'd with 1 here  to ensure this.
                     CONFIG_INI_FILE);  //param Filename    the name and full path of the .ini file to read from
            char *bitField = (char *)(entry->memLoc);
            
            if(entry->memSize < 8) { //only supports 8bit fields!!
                if(flag) {                                              //here memSize represents the actual bit location
                    *bitField |= 1 << entry->memSize;  //memSize = 0 would represent a normal bool int
                } else {
                    *bitField &= ~(1 << entry->memSize);
                }
            }
        } break;
       case CONFIG_INT: //relies on two's compliment for truncation logic (NO SIGNED BITS!)
        {
            uint32_t retVal = (uint32_t)ini_getl(
                     entry->section,    //param Section     the name of the section to search for
                     entry->key,        //param Key         the name of the entry to find the value of
                     entry->defInt,     //param DefValue    default value in the event of a failed read
                     CONFIG_INI_FILE);  //param Filename    the name of the .ini file to read from
            
            switch (entry->memSize)
            {
               case 1: //char
                {
                    *(uint8_t*)(entry->memLoc) = 0xFF & retVal;
                } break;
               case 2: //short or int
                {
                    *(uint16_t*)(entry->memLoc) = 0xFFFF & retVal;
                } break;
               case 4: //int32 or long
                {
                    *(uint32_t*)(entry->memLoc) = retVal;
                } break;
               default:
                {
                    //64bits? unknown size type? don't bother
                } break;               
            }
        } break;
       case CONFIG_FLOAT:
        {
            *(float*)(entry->memLoc) = (float)ini_getl(
                     entry->section,    //param Section     the name of the section to search for
                     entry->key,        //param Key         the name of the entry to find the value of
                     entry->defInt,     //param DefValue    default value in the event of a failed read
                     CONFIG_INI_FILE);  //param Filename    the name of the .ini file to read from
        } break;
       case CONFIG_END:
       default:
        {
            //shouldn't arrive here. error handling?
        } break;
    }
    
    return;
}

static void config_load_entry_default(ConfigEntry entry) {
    
    switch(entry->type) 
    {
       case CONFIG_STRING:
        {
            char *buffer = (char *)entry->memLoc;
            buffer[0] = '\0';
            strncat(buffer, entry->defStr, entry->memSize); //append only up to memSize
        } break;
       case CONFIG_BIT_FLAG: //bit field flags used by config.h - entry creator must take caution to set correct offset
        {
            int flag = 0x1 & entry->defInt;
            char *bitField = (char *)(entry->memLoc);
            
            if(entry->memSize < 8) { //only supports 8bit fields!!
                if(flag) {                                              //here memSize represents the actual bit location
                    *bitField |= 1 << entry->memSize;  //memSize = 0 would represent a normal bool int
                } else {
                    *bitField &= ~(1 << entry->memSize);
                }
            }
        } break;
       case CONFIG_INT: //relies on two's compliment for truncation logic (NO SIGNED BITS!)
        {
            uint32_t retVal = (uint32_t)entry->defInt;
            
            switch (entry->memSize)
            {
               case 1: //char
                {
                    *(uint8_t*)(entry->memLoc) = 0xFF & retVal;
                } break;
               case 2: //short or int
                {
                    *(uint16_t*)(entry->memLoc) = 0xFFFF & retVal;
                } break;
               case 4: //int32 or long
                {
                    *(uint32_t*)(entry->memLoc) = retVal;
                } break;
               default:
                {
                    //64bits? unknown size type? don't bother
                } break;               
            }
        } break;
       case CONFIG_FLOAT:
        {
            *(float*)(entry->memLoc) = (float)entry->defInt;
        } break;
       case CONFIG_END:
       default:
        {
            //shouldn't arrive here. error handling?
        } break;
    }
    
    return;
}

void config_load_profile(ConfigProfile profile) {
    bool validIniFile = true;
    ConfigEntry entry;
	FILINFO fno;
    
    if(!profile) return;
    
    //TODO: SYSTEM_SWITCHES for buzzer from ini file
    if (!g_bFatInitialized)
    {
        if (g_bServiceMode == true)
        {
                lcd_printl(LINE1, SDCARD_FAILED);
   //sachin             lcd_printl(LINE2, "FAT Init Failed");
                delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
        }
        validIniFile = false;
    } else if (f_stat(CONFIG_INI_FILE, &fno) == FR_NO_FILE) {
        if (g_bServiceMode == true)
        {
                lcd_printl(LINE1, "No ini File");
       //sachin         lcd_printl(LINE2, "Loading Defaults");
                delay(HUMAN_DISPLAY_LONG_INFO_DELAY);
        }
        validIniFile = false;
    }
   
    entry = *profile++;
    while(entry->type != CONFIG_END) {
        validIniFile && entry->key[0] != '\0' ? config_load_entry(entry) : config_load_entry_default(entry);
        entry = *profile++;
    }
}
