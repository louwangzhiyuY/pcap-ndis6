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

#pragma once

#include <ndis.h>
#include <minwindef.h>
#include "List.h"

//////////////////////////////////////////////////////////////////////
// Adapter definitions
//////////////////////////////////////////////////////////////////////

extern PLIST AdapterList;

typedef struct ADAPTER {
	char AdapterId[1024];
	PNDIS_STRING Name;
	char DisplayName[1024];
	UCHAR MacAddress[NDIS_MAX_PHYS_ADDRESS_LENGTH];
	ULONG MtuSize;
	NDIS_HANDLE AdapterHandle;
	PNDIS_SPIN_LOCK Lock;

	struct DEVICE* Device;

	NDIS_HANDLE BindContext;   // To complete Bind request if necessary
	NDIS_HANDLE UnbindContext; // To complete Unbind request if necessary

	BOOL Ready;

	volatile ULONG PendingOidRequests;
	volatile ULONG PendingSendPackets;
} ADAPTER;
typedef const ADAPTER *PADAPTER;

//////////////////////////////////////////////////////////////////////
// Adapter methods
//////////////////////////////////////////////////////////////////////

BOOL SendOidRequest(PADAPTER adapter, BOOL set, NDIS_OID oid, void *data, UINT size);
BOOL FreeAdapter(ADAPTER* adapter);
BOOL FreeAdapterList(PLIST list);

//////////////////////////////////////////////////////////////////////
// Adapter callbacks
//////////////////////////////////////////////////////////////////////

NDIS_STATUS Protocol_BindAdapterHandlerEx(NDIS_HANDLE ProtocolDriverContext, NDIS_HANDLE BindContext, PNDIS_BIND_PARAMETERS BindParameters);
NDIS_STATUS Protocol_UnbindAdapterHandlerEx(NDIS_HANDLE UnbindContext, NDIS_HANDLE ProtocolBindingContext);
void Protocol_OpenAdapterCompleteHandlerEx(NDIS_HANDLE ProtocolBindingContext, NDIS_STATUS Status);
void Protocol_CloseAdapterCompleteHandlerEx(NDIS_HANDLE ProtocolBindingContext);
void Protocol_OidRequestCompleteHandler(NDIS_HANDLE ProtocolBindingContext, NDIS_OID_REQUEST *OidRequest, NDIS_STATUS Status);
void Protocol_ReceiveNetBufferListsHandler(NDIS_HANDLE ProtocolBindingContext, PNET_BUFFER_LIST NetBufferLists, NDIS_PORT_NUMBER PortNumber, ULONG NumberOfNetBufferLists, ULONG ReceiveFlags);
void Protocol_SendNetBufferListsCompleteHandler(NDIS_HANDLE ProtocolBindingContext, PNET_BUFFER_LIST NetBufferList, ULONG SendCompleteFlags);