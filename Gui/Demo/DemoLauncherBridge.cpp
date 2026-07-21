/**
 * DemoLauncherBridge implementation.
 * デモランチャーブリッジの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "DemoLauncherBridge.h"

void DemoLauncherInitEntries(AppState *State) {
	if (!State) {
		return;
	}
	State->Demo.EntryCount = 6;

	State->Demo.Entries[0].Id = 1;
	State->Demo.Entries[0].Category = DemoCategory_Rigid;

	State->Demo.Entries[1].Id = 2;
	State->Demo.Entries[1].Category = DemoCategory_Fluid;

	State->Demo.Entries[2].Id = 3;
	State->Demo.Entries[2].Category = DemoCategory_Fluid;

	State->Demo.Entries[3].Id = 4;
	State->Demo.Entries[3].Category = DemoCategory_Constraints;

	State->Demo.Entries[4].Id = 5;
	State->Demo.Entries[4].Category = DemoCategory_Soft;

	State->Demo.Entries[5].Id = 6;
	State->Demo.Entries[5].Category = DemoCategory_Particles;
}
