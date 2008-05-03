/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra_v2.h"
#include "kyra/screen_v2.h"
#include "kyra/timer.h"

#include "common/endian.h"

namespace Kyra {

int KyraEngine_v2::o2_getCharacterX(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_getCharacterX(%p) ()", (const void *)script);
	return _mainCharacter.x1;
}

int KyraEngine_v2::o2_getCharacterY(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_getCharacterY(%p) ()", (const void *)script);
	return _mainCharacter.y1;
}

int KyraEngine_v2::o2_getCharacterFacing(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_getCharacterFacing(%p) ()", (const void *)script);
	return _mainCharacter.facing;
}

int KyraEngine_v2::o2_getCharacterScene(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_getCharacterScene(%p) ()", (const void *)script);
	return _mainCharacter.sceneId;
}

int KyraEngine_v2::o2_trySceneChange(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_trySceneChange(%p) (%d, %d, %d, %d)", (const void *)script,
			stackPos(0), stackPos(1), stackPos(2), stackPos(3));

	_unkHandleSceneChangeFlag = 1;
	int success = inputSceneChange(stackPos(0), stackPos(1), stackPos(2), stackPos(3));
	_unkHandleSceneChangeFlag = 0;

	if (success) {
		_emc->init(script, script->dataPtr);
		_unk4 = 0;
		_unk3 = -1;
		_unk5 = 1;
		return 0;
	} else {
		return (_unk4 != 0) ? 1 : 0;
	}
}

int KyraEngine_v2::o2_moveCharacter(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_moveCharacter(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	moveCharacter(stackPos(0), stackPos(1), stackPos(2));
	return 0;
}

int KyraEngine_v2::o2_queryGameFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_queryGameFlag(%p) (%d)", (const void *)script, stackPos(0));
	return queryGameFlag(stackPos(0));
}

int KyraEngine_v2::o2_resetGameFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_resetGameFlag(%p) (%d)", (const void *)script, stackPos(0));
	return resetGameFlag(stackPos(0));
}

int KyraEngine_v2::o2_setGameFlag(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_setGameFlag(%p) (%d)", (const void *)script, stackPos(0));
	return setGameFlag(stackPos(0));
}

int KyraEngine_v2::o2_setHandItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_setHandItem(%p) (%d)", (const void *)script, stackPos(0));
	setHandItem(stackPos(0));
	return 0;
}

int KyraEngine_v2::o2_removeHandItem(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_removeHandItem(%p) ()", (const void *)script);
	removeHandItem();
	return 0;
}

int KyraEngine_v2::o2_handItemSet(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_handItemSet(%p) ()", (const void *)script);
	return _handItemSet;
}

int KyraEngine_v2::o2_hideMouse(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_hideMouse(%p) ()", (const void *)script);
	screen()->hideMouse();
	return 0;
}

int KyraEngine_v2::o2_addSpecialExit(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_addSpecialExit(%p) (%d, %d, %d, %d, %d)", (const void *)script,
		stackPos(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4));
	if (_specialExitCount < 5) {
		_specialExitTable[_specialExitCount+0] = stackPos(0);
		_specialExitTable[_specialExitCount+5] = stackPos(1);
		_specialExitTable[_specialExitCount+10] = stackPos(2) + stackPos(0) - 1;
		_specialExitTable[_specialExitCount+15] = stackPos(3) + stackPos(1) - 1;
		_specialExitTable[_specialExitCount+20] = stackPos(4);
		++_specialExitCount;
	}
	return 0;
}

int KyraEngine_v2::o2_setMousePos(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_setMousePos(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	setMousePos(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_v2::o2_showMouse(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_showMouse(%p) ()", (const void *)script);
	screen()->showMouse();
	return 0;
}

int KyraEngine_v2::o2_defineRoomEntrance(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_defineRoomEntrance(%p) (%d, %d, %d)", (const void *)script, stackPos(0), stackPos(1), stackPos(2));
	switch (stackPos(0)) {
	case 0:
		_sceneEnterX1 = stackPos(1);
		_sceneEnterY1 = stackPos(2);
		break;

	case 1:
		_sceneEnterX2 = stackPos(1);
		_sceneEnterY2 = stackPos(2);
		break;

	case 2:
		_sceneEnterX3 = stackPos(1);
		_sceneEnterY3 = stackPos(2);
		break;

	case 3:
		_sceneEnterX4 = stackPos(1);
		_sceneEnterY4 = stackPos(2);
		break;

	default:
		break;
	}
	return 0;
}

int KyraEngine_v2::o2_runAnimationScript(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_runAnimationScript(%p) ('%s', %d, %d, %d)", (const void *)script, stackPosString(0), stackPos(1),
			stackPos(2), stackPos(3));

	runAnimationScript(stackPosString(0), stackPos(3), stackPos(2) ? 1 : 0, stackPos(1), stackPos(2));
	return 0;
}

int KyraEngine_v2::o2_setSpecialSceneScriptRunTime(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_setSpecialSceneScriptRunTime(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	assert(stackPos(0) >= 0 && stackPos(0) < 10);
	_sceneSpecialScriptsTimer[stackPos(0)] = _system->getMillis() + stackPos(1) * _tickLength;
	return 0;
}

int KyraEngine_v2::o2_setSpecialSceneScriptState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_setSpecialSceneScriptState(%p) (%d)", (const void *)script, stackPos(0));
	_specialSceneScriptState[stackPos(0)] = 1;
	return 1;
}

int KyraEngine_v2::o2_clearSpecialSceneScriptState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_clearSpecialSceneScriptState(%p) (%d)", (const void *)script, stackPos(0));
	_specialSceneScriptState[stackPos(0)] = 0;
	return 0;
}

int KyraEngine_v2::o2_querySpecialSceneScriptState(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_querySpecialSceneScriptState(%p) (%d)", (const void *)script, stackPos(0));
	return _specialSceneScriptState[stackPos(0)];
}

int KyraEngine_v2::o2_disableTimer(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_disableTimer(%p) (%d)", (const void *)script, stackPos(0));
	_timer->disable(stackPos(0));
	return 0;
}

int KyraEngine_v2::o2_enableTimer(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_enableTimer(%p) (%d)", (const void *)script, stackPos(0));
	_timer->enable(stackPos(0));
	return 0;
}

int KyraEngine_v2::o2_setTimerCountdown(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_setTimerCountdown(%p) (%d, %d)", (const void *)script, stackPos(0), stackPos(1));
	_timer->setCountdown(stackPos(0), stackPos(1));
	return 0;
}

int KyraEngine_v2::o2_setVocHigh(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_setVocHigh(%p) (%d)", (const void *)script, stackPos(0));
	_vocHigh = stackPos(0);
	return _vocHigh;
}

int KyraEngine_v2::o2_getVocHigh(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2_getVocHigh(%p) ()", (const void *)script);
	return _vocHigh;
}

#pragma mark -

int KyraEngine_v2::o2a_setAnimationShapes(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_v2::o2a_setAnimationShapes(%p) ('%s', %d, %d, %d, %d, %d)", (const void *)script,
			stackPosString(0), stackPos(1), stackPos(2), stackPos(3), stackPos(4), stackPos(5));
	strcpy(_animShapeFilename, stackPosString(0));
	_animShapeLastEntry = stackPos(1);
	_animShapeWidth = stackPos(2);
	_animShapeHeight = stackPos(3);
	_animShapeXAdd = stackPos(4);
	_animShapeYAdd = stackPos(5);
	return 0;
}

int KyraEngine_v2::o2a_setResetFrame(EMCState *script) {
	debugC(3, kDebugLevelScriptFuncs, "KyraEngine_MR::o3t_setResetFrame(%p) (%d)", (const void *)script, stackPos(0));
	_animResetFrame = stackPos(0);
	return 0;
}

} // end of namespace Kyra

