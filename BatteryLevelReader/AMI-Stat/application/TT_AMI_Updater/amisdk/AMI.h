/******************************************************************************
Project:	Advanced MyoTrac Infiniti (AMI)
SubProject:	AMICmds
File name:	AMI.h
Function:	All type definitions used by project.

Date:	25/10/2017
********************************************************************************/


#pragma once
#ifndef TTL_AMI_H
#define TTL_AMI_H

#ifdef AMI_EXPORTS
#ifdef __GNUC__
#define AMI_DEC_SPEC     __attribute__((dllexport))
#else
#define AMI_DEC_SPEC    __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define AMI_DEC_SPEC    __attribute__((dllimport))
#else
#define AMI_DEC_SPEC    __declspec(dllimport)
#endif
#endif


#define ENTRY( RET)  extern "C" AMI_DEC_SPEC  RET  __stdcall


#ifdef EXPORT_OBJ
#define AMICMDS_API  AMI_DEC_SPEC
#else
#define AMICMDS_API
#endif

 
typedef char TTL_CHAR;
typedef short TTL_SHORT;
typedef long TTL_LONG;
typedef unsigned char TTL_UCHAR;
typedef unsigned short TTL_USHORT;
typedef unsigned long TTL_ULONG;
typedef const TTL_CHAR* TTL_PCSTR;
typedef unsigned char TTL_BYTE;
typedef unsigned __int32  TTL_UINT32;
typedef unsigned __int64  TTL_UINT64;
typedef unsigned __int16  TTL_UINT16;
typedef __int16  TTL_INT16;
typedef __int32  TTL_INT32;
typedef __int64  TTL_INT64;
typedef unsigned char TTL_BOOL;

typedef TTL_UINT64 AMI_DEVICE_HANDLE;
typedef TTL_UINT64 CALL_RESULT;
//typedef TTL_UINT32 AMI_SESSION_HANDLE;



#define TTL_TRUE 1
#define TTL_FALSE 0

typedef TTL_INT16 TTL_ERROR_CODE;


typedef TTL_UINT64 CALL_RESULT;
#define MAKE_RESULT(error_code, status) ((((TTL_UINT64)error_code) << 32)|status)
#define TTL_ERROR_CODE(i64)  (static_cast<TTL_UINT32>(((i64)>>32)))
#define TTL_DEVICE_STATUS(i64)  (static_cast<TTL_UINT32>(((i64)&0xffffffff)))

//firmware return error codes
#define TTL_ERROR_NONE 0 //operation success

#define TTL_ERROR_INVALID_JSON 1
#define TTL_ERROR_INVALID_COMMAND_ID 2
#define TTL_ERROR_INVALID_PARAMETER 3
#define TTL_ERROR_INVALID_PARAMETER_VALUES 4
#define TTL_ERROR_INVALID_SESSION_CONFIGURATION 5
#define TTL_ERROR_UNSUPPORTED_OPERATION_MODE 6 
#define TTL_ERROR_FEATURE_UNSUPPORTED 7
#define TTL_ERROR_DEVICE_BUSY 8
#define TTL_ERROR_OPERATION_FAILURE 9
#define TTL_ERROR_CANNOT_CONNECT 10  //cannot etablish connection with the device


//API custom error code
#define TTL_TIMEOUT 101
#define TTL_STATUS_SET 102
#define TTL_INVALID_DEVICE_STATUS 103
#define TTL_INVALID_DEVICE_HANDLE 104
#define TTL_SENSOR_NOT_SUPPORTED 105
#define TTL_INVALID_UNIT_CONVERSION 106
#define TTL_ACQ_CONTEXT_NOT_READY 107
#define TTL_INVALID_CHANNEL 108
#define TTL_INVALID_SENSOR_CONNECTED 109
#define TTL_INVALID_SENSOR_SELECTED 110
#define TTL_DEVICE_NOT_FOUND 111
#define TTL_INVALID_SESSION_HANDLE 112
#define TTL_CANT_ADD_DEVICE 113
#define TTL_CHANNEL_NOT_ADDED 114
#define TTL_UNKNOWN_ERROR 115
#define TTL_DEVICE_INUSE 116
#define TTL_COMMAND_FAILED 117
#define TTL_DEVICE_OFFLINE 118
#define TTL_INDEX_OUT_OF_RANGE 119
#define TTL_COMM_ERROR  121


#define TTL_EVENT_DEVICE_CONNECTION  120
#define TTL_EVENT_SESION_STATUS_CHANGED 8





#define TTL_MAX_AMI_CHANNELS 4
#define TTL_MAX_AMI_STREAMING_CHANNELS 2

#define SAMPLING_RATE 2048
#define DOWN_SAMPLING 102
#define OUTPUT_RATE 20

#define BlUETOOTH_MAX_STREAM_PACKET_SIZE 520

#define NOTCH_FR_US 60.0
#define NOTCH_FR_EU 50.0
#define NOTCH_Q     10.0


#include "AMIenums.h"
#include "AMICommandContent.h"


#define LOG_SEQ_LOST  1
#define LOG_DEBUG 2
#define LOG_BIN 4
#define LOG_BLUETOOTH_RX 8
#define LOG_BLUETOOTH_TX 16
#define LOG_CHECKSUM_ERROR 32
#define LOG_LATENCY_ERROR 128
#define LOG_EVENT 512
#define LOG_RESPONSE 2048
#define LOG_REQUEST 8192
#define LOG_TIMEOUT    16384
#define OVERRIDE_CONVERSION  64
#define LOG_UNKNOWN_SLIP  256
#define OVERRIDE_COUNT_CONVERSION  32768

//////////////////////////////////////////////////////////////////////////////////
///////typedef   void (CALL_CONV* SESSION_CALLBACK) (AMI_SESSION_HANDLE, AMI_DEVICE_HANDLE, TTL_UINT32 event_id, void * data ,  void * );
typedef   void(__stdcall *  DEVICE_CALLBACK) (AMI_DEVICE_HANDLE, TTL_UINT32 event_id, void * data, void *);
typedef   void(__stdcall  * LOG_CALLBACK) (int type, const char * );
typedef   void(__stdcall  * DOWNLOAD_CALLBACK) (TTL_UINT32 batchNumber, const char * buffer, TTL_UINT32 buffer_size, void * param);
typedef   void(__stdcall  * UPDATE_CALLBACK) ( TTL_BYTE command, TTL_BYTE error_code, TTL_UINT32 offset, void * param);

	ENTRY(unsigned char) AMI_Init2();
	ENTRY(CALL_RESULT) AMI_Init();
	ENTRY(CALL_RESULT) AMI_End();
	ENTRY(unsigned char) AMI_End2();
	ENTRY(CALL_RESULT) AMI_SetScanFilter(const char *);
	ENTRY(CALL_RESULT) AMI_SetScanMode(TTL_ScanMode);
	ENTRY(CALL_RESULT) AMI_GetAvailableDevicesCount(TTL_UINT32 & count);
	ENTRY(unsigned char) AMI_GetAvailableDevicesCount2(TTL_UINT32 & count);
	ENTRY(CALL_RESULT) AMI_GetDeviceInfo(AMI_DEVICE_HANDLE, DeviceInfoC & device_info);
	ENTRY(CALL_RESULT) AMI_GetDeviceConnectionInfo(AMI_DEVICE_HANDLE, ConnectionInfo & );

	ENTRY(unsigned char) AMI_GetDeviceInfo2(TTL_UINT32, DeviceInfoC & device_info);

	ENTRY(CALL_RESULT) AMI_ResumeScan();
	ENTRY(CALL_RESULT) AMI_PauseScan();
	ENTRY(unsigned char) AMI_ResumeScan2();
	ENTRY(unsigned char) AMI_PauseScan2();

	ENTRY(void) AMI_SetCallBack(DEVICE_CALLBACK callback, void * param);
	ENTRY(CALL_RESULT) AMI_SetFlags(TTL_UINT32 flags);
	ENTRY(CALL_RESULT) AMI_GetFlags(TTL_UINT32 & flags);
	ENTRY(CALL_RESULT) AMI_SetLogCallback(LOG_CALLBACK callback);
	ENTRY(CALL_RESULT) AMI_GetChannelStatus(AMI_DEVICE_HANDLE device_handle, TTL_Channel channel, ChannelStatus & channel_status);
	ENTRY(unsigned char) AMI_GetChannelStatus2(TTL_UINT32 device_handle, TTL_BYTE channel, ChannelStatus2 & channel_status);

	ENTRY(CALL_RESULT) AMI_DeviceFlush(AMI_DEVICE_HANDLE);
	ENTRY(CALL_RESULT) AMI_DeviceEnableHeartBeat(AMI_DEVICE_HANDLE, TTL_BYTE);

	typedef   void(__stdcall  * HB_CALLBACK) (unsigned int, void *);
	//ENTRY(CALL_RESULT) AMI_DeviceSetHBCallback(AMI_DEVICE_HANDLE device_handle, HB_CALLBACK callback, void * param);

	ENTRY(CALL_RESULT) AMI_DeviceGetInfo(AMI_DEVICE_HANDLE handle, DeviceInfoC & info, TTL_BOOL force);
	ENTRY(CALL_RESULT) AMI_DeviceGetChannelStatus(AMI_DEVICE_HANDLE handle, TTL_Channel channel, ChannelStatus & ref, TTL_BOOL force);
	ENTRY(CALL_RESULT) AMI_DeviceGetStatus(AMI_DEVICE_HANDLE handle, TTL_DeviceStatus & status);
	ENTRY(CALL_RESULT) AMI_DeviceSetDateTime(AMI_DEVICE_HANDLE handle, const  TTLDateTime & datetime);
	ENTRY(CALL_RESULT) AMI_DeviceGetDateTime(AMI_DEVICE_HANDLE handle, TTLDateTime & datetime);
	ENTRY(CALL_RESULT) AMI_DeviceSetContrast(AMI_DEVICE_HANDLE handle, TTL_BYTE  contrast);
	ENTRY(CALL_RESULT) AMI_DeviceGetContrast(AMI_DEVICE_HANDLE handle, TTL_BYTE &  contrast);
	ENTRY(CALL_RESULT) AMI_DeviceSetBrightness(AMI_DEVICE_HANDLE handle, TTL_BYTE  brightness);
	ENTRY(CALL_RESULT) AMI_DeviceGetBrightness(AMI_DEVICE_HANDLE handle, TTL_BYTE &  brightness);

	ENTRY(CALL_RESULT) AMI_DeviceSetRecordingFlag(AMI_DEVICE_HANDLE handle, TTL_BYTE  flag);
	ENTRY(CALL_RESULT) AMI_DeviceGetRecordingFlag(AMI_DEVICE_HANDLE handle, TTL_BYTE &  flag);

	ENTRY(CALL_RESULT) AMI_DeviceStartStreaming(AMI_DEVICE_HANDLE handle);
	ENTRY(CALL_RESULT) AMI_DeviceStopStreaming(AMI_DEVICE_HANDLE handle);

	ENTRY(CALL_RESULT) AMI_DeviceGetBatteryStatus(AMI_DEVICE_HANDLE handle, BatteryStatus &  battery_status);
	ENTRY(CALL_RESULT) AMI_DeviceGetTemperature(AMI_DEVICE_HANDLE handle, TTL_INT16 &  temperature);

	ENTRY(CALL_RESULT) AMI_DeviceAvailableSamples(AMI_DEVICE_HANDLE handle, TTL_BYTE & mask, TTL_UINT32  *samples);
	ENTRY(CALL_RESULT) AMI_DeviceChannelData(AMI_DEVICE_HANDLE handle, TTL_Channel channel, TTL_UINT32 & read, float  *samples);
	ENTRY(CALL_RESULT) AMI_DeviceOpenConnection(AMI_DEVICE_HANDLE handle, TTL_OperationMode op_, TTL_EMGType emgtype_, TTL_NotchFrequency notch_);
	ENTRY(CALL_RESULT) AMI_DeviceCloseConnection(AMI_DEVICE_HANDLE handle);
	ENTRY(CALL_RESULT) AMI_DeviceUploadBioFeedBackSessionParameters(AMI_DEVICE_HANDLE handle, const BiofeedbackSessionParams & value, const char  * PatientID, TTL_BOOL record);
	ENTRY(CALL_RESULT) AMI_DeviceStartSession(AMI_DEVICE_HANDLE handle, const SessionParametersC & value);
	ENTRY(CALL_RESULT) AMI_DeviceStopSession(AMI_DEVICE_HANDLE handle);
	ENTRY(CALL_RESULT) AMI_DevicePauseSession(AMI_DEVICE_HANDLE handle);
	ENTRY(CALL_RESULT) AMI_DeviceResumeSession(AMI_DEVICE_HANDLE handle);
	ENTRY(CALL_RESULT) AMI_DeviceOpen(AMI_DEVICE_HANDLE handle);
	ENTRY(CALL_RESULT) AMI_DeviceClose(AMI_DEVICE_HANDLE handle);
	ENTRY(CALL_RESULT) AMI_DeviceEnableChannel(AMI_DEVICE_HANDLE handle, TTL_Channel channel, TTL_BOOL enable);
	ENTRY(CALL_RESULT) AMI_DeviceIsOnline(AMI_DEVICE_HANDLE handle, TTL_BOOL  & online);
	ENTRY(CALL_RESULT) AMI_DeviceSetSoundVolume(AMI_DEVICE_HANDLE handle, TTL_BYTE  value);
	ENTRY(CALL_RESULT) AMI_DeviceGetSoundVolume(AMI_DEVICE_HANDLE handle, TTL_BYTE & value);
	ENTRY(CALL_RESULT) AMI_DeviceSetAutoThreshold(AMI_DEVICE_HANDLE handle, TTL_ThresholdType channel, TTL_BOOL  value);
	ENTRY(CALL_RESULT) AMI_DeviceSetThresholdValue(AMI_DEVICE_HANDLE handle, TTL_ThresholdType type, TTL_USHORT value);
	ENTRY(CALL_RESULT) AMI_DeviceGetThresholdValue(AMI_DEVICE_HANDLE handle, TTL_ThresholdType type, TTL_USHORT & value);
	ENTRY(CALL_RESULT) AMI_DeviceGetConnectionConfig(AMI_DEVICE_HANDLE handle, ConnectionConfig & cc);
	ENTRY(CALL_RESULT) AMI_DeviceGetSessionType(AMI_DEVICE_HANDLE handle, TTL_SessionType & value);
	ENTRY(CALL_RESULT) AMI_DeviceGetSessionStatus(AMI_DEVICE_HANDLE handle, TTL_SessionStatus & value);
	ENTRY(CALL_RESULT) AMI_DeviceGetSessionVariables(AMI_DEVICE_HANDLE handle, SessionVariables & value);
	ENTRY(CALL_RESULT) AMI_DeviceGetStimSessionParameters(AMI_DEVICE_HANDLE handle, StimSessionParameters & value);
	ENTRY(CALL_RESULT) AMI_DeviceGetStimProgramParameters(AMI_DEVICE_HANDLE handle, TTL_BYTE index, StimCustomProgramParameters & value);
	ENTRY(CALL_RESULT) AMI_DeviceGetBioFeedBackSessionParameters(AMI_DEVICE_HANDLE handle, BiofeedbackSessionParams & value);
	ENTRY(CALL_RESULT) AMI_DeviceGetETSSessionParameters(AMI_DEVICE_HANDLE handle, ETSSessionParameters & value);
	ENTRY(CALL_RESULT) AMI_DeviceGetRETSSessionParameters(AMI_DEVICE_HANDLE handle, RETSSessionParameters & value);
	ENTRY(CALL_RESULT) AMI_DeviceUploadStimSessionParameters(AMI_DEVICE_HANDLE handle, const StimSessionParameters & value);
	ENTRY(CALL_RESULT) AMI_DeviceUploadETSSessionParameters(AMI_DEVICE_HANDLE handle, const ETSSessionParameters & value, const char  * PatientID, TTL_BOOL record);
	ENTRY(CALL_RESULT) AMI_DeviceUploadRETSSessionParameters(AMI_DEVICE_HANDLE handle, const RETSSessionParameters & value, const char  * PatientID, TTL_BOOL record);
	ENTRY(CALL_RESULT) AMI_DeviceSetAudio(AMI_DEVICE_HANDLE handle, const AudioRequest  & value);
	ENTRY(CALL_RESULT) AMI_DeviceSetAudioFeedBackType(AMI_DEVICE_HANDLE handle, const SetAudioFeedbackTypeRequest  & value);

	ENTRY(CALL_RESULT) AMI_DeviceGetSessionInformation(AMI_DEVICE_HANDLE handle, TTL_BYTE index, SessionInformation & value);
	ENTRY(CALL_RESULT) AMI_DeviceSetBufferSpaceEvent(AMI_DEVICE_HANDLE handle, TTL_BOOL   value);
	ENTRY(CALL_RESULT) AMI_DeviceGetErrorLog(AMI_DEVICE_HANDLE handle, TTL_UINT16 ErrorLog[]);
	ENTRY(CALL_RESULT) AMI_DeviceGetLastSelftestDate(AMI_DEVICE_HANDLE handle, TTLDateTime &  value);
	ENTRY(CALL_RESULT) AMI_DeviceLaunchSelfTest(AMI_DEVICE_HANDLE handle);
	ENTRY(CALL_RESULT) AMI_DeviceRebootIntoBootLoader(AMI_DEVICE_HANDLE handle);
	ENTRY(CALL_RESULT) AMI_DeviceUnlockRestrictedCommands(AMI_DEVICE_HANDLE handle, const char * value);
	ENTRY(CALL_RESULT) AMI_DeviceRestrictedSetDeviceInfo(AMI_DEVICE_HANDLE handle, const RestrictedDeviceInfoC  & value);
	ENTRY(CALL_RESULT) AMI_DeviceUploadCustomStimProgram(AMI_DEVICE_HANDLE handle, const StimCustomProgramParameters & value);
	ENTRY(CALL_RESULT) AMI_DeviceRestoreDefaultPrograms(AMI_DEVICE_HANDLE handle);
	ENTRY(CALL_RESULT) AMI_DeviceShutdown(AMI_DEVICE_HANDLE handle, TTL_BOOL reboot);




	//////// Session Storage
	ENTRY(CALL_RESULT) AMI_DeviceGetStoredSessionCount(AMI_DEVICE_HANDLE handle, TTL_UINT16 & count);
	ENTRY(CALL_RESULT) AMI_DeviceGetStoredSessionInfo(AMI_DEVICE_HANDLE handle, TTL_UINT16 index, SessionInfo & info);
	ENTRY(CALL_RESULT) AMI_DeviceStartSessionRecording(AMI_DEVICE_HANDLE handle, const char  * PatientID );
	ENTRY(CALL_RESULT) AMI_DeviceStopSessionRecording(AMI_DEVICE_HANDLE handle);
	ENTRY(CALL_RESULT) AMI_DeviceDeleteStoredSession(AMI_DEVICE_HANDLE handle, TTL_UINT16 index);
	ENTRY(CALL_RESULT) AMI_DeviceDownloadStoredSession(AMI_DEVICE_HANDLE handle, DownloadInfo  info, TTL_UINT32 & batchCount);
	ENTRY(CALL_RESULT) AMI_DeviceDownloadStoredSessionError(AMI_DEVICE_HANDLE handle, TTL_UINT32 batch);
	ENTRY(CALL_RESULT) AMI_DeviceDownloadStoredSessionEnd(AMI_DEVICE_HANDLE handle);
	ENTRY(CALL_RESULT) AMI_DeviceDownloadSetCallback(AMI_DEVICE_HANDLE handle, DOWNLOAD_CALLBACK, void * param);
	ENTRY(CALL_RESULT) AMI_DeviceDeleteAllStoredSessions(AMI_DEVICE_HANDLE handle);
	ENTRY(CALL_RESULT) AMI_DeviceGetSessionMemoryStatus(AMI_DEVICE_HANDLE handle, SessionMemoryStatus  & value);



	//////// DEBUG ONLY
	ENTRY(CALL_RESULT) AMI_DeviceSetDaisyChainOutput(AMI_DEVICE_HANDLE handle, TTL_BYTE value);
	ENTRY(CALL_RESULT) AMI_DeviceGetDaisyChainInputs(AMI_DEVICE_HANDLE handle, TTL_BYTE & value);

	ENTRY(CALL_RESULT) AMI_DeviceSetVariable(AMI_DEVICE_HANDLE handle, const char * var, TTL_INT32 value);
	ENTRY(CALL_RESULT) AMI_DeviceSendCustomCommand(AMI_DEVICE_HANDLE handle, const char * msg);
	ENTRY(CALL_RESULT) AMI_DeviceTestJIG(AMI_DEVICE_HANDLE handle, JIG & jig);
	ENTRY(CALL_RESULT) AMI_EnableServiceMode();
	ENTRY(TTL_BOOL) AMI_IsInServiceMode();




	///////////  UPDATE & UPGRADE
	ENTRY(CALL_RESULT) AMI_WriteUpdateBatch(AMI_DEVICE_HANDLE handle, TTL_UINT32 offset, const TTL_BYTE * data, size_t datalen, UpdateResponse & update_response, TTL_UINT32 timeout);
	ENTRY(CALL_RESULT) AMI_WriteUpgradeKey(AMI_DEVICE_HANDLE device_handle, const char * key, TTL_UINT32 len, UpdateResponse & update_response, TTL_UINT32 timeout);


	///////  Session Recording utils
	typedef enum 
	{
		EXPORT_BGI
	} MyonixExportType;

	typedef void  (__stdcall * ExportProgressCallback) ( int  , void * );
	ENTRY(CALL_RESULT) AMI_BeginRecordedSessionsEnum(const wchar_t * path);
	ENTRY(CALL_RESULT) AMI_EndRecordedSessionsEnum();
	ENTRY(CALL_RESULT) AMI_GetRecordedSessionInfo(TTL_UINT32 index, MyonixHeaderInfo * & headerinfo, wchar_t * & fileName);
	ENTRY(CALL_RESULT) AMI_GetRecordedSessionsCount(TTL_UINT32 & count);
	ENTRY(CALL_RESULT) AMI_ExportRecordedSession(wchar_t *  srcFile, wchar_t *  dstFile, MyonixExportType exp, TTL_UINT32 & exported_samples, ExportProgressCallback callback, void * param, TTL_INT32 sensorIDA, TTL_INT32 sensorIDB);
	ENTRY(CALL_RESULT) AMI_GetSessionFileInfo(wchar_t * fileName, MyonixHeaderInfo *  headerinfo);

	///////////////////////// upload
	ENTRY(CALL_RESULT) AMI_UploadFileStart(AMI_DEVICE_HANDLE device_handle, TTL_BYTE ID, const char * fileName, TTL_UINT32 size, TTL_UINT16 batchCount, TTL_UINT32 cs);
	ENTRY(CALL_RESULT) AMI_UploadFileCancel(AMI_DEVICE_HANDLE device_handle, TTL_BYTE ID);
	ENTRY(CALL_RESULT) AMI_UploadFileWriteBatch(AMI_DEVICE_HANDLE device_handle, TTL_BYTE ID, TTL_UINT16 batchNumber, TTL_BYTE *buffer, TTL_UINT16 size, TTL_BYTE & error_code);


	//////// vichy specifics
	ENTRY(CALL_RESULT) AMI_IncrementAmplitud(AMI_DEVICE_HANDLE, TTL_Channel);
	ENTRY(CALL_RESULT) AMI_DecrementAmplitud(AMI_DEVICE_HANDLE, TTL_Channel);



#endif
