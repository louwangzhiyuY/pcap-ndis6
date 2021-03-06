//////////////////////////////////////////////////////////////////////
// Project: pcap-ndis6
// Description: WinPCAP fork with NDIS6.x support 
// License: MIT License, read LICENSE file in project root for details
//
// Copyright (c) 2017 ChangeDynamix, LLC
// All Rights Reserved.
// 
// https://changedynamix.io/
// 
// Author: Mikhail Burilov
// 
// Based on original WinPcap source code - https://www.winpcap.org/
// Copyright(c) 1999 - 2005 NetGroup, Politecnico di Torino(Italy)
// Copyright(c) 2005 - 2007 CACE Technologies, Davis(California)
// Filter driver based on Microsoft examples - https://github.com/Microsoft/Windows-driver-samples
// Copyrithg(C) 2015 Microsoft
// All rights reserved.
//////////////////////////////////////////////////////////////////////

#include "filter.h"
#include "Adapter.h"
#include "Client.h"
#include "Device.h"
#include "Events.h"
#include "Packet.h"
#include "KernelUtil.h"
#include <flt_dbg.h>

//////////////////////////////////////////////////////////////////////
// Client methods
//////////////////////////////////////////////////////////////////////

PCLIENT CreateClient(PDEVICE device, PFILE_OBJECT fileObject)
{
	DEBUGP(DL_TRACE, "===>CreateClient...\n");
	CLIENT* client = FILTER_ALLOC_MEM(FilterDriverObject, sizeof(CLIENT));
	NdisZeroMemory(client, sizeof(CLIENT));

	client->Device = device;
	client->FileObject = fileObject;
	client->Event = CreateEvent();
	client->ReadLock = CreateSpinLock();
	client->BytesSent = 0;
	client->PacketList = CreateList();

	NET_BUFFER_LIST_POOL_PARAMETERS parameters;
	memset(&parameters, 0, sizeof(NET_BUFFER_LIST_POOL_PARAMETERS));

	parameters.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
	parameters.Header.Revision = NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
	parameters.Header.Size = NDIS_SIZEOF_NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
	parameters.ProtocolId = NDIS_PROTOCOL_ID_DEFAULT;
	parameters.fAllocateNetBuffer = TRUE;
	parameters.ContextSize = 32 + sizeof(UINT32) * 12;
	parameters.DataSize = MAX_PACKET_SIZE;
	parameters.PoolTag = FILTER_TAG;

	client->NetBufferListPool = NdisAllocateNetBufferListPool(NULL, &parameters);

	AddToList(device->ClientList, client);

	DEBUGP(DL_TRACE, "<===CreateClient\n");
	return client;
}

BOOL FreeClient(PCLIENT client)
{
	DEBUGP(DL_TRACE, "===>FreeClient...\n");
	if(!client)
	{
		return FALSE;
	}

	NdisAcquireSpinLock(client->ReadLock);

	FreePacketList(client->PacketList);
	RemoveFromListByData(client->Device->ClientList, client);
	client->PacketList = NULL;

	NdisReleaseSpinLock(client->ReadLock);

	FreeEvent(client->Event);
	FreeSpinLock(client->ReadLock);

	DEBUGP(DL_TRACE, "releasing net buffer list pool 0x%08x\n", client->NetBufferListPool);

	NdisFreeNetBufferListPool(client->NetBufferListPool);

	FILTER_FREE_MEM(client);

	DEBUGP(DL_TRACE, "<===FreeClient\n");
	return TRUE;
}

void FreeClientList(PLIST list)
{
	DEBUGP(DL_TRACE, "===>FreeClientList...\n");

	//Do not need to lock list because device is being closed anyways and there will be no new clients
	
	PLIST_ITEM item = list->First;
	while(item)
	{
		PCLIENT client = (PCLIENT)item->Data;
		if(client)
		{
			FreeClient(client);
		}
		item->Data = NULL;

		item = item->Next;
	}

	//TODO: possible memory leak if something is added to the list before it's released
	FreeList(list);
	DEBUGP(DL_TRACE, "<===FreeClientList\n");
}
