/*
 *Copyright © 2008 Silicondust USA Inc. <www.silicondust.com>.
 */

#ifndef DEVICES_H
#define DEVICES_H

class THDHomeRunDevice {
public:
	THDHomeRunDevice(string Name, uint32_t DeviceID, uint8_t TunerIndex);
	~THDHomeRunDevice();

	THDHomeRunDevice *Next;
	string Name;
	string Model;
	uint32_t DeviceID;
	uint8_t TunerIndex;
	bool Discovered;
	struct hdhomerun_device_t *hd;

	void ResolveModel();
};

class THDHomeRunDeviceList {
protected:
	GRWLock Lock;
	THDHomeRunDevice *DeviceList;
	uint16_t FFoundErrorCode;

	THDHomeRunDevice *AddDeviceInternal(string Name, uint32_t DeviceID, uint8_t TunerIndex);
	THDHomeRunDevice *AddDevice(string Name, uint32_t DeviceID, uint8_t TunerIndex);
	THDHomeRunDevice *AddDevice(uint32_t DeviceID, uint8_t TunerIndex);
	THDHomeRunDevice *AddDevice(string DeviceName);

	void DetectDiscoverTuners(uint32_t DeviceID, uint8_t TunerCount);
	void DetectDiscover();
	void DetectEntry();
	void DetectCleanup();

public:
	 THDHomeRunDeviceList();
	~THDHomeRunDeviceList();

	uint16_t FoundErrorCode();
	uint32_t DiscoveredCount();
	void ResolveModels();
	string ModelCommon();

	THDHomeRunDevice *Find(uint32_t DeviceID, uint8_t TunerIndex);
	THDHomeRunDevice *Find(string Name);
	THDHomeRunDevice *IterateFirst();
	THDHomeRunDevice *IterateNext(THDHomeRunDevice *Dev);
	void IterateComplete();
	void IterateExceptionCleanup();

	void Detect();
};
#endif
