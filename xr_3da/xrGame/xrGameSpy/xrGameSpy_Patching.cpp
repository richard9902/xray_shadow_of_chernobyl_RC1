#include "stdafx.h"
#include "windows.h"
#include "xrGameSpy_MainDefs.h"

#include "xrGameSpy_Patching.h"

XRGAMESPY_API bool xrGS_ptCheckForPatch(
										//int productID,  const gsi_char * versionUniqueID,  int distributionID, 
										ptPatchCallback callback, 
										PTBool blocking, 
										void * instance )
{
//	return ptCheckForPatch(productID, versionUniqueID, distributionID, callback, blocking, instance )!=PTFalse;
	return ptCheckForPatch(GAMESPY_PRODUCTID, 
		GAME_VERSION,
//		GAMESPY_PATCHING_VERSIONUNIQUE_ID, 
		GAMESPY_PATCHING_DISTRIBUTION_ID, callback, blocking, instance )!=PTFalse;
};