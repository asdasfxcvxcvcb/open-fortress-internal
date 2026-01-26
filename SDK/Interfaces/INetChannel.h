#pragma once

#include <cstdint>

// Forward declarations
class INetMessage;
class INetMessageBinder;
class bf_read;
class bf_write;

// INetChannelInfo - read-only interface for network channel information
class INetChannelInfo
{
public:
	virtual const char* GetName(void) const = 0;
	virtual const char* GetAddress(void) const = 0;
	virtual float GetTime(void) const = 0;
	virtual float GetTimeConnected(void) const = 0;
	virtual int GetBufferSize(void) const = 0;
	virtual int GetDataRate(void) const = 0;
	virtual bool IsLoopback(void) const = 0;
	virtual bool IsTimingOut(void) const = 0;
	virtual bool IsPlayback(void) const = 0;
	virtual float GetLatency(int flow) const = 0;
	virtual float GetAvgLatency(int flow) const = 0;
	virtual float GetAvgLoss(int flow) const = 0;
	virtual float GetAvgChoke(int flow) const = 0;
	virtual float GetAvgData(int flow) const = 0;
	virtual float GetAvgPackets(int flow) const = 0;
	virtual int GetTotalData(int flow) const = 0;
	virtual int GetSequenceNr(int flow) const = 0;
	virtual bool IsValidPacket(int flow, int frame_number) const = 0;
	virtual float GetPacketTime(int flow, int frame_number) const = 0;
	virtual int GetPacketBytes(int flow, int frame_number, int group) const = 0;
	virtual bool GetStreamProgress(int flow, int* received, int* total) const = 0;
	virtual float GetTimeSinceLastReceived(void) const = 0;
	virtual float GetCommandInterpolationAmount(int flow, int frame_number) const = 0;
	virtual void GetPacketResponseLatency(int flow, int frame_number, int* pnLatencyMsecs, int* pnChoke) const = 0;
	virtual void GetRemoteFramerate(float* pflFrameTime, float* pflFrameTimeStdDeviation) const = 0;
	virtual float GetTimeoutSeconds(void) const = 0;
};

// INetChannel extends INetChannelInfo and provides access to internal data
// CNetChan vtable: 0x1032A2D4 (engine.dll)
// CNetChan::Transmit: 0x101CB380 (verified in IDA Pro)
class INetChannel : public INetChannelInfo
{
public:
	// INetChannel specific methods (after INetChannelInfo virtuals)
	virtual void SetDataRate(float rate) = 0;
	virtual bool RegisterMessage(INetMessageBinder* msg) = 0;
	virtual bool StartStreaming(unsigned int challengeNr) = 0;
	virtual void ResetStreaming(void) = 0;
	virtual void SetTimeout(float seconds) = 0;
	virtual void SetDemoRecorder(void* recorder) = 0;
	virtual void SetChallengeNr(unsigned int chal) = 0;
	virtual void Reset(void) = 0;
	virtual void Clear(void) = 0;
	virtual void Shutdown(const char* reason) = 0;
	virtual void ProcessPlayback(void) = 0;
	virtual bool ProcessStream(void) = 0;
	virtual void ProcessPacket(struct netpacket_s* packet, bool bHasHeader) = 0;
	virtual bool SendNetMsg(INetMessage& msg, bool bForceReliable = false, bool bVoice = false) = 0;
	virtual bool SendData(bf_write& msg, bool bReliable = true) = 0;
	virtual bool SendFile(const char* filename, unsigned int transferID) = 0;
	virtual void DenyFile(const char* filename, unsigned int transferID) = 0;
	virtual void RequestFile_OLD(const char* filename, unsigned int transferID) = 0;
	virtual void SetChoked(void) = 0;
	virtual int SendDatagram(bf_write* data) = 0;
	virtual bool Transmit(bool onlyReliable = false) = 0;
	virtual const unsigned char* GetChannelEncryptionKey(void) const = 0;

	// Access to internal CNetChan data members (verified in IDA Pro at 0x101C75E0)
	// Structure layout from CNetChan constructor:
	// +0x00: vtable pointer
	// +0x08: m_nOutSequenceNr (int) - initialized to 1 at 0x101C77B2
	// +0x0C: m_nInSequenceNr (int) - initialized to 0 at 0x101C77B9
	// +0x10: m_nOutSequenceNrAck (int) - initialized to 0 at 0x101C77C0
	// +0x14: m_nOutReliableState (int) - initialized to 0 at 0x101C77C7
	// +0x18: m_nInReliableState (int) - initialized to 0 at 0x101C77CE
	
	inline int& GetOutSequenceNr() {
		return *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0x8);
	}
	
	inline int& GetInSequenceNr() {
		return *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0xC);
	}
	
	inline int& GetOutSequenceNrAck() {
		return *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0x10);
	}
	
	inline int& GetOutReliableState() {
		return *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0x14);
	}
	
	inline int& GetInReliableState() {
		return *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0x18);
	}
};
