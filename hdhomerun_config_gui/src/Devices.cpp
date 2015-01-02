/*
 * Copyright © 2008 Silicondust USA Inc. <www.silicondust.com>.
 */

#include "AppInclude.h"

THDHomeRunDevice::THDHomeRunDevice(string Name, uint32_t DeviceID, uint8_t TunerIndex)
{
	this->Name = Name;
	this->DeviceID = DeviceID;
	this->TunerIndex = TunerIndex;

	Next = NULL;
	Model = "";
	Discovered = false;

	hd = hdhomerun_device_create(DeviceID, 0, TunerIndex, dbg);
}

THDHomeRunDevice::~THDHomeRunDevice()
{
	if (hd) {
		hdhomerun_device_destroy(hd);
	}
}

void THDHomeRunDevice::ResolveModel()
{
	if (!Model.empty()) {
		return;
	}

	if (!Discovered || !hd) {
		return;
	}

	const char *cModel = hdhomerun_device_get_model_str(hd);
	if (!cModel) {
		return;
	}

	Model = cModel;
}

THDHomeRunDeviceList::THDHomeRunDeviceList()
{
	g_rw_lock_init(&Lock);
	DeviceList = NULL;
	FFoundErrorCode = 0;
}

THDHomeRunDeviceList::~THDHomeRunDeviceList()
{
}

uint16_t THDHomeRunDeviceList::FoundErrorCode()
{
	g_rw_lock_reader_lock(&Lock);
	uint16_t Result = FFoundErrorCode;
	g_rw_lock_reader_unlock(&Lock);
	return Result;
}

guint32 THDHomeRunDeviceList::DiscoveredCount()
{
	try {
		int Count = 0;
		THDHomeRunDevice *Dev = IterateFirst();
		while (Dev) {
			if (Dev->Discovered) {
				Count++;
			}
			Dev = IterateNext(Dev);
		}
		IterateComplete();
		return Count;
	}
	catch(...) {
		IterateExceptionCleanup();
		return 0;
	}
}

void THDHomeRunDeviceList::ResolveModels()
{
	try {
		THDHomeRunDevice *Dev = IterateFirst();
		while (Dev) {
			Dev->ResolveModel();
			Dev = IterateNext(Dev);
		}
		IterateComplete();
	}
	catch (...) {
		IterateExceptionCleanup();
	}
}

string THDHomeRunDeviceList::ModelCommon()
{
	string CommonModel = "";
	try {
		THDHomeRunDevice *Dev = IterateFirst();
		while (Dev) {
			if (!Dev->Discovered || Dev->Model.empty()) {
				Dev = IterateNext(Dev);
				continue;
			}

			if (CommonModel.empty()) {
				CommonModel = Dev->Model;
				Dev = IterateNext(Dev);
				continue;
			}

			if (CommonModel != Dev->Model) {
				CommonModel = "";
				break;
			}

			Dev = IterateNext(Dev);
		}
		IterateComplete();
		return CommonModel;
	}
	catch(...) {
		IterateExceptionCleanup();
		return "";
	}
}

THDHomeRunDevice *THDHomeRunDeviceList::Find(uint32_t DeviceID, uint8_t TunerIndex)
{
	try {
		THDHomeRunDevice *Dev = IterateFirst();
		while (Dev) {
			if ((Dev->DeviceID == DeviceID) && (Dev->TunerIndex == TunerIndex)) {
				break;
			}
			Dev = IterateNext(Dev);
		}

		IterateComplete();
		return Dev;
	}
	catch(...) {
		IterateExceptionCleanup();
		return NULL;
	}
}

THDHomeRunDevice *THDHomeRunDeviceList::Find(string Name)
{
	try {
		THDHomeRunDevice *Dev = IterateFirst();
		while (Dev) {
			if (Dev->Name == Name) {
				break;
			}
			Dev = IterateNext(Dev);
		}
		IterateComplete();
		return Dev;
	}
	catch(...) {
		IterateExceptionCleanup();
		return NULL;
	}
}

THDHomeRunDevice *THDHomeRunDeviceList::IterateFirst()
{
	g_rw_lock_writer_lock(&Lock);
	return DeviceList;
}

THDHomeRunDevice *THDHomeRunDeviceList::IterateNext(THDHomeRunDevice * Dev)
{
	return Dev->Next;
}

void THDHomeRunDeviceList::IterateComplete()
{
	g_rw_lock_writer_unlock(&Lock);
}

void THDHomeRunDeviceList::IterateExceptionCleanup()
{
	g_rw_lock_writer_unlock(&Lock);
}

THDHomeRunDevice *THDHomeRunDeviceList::AddDeviceInternal(string Name, uint32_t DeviceID, guint8 TunerIndex)
{
	THDHomeRunDevice *Prev = NULL;
	THDHomeRunDevice *P = DeviceList;

	while (P) {
		if (P->DeviceID < DeviceID) {
			Prev = P;
			P = P->Next;
			continue;
		}
		if (P->DeviceID > DeviceID) {
			break;
		}

		if (P->TunerIndex < TunerIndex) {
			Prev = P;
			P = P->Next;
			continue;
		}
		if (P->TunerIndex > TunerIndex) {
			break;
		}

		return P;
	}

	THDHomeRunDevice *Dev = new THDHomeRunDevice(Name, DeviceID, TunerIndex);

	if (Prev) {
		Dev->Next = P;
		Prev->Next = Dev;
	} else {
		Dev->Next = DeviceList;
		DeviceList = Dev;
	}

	return Dev;
}

THDHomeRunDevice *THDHomeRunDeviceList::AddDevice(string Name, guint32 DeviceID, guint8 TunerIndex)
{
	if (DeviceID == HDHOMERUN_DEVICE_ID_WILDCARD) {
		hdhomerun_debug_printf(dbg, "devices: invalid DeviceID %08lx\n", DeviceID);
		FFoundErrorCode = 0x4001;
		return NULL;
	}
	if (!hdhomerun_discover_validate_device_id(DeviceID)) {
		hdhomerun_debug_printf(dbg, "devices: invalid DeviceID %08lx\n", DeviceID);
		FFoundErrorCode = 0x4002;
		return NULL;
	}

	g_rw_lock_writer_lock(&Lock);

	THDHomeRunDevice *Dev = NULL;
	try {
		Dev = AddDeviceInternal(Name, DeviceID, TunerIndex);
	}
	catch(...) {
		hdhomerun_debug_printf(dbg, "devices: AddDevice exception!\n");
	}

	g_rw_lock_writer_unlock(&Lock);
	return Dev;
}

THDHomeRunDevice *THDHomeRunDeviceList::AddDevice(uint32_t DeviceID, uint8_t TunerIndex)
{
	char cName[16];
	snprintf(cName, sizeof(cName), "%08lX-%u", (unsigned long)DeviceID, TunerIndex);
	return AddDevice((string)cName, DeviceID, TunerIndex);
}

THDHomeRunDevice *THDHomeRunDeviceList::AddDevice(string DeviceName)
{
	uint32_t DeviceID;
	uint8_t TunerIndex;

	if (sscanf(DeviceName.c_str(), "%x-%hhu", &DeviceID, &TunerIndex) != 2) {
		return NULL;
	}

	return AddDevice(DeviceID, TunerIndex);
}

void THDHomeRunDeviceList::DetectDiscoverTuners(uint32_t DeviceID, uint8_t TunerCount)
{
	if (TunerCount == 0) {
		hdhomerun_debug_printf(dbg, "devices: %08lX: DetectDiscoverTuners tuner count = 0", DeviceID);

		char cName[16];
		snprintf(cName, sizeof(cName), "%08lX", (unsigned long)DeviceID);
		THDHomeRunDevice *Dev = AddDevice((string)cName, DeviceID, 0);
		if (!Dev) {
			return;
		}

		Dev->Discovered = true;
		return;
	}

	for (int TunerIndex = 0; TunerIndex < TunerCount; TunerIndex++) {
		hdhomerun_debug_printf(dbg, "devices: %08lX: DetectDiscoverTuners found tuner %u", DeviceID, TunerIndex);

		THDHomeRunDevice *Dev = AddDevice(DeviceID, TunerIndex);
		if (!Dev) {
			continue;
		}

		Dev->Discovered = true;
	}
}

void THDHomeRunDeviceList::DetectDiscover()
{
	struct hdhomerun_discover_device_t ResultList[64];
	int Count = hdhomerun_discover_find_devices_custom(0, HDHOMERUN_DEVICE_TYPE_TUNER, HDHOMERUN_DEVICE_ID_WILDCARD, ResultList, 64);
	if (Count < 0) {
		hdhomerun_debug_printf(dbg, "devices: DetectDiscover error finding devices\n");
		return;
	}
	if (Count == 0) {
		hdhomerun_debug_printf(dbg, "devices: DetectDiscover no devices found\n");
		return;
	}

	struct hdhomerun_discover_device_t *Result = ResultList;
	struct hdhomerun_discover_device_t *ResultEnd = ResultList + Count;
	while (Result < ResultEnd) {
		/* Detect the number of tuners and add a device record for each one. */
		DetectDiscoverTuners(Result->device_id, Result->tuner_count);
		Result++;
	}
}

void THDHomeRunDeviceList::DetectEntry()
{
	g_rw_lock_writer_lock(&Lock);

	THDHomeRunDevice *Dev = DeviceList;
	while (Dev) {
		Dev->Discovered = false;
		Dev = Dev->Next;
	}

	FFoundErrorCode = 0;

	g_rw_lock_writer_unlock(&Lock);
}

void THDHomeRunDeviceList::DetectCleanup()
{
	g_rw_lock_writer_lock(&Lock);

	THDHomeRunDevice *Prev = NULL;
	THDHomeRunDevice *Dev = DeviceList;
	while (Dev) {
		if (Dev->Discovered) {
			Prev = Dev;
			Dev = Dev->Next;
			continue;
		}

		if (Prev) {
			Prev->Next = Dev->Next;
			Dev = Dev->Next;
		} else {
			DeviceList = Dev->Next;
			Dev = Dev->Next;
		}
	}

	g_rw_lock_writer_unlock(&Lock);
}

void THDHomeRunDeviceList::Detect()
{
	DetectEntry();
	DetectDiscover();
	DetectCleanup();
}
