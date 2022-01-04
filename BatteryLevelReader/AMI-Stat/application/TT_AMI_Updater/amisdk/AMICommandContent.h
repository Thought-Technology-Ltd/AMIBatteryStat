/******************************************************************************
Project:	Advanced MyoTrac Infiniti (AMI)
SubProject:	AMICmds
File name:	AMICommandContent.h
Function:	Structure definitions for AMI command contents.

Date:	25/10/2017
********************************************************************************/
#ifndef AMICOMMANDCONTENT_H
#define AMICOMMANDCONTENT_H

#define AMI_MAX_ERROR_TYPES 32


typedef struct tagConnectionInfo
{
	char	 ID[248];
	TTL_BYTE	Reserved;
}
ConnectionInfo;


#  include "pshpack4.h"
typedef struct tagDeviceInfoC
{
	char	 ProductNumber[32];
	char	 SerialNumber[32];
	TTL_BYTE	ProductType;
	TTL_BYTE HardwareConfig;
	char	 FirmwarePNumber[32];
	char	 FirmwareVersion[32];
	char	 HardwareVersion[32];
	char	 ProtocolVersion[32];
	char	 SubMCUVersion[32];
}
DeviceInfoC;

typedef struct tagChannelStatus2
{
	TTL_BYTE Channel;
	TTL_BYTE Connected;
	TTL_UINT32 ID;
	TTL_BYTE Status;
}ChannelStatus2;

#  include "poppack.h"



typedef struct tagRestrictedDeviceInfoC
{
	char	ProductNumber[7];
	char	SerialNumber[9];
	TTL_BYTE ProductType;
	TTL_BYTE HardwareConfig;
	char  CableKey[40];
}RestrictedDeviceInfoC;



typedef struct tagDownloadInfo
{
	TTL_UINT16   index;
	TTL_EMGType format;
} DownloadInfo;



typedef struct tagSessionStatusChangedResponse
{
	TTL_SessionStatus Status;
	TTL_SessionStatusChangedCause Cause;
}
SessionStatusChangedResponse;

typedef struct tagSessionStatusChanged
{
	TTL_SessionStatus Status;
	TTL_SessionStatus PrevStatus;
	TTL_SessionStatusChangedCause Cause;
}
SessionStatusChanged;


typedef struct tagDeviceDateTime
{
	TTL_USHORT year;
	TTL_USHORT month;
	TTL_USHORT day;
	TTL_USHORT hour;
	TTL_USHORT minutes;
	TTL_USHORT seconds;
}
TTLDateTime;

typedef	struct tagDeviceScreenContrast
{
	TTL_BYTE Contrast;
}
DeviceScreenContrast;

typedef	struct tagDeviceSoundVolume
{
	TTL_BYTE Volume;
}
DeviceSoundVolume;


typedef struct tagConnectionConfig
{
	TTL_OperationMode OperationMode;
	TTL_EMGType EMGtype;
	TTL_NotchFrequency NotchFreq;
}
ConnectionConfig;

typedef struct tagBiofeedbackSessionParams
{
	TTL_USHORT RepNum;
	TTL_USHORT Work;
	TTL_USHORT Rest;

}BiofeedbackSessionParams;

//SESSION PARAMETERS
typedef struct tagStimSessionParameters
{
	TTL_USHORT		RepNumAB;
	TTL_StimMode	StimModeAB;
	TTL_USHORT		WorkAB;
	TTL_USHORT		RestAB;
	TTL_USHORT		RepNumCD;
	TTL_StimMode	StimModeCD;
	TTL_USHORT		WorkCD;
	TTL_USHORT		RestCD;

	//Channel A
	TTL_StimModality ModalityChA;
	TTL_StimPattern PatternChA;
	TTL_USHORT RampUpChA;
	TTL_USHORT RampDownChA;
	TTL_USHORT PulseRateChA;
	TTL_USHORT PulseWidthChA;
	TTL_StimAltPos AltPosChA;

	//Channel B
	TTL_StimModality ModalityChB;
	TTL_StimPattern PatternChB;
	TTL_USHORT RampUpChB;
	TTL_USHORT RampDownChB;
	TTL_USHORT PulseRateChB;
	TTL_USHORT PulseWidthChB;
	TTL_StimAltPos AltPosChB;
	
	//Channel C
	TTL_StimModality ModalityChC;
	TTL_StimPattern PatternChC;
	TTL_USHORT RampUpChC;
	TTL_USHORT RampDownChC;
	TTL_USHORT PulseRateChC;
	TTL_USHORT PulseWidthChC;
	TTL_StimAltPos AltPosChC;

	//Channel D
	TTL_StimModality ModalityChD;
	TTL_StimPattern PatternChD;
	TTL_USHORT RampUpChD;
	TTL_USHORT RampDownChD;
	TTL_USHORT PulseRateChD;
	TTL_USHORT PulseWidthChD;
	TTL_StimAltPos AltPosChD;

} StimSessionParameters;




//STIM PROGRAM PARAMETERS
typedef struct tagStimCustomProgramParameters
{
	TTL_USHORT ProgramPos;
	char  Name [129];
	StimSessionParameters Params;

}StimCustomProgramParameters;






typedef enum tagDualETSSequence
{
	ETS_SYNCRHONOUS=1,
	ETS_ALTERNATE
} DualETSSequence;

typedef enum tagETSChannel
{
	ETS_SINGLE_A =1,
	ETS_SINGLE_B ,
	ETS_DUAL
} ETSChannel;

typedef enum tagETSType
{
	ETS_INFINITE_WORK=1,
	ETS_FINITE_WORK
} ETSType;


typedef struct tagETSSessionParameters
{
	//TODO:update structure when finalized
	DualETSSequence Sequence;
	ETSChannel Channel;
	ETSType Type;
	TTL_USHORT RepetitionsA;
	TTL_USHORT RepetitionsB;
	TTL_USHORT	DurationA;//session duration in seconds
	TTL_USHORT	DurationB;//session duration in seconds
	TTL_USHORT RestDurationA;//(s)
	TTL_USHORT	StimDurationA;//(s)
	TTL_USHORT	WorkDurationA;//(s)
	TTL_USHORT	WorkDurationB;//(s)
	TTL_USHORT	RampupA;//(ms)
	TTL_USHORT	RampdownA;//(ms)
	TTL_USHORT	PulseRateA;
	TTL_USHORT	PulseWidthA;
	TTL_USHORT RestDurationB;//(s)
	TTL_USHORT	StimDurationB;//(s)
	TTL_USHORT	RampupB;//(ms)
	TTL_USHORT	RampdownB;//(ms)
	TTL_USHORT	PulseRateB;
	TTL_USHORT	PulseWidthB;
}ETSSessionParameters;


typedef struct tagRETSSessionParameters
{
	TTL_Channel	EMGCh;
	TTL_Channel	STIMCh;
	TTL_USHORT	Duration;//session duration in seconds
	TTL_USHORT  RestDuration;//(s)
	TTL_USHORT	StimDuration;//(s)
	TTL_USHORT	Rampup;//(ms)
	TTL_USHORT	Rampdown;//(ms)
	TTL_USHORT	PulseRate;
	TTL_USHORT	PulseWidth;
}RETSSessionParameters;

typedef	struct tagSessionVariables
{
	TTL_UINT32 SessionTimeAB;
	TTL_UINT32 SessionTimeCD;

	TTL_UINT32 PhaseTimeA;
	TTL_UINT32 PhaseTimeB;
	TTL_UINT32 PhaseTimeC;
	TTL_UINT32 PhaseTimeD;
	
	TTL_PhaseStatus PhaseStatusA;
	TTL_PhaseStatus PhaseStatusB;
	TTL_PhaseStatus PhaseStatusC;
	TTL_PhaseStatus PhaseStatusD;

	TTL_USHORT ThresholdA;
	TTL_USHORT ThresholdB;


	TTL_USHORT StimAmpMaxA;
	TTL_USHORT StimAmpMaxB;
	TTL_USHORT StimAmpMaxC;
	TTL_USHORT StimAmpMaxD;

	TTL_USHORT StimAmpLiveA;
	TTL_USHORT StimAmpLiveB;
	TTL_USHORT StimAmpLiveC;
	TTL_USHORT StimAmpLiveD;

	TTL_BOOL PatientDetectA;
	TTL_BOOL PatientDetectB;
	TTL_BOOL PatientDetectC;
	TTL_BOOL PatientDetectD;

	TTL_UINT32 TotalTrialsA;
	TTL_UINT32 CurrentTrialA;
	TTL_UINT32 SuccessfulTrialsA;

	TTL_UINT32 TotalTrialsB;
	TTL_UINT32 CurrentTrialB;
	TTL_UINT32 SuccessfulTrialsB;

}
SessionVariables;

typedef	struct tagAutoThreshold
{
	TTL_ThresholdType Channel;
	TTL_BOOL AutoThres;
}AutoThreshold;

typedef	struct tagThreshold
{
	TTL_ThresholdType Type;
	TTL_USHORT Threshold;
}ThresholdValue;

/*
typedef struct tagSessionType
{
	TTL_SessionType SessionType;
}SessionType;
*/

typedef struct tagSessionParametersC
{
	TTL_SessionType SessionType;
	TTL_BOOL Recording;
	char PatientID[32];
	TTL_USHORT MVC;
}SessionParametersC;

////////////////////////////////
//
//  EVENTS
//
////////////////////////////////

typedef struct tagSessionPhaseStatus
{
	TTL_PhaseStatus PrevPhase;
	TTL_PhaseStatus NewPhase;
}SessionPhaseStatus;


typedef struct tagSTIMSessionPhaseStatus
{
	TTL_Channel Channel;
	SessionPhaseStatus PhaseStatus;
}STIMSessionPhaseStatus;

typedef struct tagETSSessionPhaseStatus
{
	SessionPhaseStatus PhaseStatus;
}ETSSessionPhaseStatus;


typedef struct tagThresholdChanged
{
	TTL_Channel Channel;
	TTL_ThresholdType Type;
	TTL_USHORT Threshold;

}ThresholdChanged;

typedef struct tagSTIMAmplitudeChanged
{
	TTL_Channel Channel;
	TTL_UINT32 StimAmpMax;

}STIMAmplitudeChanged;

typedef struct tagPatientDetectStatusChanged
{
	TTL_Channel Channel;
}PatientDetectStatusChanged;

typedef struct tagChannelStatusChanged
{
	TTL_Channel Channel;
	TTL_BOOL Connected;
	TTL_USHORT ID;
	TTL_SensorStatus Status;
}ChannelStatusChanged;


typedef struct tagGetChannelStatus
{
	TTL_Channel Channel;
	TTL_BOOL Connected;
	TTL_USHORT ID;
	TTL_SensorStatus Status;
}ChannelStatus;









struct ChannelConfig
{
	TTL_UINT16 SampleRate;
	TTL_BOOL Active;
};

typedef struct tagSetAudioRequest
{
	TTL_BOOL Global;
	TTL_BOOL VoicePrompt;
	TTL_BOOL AudioChA;
	TTL_BOOL AudioChB;
}AudioRequest;

typedef struct tagSetAudioFeedbackTypeRequest
{
	TTL_CHAR Channel;
	TTL_AudioType AudioType;

}SetAudioFeedbackTypeRequest;

typedef struct tagGetBatteryStatusResponse
{
	TTL_BYTE SOC;//%
	TTL_USHORT Voltage;//mV
	TTL_BOOL Charging;
} BatteryStatus;


typedef struct tagGetSessionInformationRequest
{
	TTL_BYTE SessionIndex;
}GetSessionInformationRequest;

typedef struct tagGetSessionInformationResponse
{
	tagDeviceDateTime TimeDate;
	TTL_USHORT Duration;
	char PatientID[128];
	TTL_SessionType SessionType;
}SessionInformation;


typedef struct tagSessionMemoryStatus
{
	TTL_UINT32 StorageSize;
	TTL_UINT32 FreeSpace;
	TTL_UINT32 SessionNum;
}SessionMemoryStatus;


typedef struct tagGetSessionDataRequest
{
	TTL_BYTE SessionIndex;
}GetSessionDataRequest;

///////////////////////////////////
//
// Communication buffer commands
//
//

typedef struct tagSetBufferSpaceEventRequest
{
	TTL_BOOL Active;

}BufferSpaceEvent;

typedef struct tagBufferSpaceChanged
{
	TTL_UINT32 Timestamp;
	TTL_USHORT Latency;
	TTL_BYTE BufferFillPercent;

}BufferSpaceChanged;

///////////////////////////////////
//
// CRITIAL ERROR COMMANDS
//
//

typedef struct tagGetErrorLogResponse
{
	TTL_UINT16 Log[AMI_MAX_ERROR_TYPES];
}ErrorLog;

typedef struct tagCriticalErrorDetected
{
	TTLDateTime DateTime;
	TTL_CriticalErrorCode ErrorCode;
}CriticalErrorDetected;

///////////////////////////////////////////
//
// SELF-TEST
//
//

typedef struct tagDateResponse
{
	tagDeviceDateTime Date;
}DateResponse;


///////////////////////////////////////////
//
// BINARY STREAM DATA
//
//
typedef struct tagDataStreamSample
{
	TTL_BOOL HasChannelA;
	TTL_BOOL HasChannelB;
	TTL_INT16 ChannelAData;
	TTL_INT16 ChannelBData;

}DataStreamSample;

#define DATA_STREAM_SIZE sizeof(DataStream)
#pragma pack(push, 1)
typedef struct tagDataStream
{
	TTL_INT16 ContentMask;
	TTL_INT16 ChannelA;
	TTL_INT16 ChannelB;
	TTL_BYTE SequenceNumber;
	TTL_UINT16 Checksum;

}DataStream;
#pragma pack(pop)

///////////////////////////////////////////
//
// ASSETS
//
//

typedef struct tagAssetPackageHeader
{
	TTL_UINT32	PackageSize;
	TTL_UINT32	HeaderSize;
	TTL_UINT32	MemMapSize;
	TTL_UINT32	IndexSize;
	TTL_UINT32	BmpStructsSize;
	TTL_UINT32	WavStructsSize;
	TTL_UINT32	ImageDataSize;
	TTL_UINT32	SoundDataSize;
	TTL_BYTE	NbImages;
	TTL_BYTE	NbSounds;
	TTL_BYTE	PackageVersion;
	TTL_UINT32	Checksum;
}AssetPackageHeader;

typedef struct tagSelfTestResult
{
	TTL_BOOL	Passed;
	TTL_Channel	ChannelFault;
	TTL_ConfigFault ConfigFault;
}SelfTestResult;


///////////////////// DEBUG 
typedef struct tagJIG
{
	TTL_BYTE TestNumber;
	char Comment[32];
	TTL_BYTE  TestValue;
	TTL_UINT16  ResultValue;
} JIG;



typedef struct
{
	TTL_UINT32 cylesCount;
	TTL_UINT32 restDuration;
	TTL_UINT32 workDuration;
}BFBHeaderInfo;

typedef struct
{
	//ETS
	TTL_BYTE etsChannel;
	TTL_BYTE type;
	TTL_BYTE sequence;

	TTL_UINT32 sessionDurationA;
	TTL_UINT32 stimDurationA;
	TTL_UINT32 restDurationA;
	TTL_UINT32 workDurationA;
	TTL_UINT32 rampUpA;
	TTL_UINT32 rampDownA;
	TTL_UINT32 pulseRateA;
	TTL_UINT32 pulseWidthA;
	TTL_UINT32 nbRepsA;

	TTL_UINT32 sessionDurationB;
	TTL_UINT32 stimDurationB;
	TTL_UINT32 restDurationB;
	TTL_UINT32 workDurationB;
	TTL_UINT32 rampUpB;
	TTL_UINT32 rampDownB;
	TTL_UINT32 pulseRateB;
	TTL_UINT32 pulseWidthB;
	TTL_UINT32 nbRepsB;
}ETSHeaderInfo;

typedef struct
{
	//RETS
	TTL_BYTE chEMG;
	TTL_BYTE chSTIM;
	TTL_UINT32 sessionDuration;
	TTL_UINT32 stimDuration;
	TTL_UINT32 restDuration;
	TTL_UINT32 rampUp;
	TTL_UINT32 rampDown;
	TTL_UINT32 pulseRate;
	TTL_UINT32 pulseWidth;
}RETSHeaderInfo;

typedef struct TagMyonixHeaderInfo
{
	TTL_UINT32 sampleRate;
	TTL_UINT32 samplesCount_raw;
	TTL_UINT32 samplesCount_rms;
	TTL_UINT32 eventsCount;
	TTL_UINT32 sessionType;
	TTL_INT32 channelSensorsID[4];
	char patientID[129];
	long dateTime;
	unsigned char Modified;
	char serialNumber[10];
	char protocolVersion[16];

	union
	{
		BFBHeaderInfo BFB;
		ETSHeaderInfo ETS;
		RETSHeaderInfo RETS;
	}SessionParams;	
	TTL_BYTE productType;
	char reserved[15];
} MyonixHeaderInfo;





typedef struct tagSessionInfo
{	
	
	TTL_INT16 Index;
	TTL_UINT32 Size;
	MyonixHeaderInfo header;
	TTL_BOOL   Deleted;

}SessionInfo;



typedef struct tagUpdateResponse
{
	TTL_UINT32 offset;
	TTL_BYTE error_code;
	TTL_BYTE command;
} UpdateResponse;


#endif