// Copyright © 2018 nekoatsume_atsuko. All rights reserved.

#include "StatusTriggerChecker.h"
#include "bishrpg.h"

void UStatusTriggerChecker::Initialize()
{
	CurrentStatusMap.Reset();
	AddedStatusEvent.Clear();
	RemovedStatusEvent.Clear();
}

bool UStatusTriggerChecker::Check(const FBattleObjectHandle& handle, int32 currentStatus)
{
	uint32  handleHash = handle.CalcHash();
	int32*  prevStatus = CurrentStatusMap.Find(handleHash);
	if(!prevStatus) {
		CurrentStatusMap.Emplace(handleHash, currentStatus);
	}

	if(*prevStatus == currentStatus) {
		return false;
	}

	// ステータスフラグの差分をチェックしてデリゲートを呼び出す
	auto callEvent = [currentStatus, prevStatus, handle](int32 checkStatus, FBattleChangedStatusDelegate& callDelegate) {
		const uint32 diffStatus      = (currentStatus ^ *prevStatus) & checkStatus;
		uint32       checkDiffStatus = diffStatus;
		uint32       shiftCount      = 0;
		while(checkDiffStatus != 0) {
			if(checkDiffStatus & 0x1) {
				const EStatusFlag status = static_cast<EStatusFlag>(0x1 << shiftCount);
				callDelegate.Broadcast(handle, status);
			}
			checkDiffStatus = checkDiffStatus >> 1;
			++shiftCount;
		}
	};
	callEvent(currentStatus, AddedStatusEvent);
	callEvent(*prevStatus, RemovedStatusEvent);

	return true;
}
