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


#include "common/endian.h"
#include "common/md5.h"
#include "kyra/kyra.h"
#include "kyra/kyra_v1.h"
#include "kyra/kyra_v2.h"
#include "kyra/kyra_v3.h"
#include "kyra/screen.h"
#include "kyra/screen_v1.h"
#include "kyra/screen_v2.h"
#include "kyra/screen_v3.h"
#include "kyra/resource.h"
#include "kyra/gui_v1.h"
#include "kyra/gui_v2.h"
#include "kyra/gui_v3.h"

#include "gui/message.h"

namespace Kyra {

#define RESFILE_VERSION 24

bool StaticResource::checkKyraDat() {
	Common::File kyraDat;
	if (!kyraDat.open(StaticResource::staticDataFilename()))
		return false;

	uint32 size = kyraDat.size() - 16;
	uint8 digest[16];
	kyraDat.seek(size, SEEK_SET);
	if (kyraDat.read(digest, 16) != 16)
		return false;
	kyraDat.close();

	uint8 digestCalc[16];
	if (!Common::md5_file(StaticResource::staticDataFilename().c_str(), digestCalc, size))
		return false;

	for (int i = 0; i < 16; ++i)
		if (digest[i] != digestCalc[i])
			return false;
	return true;
}

// used for the KYRA.DAT file which still uses
// the old flag system, we just convert it, which
// is less work than to change KYRA.DAT again
enum {
	GF_FLOPPY	= 1 <<  0,
	GF_TALKIE	= 1 <<  1,
	GF_FMTOWNS	= 1 <<  2,
	GF_DEMO		= 1 <<  3,
	GF_ENGLISH	= 1 <<  4,
	GF_FRENCH	= 1 <<  5,
	GF_GERMAN	= 1 <<  6,
	GF_SPANISH	= 1 <<  7,
	GF_ITALIAN	= 1 <<  8,
	GF_JAPANESE = 1 <<  9,
	// other languages here
	GF_LNGUNK	= 1 << 16,	// also used for multi language in kyra3
	GF_AMIGA	= 1 << 17
};

#define GAME_FLAGS (GF_FLOPPY | GF_TALKIE | GF_DEMO | GF_FMTOWNS | GF_AMIGA)
#define LANGUAGE_FLAGS (GF_ENGLISH | GF_FRENCH | GF_GERMAN | GF_SPANISH | GF_ITALIAN | GF_JAPANESE | GF_LNGUNK)

uint32 createFeatures(const GameFlags &flags) {
	if (flags.isTalkie)
		return GF_TALKIE;
	if (flags.isDemo)
		return GF_DEMO;
	if (flags.platform == Common::kPlatformFMTowns || flags.platform == Common::kPlatformPC98)
		return GF_FMTOWNS;
	if (flags.platform == Common::kPlatformAmiga)
		return GF_AMIGA;
	return GF_FLOPPY;
}

uint32 createLanguage(const GameFlags &flags) {
	if (flags.lang == Common::EN_ANY)
		return GF_ENGLISH;
	if (flags.lang == Common::DE_DEU)
		return GF_GERMAN;
	if (flags.lang == Common::FR_FRA)
		return GF_FRENCH;
	if (flags.lang == Common::ES_ESP)
		return GF_SPANISH;
	if (flags.lang == Common::IT_ITA)
		return GF_ITALIAN;
	if (flags.lang == Common::JA_JPN)
		return GF_JAPANESE;
	return GF_LNGUNK;
}

struct LanguageTypes {
	uint32 flags;
	const char *ext;
};

static const LanguageTypes languages[] = {
	{ GF_ENGLISH, "ENG" },	// this is the default language
	{ GF_FRENCH, "FRE" },
	{ GF_GERMAN, "GER" },
	{ GF_SPANISH, "SPA" },
	{ GF_ITALIAN, "ITA" },
	{ GF_JAPANESE, "JPN" },
	{ 0, 0 }
};

bool StaticResource::init() {
#define proc(x) &StaticResource::x
	static const FileType fileTypeTable[] = {
		{ kLanguageList, proc(loadLanguageTable), proc(freeStringTable) },
		{ kStringList, proc(loadStringTable), proc(freeStringTable) },
		{ StaticResource::kRoomList, proc(loadRoomTable), proc(freeRoomTable) },
		{ kShapeList, proc(loadShapeTable), proc(freeShapeTable) },
		{ kRawData, proc(loadRawData), proc(freeRawData) },
		{ kPaletteTable, proc(loadPaletteTable), proc(freePaletteTable) },

		{ k2SeqData, proc(loadHofSequenceData), proc(freeHofSequenceData) },
		{ k2ShpAnimDataV1, proc(loadHofShapeAnimDataV1), proc(freeHofShapeAnimDataV1) },
		{ k2ShpAnimDataV2, proc(loadHofShapeAnimDataV2), proc(freeHofShapeAnimDataV2) },

		{ 0, 0, 0 }
	};
#undef proc
	_fileLoader = fileTypeTable;

	// Kyrandia 1 Filenames
	static const FilenameTable kyra1StaticRes[] = {
		// INTRO / OUTRO sequences
		{ kForestSeq, kRawData, "FOREST.SEQ" },
		{ kKallakWritingSeq, kRawData, "KALLAK-WRITING.SEQ" },
		{ kKyrandiaLogoSeq, kRawData, "KYRANDIA-LOGO.SEQ" },
		{ kKallakMalcolmSeq, kRawData, "KALLAK-MALCOLM.SEQ" },
		{ kMalcolmTreeSeq, kRawData, "MALCOLM-TREE.SEQ" },
		{ kWestwoodLogoSeq, kRawData, "WESTWOOD-LOGO.SEQ" },
		{ kDemo1Seq, kRawData, "DEMO1.SEQ" },
		{ kDemo2Seq, kRawData, "DEMO2.SEQ" },
		{ kDemo3Seq, kRawData, "DEMO3.SEQ" },
		{ kDemo4Seq, kRawData, "DEMO4.SEQ" },
		{ kOutroReunionSeq, kRawData, "REUNION.SEQ" },

		// INTRO / OUTRO strings
		{ kIntroCPSStrings, kStringList, "INTRO-CPS.TXT" },
		{ kIntroCOLStrings, kStringList, "INTRO-COL.TXT" },
		{ kIntroWSAStrings, kStringList, "INTRO-WSA.TXT" },
		{ kIntroStrings, kLanguageList, "INTRO-STRINGS." },
		{ kOutroHomeString, kLanguageList, "HOME." },

		// INGAME strings
		{ kItemNames, kLanguageList, "ITEMLIST." },
		{ kTakenStrings, kLanguageList, "TAKEN." },
		{ kPlacedStrings, kLanguageList, "PLACED." },
		{ kDroppedStrings, kLanguageList, "DROPPED." },
		{ kNoDropStrings, kLanguageList, "NODROP." },
		{ kPutDownString, kLanguageList, "PUTDOWN." },
		{ kWaitAmuletString, kLanguageList, "WAITAMUL." },
		{ kBlackJewelString, kLanguageList, "BLACKJEWEL." },
		{ kPoisonGoneString, kLanguageList, "POISONGONE." },
		{ kHealingTipString, kLanguageList, "HEALINGTIP." },
		{ kThePoisonStrings, kLanguageList, "THEPOISON." },
		{ kFluteStrings, kLanguageList, "FLUTE." },
		{ kWispJewelStrings, kLanguageList, "WISPJEWEL." },
		{ kMagicJewelStrings, kLanguageList, "MAGICJEWEL." },
		{ kFlaskFullString, kLanguageList, "FLASKFULL." },
		{ kFullFlaskString, kLanguageList, "FULLFLASK." },
		{ kVeryCleverString, kLanguageList, "VERYCLEVER." },
		{ kNewGameString, kLanguageList, "NEWGAME." },

		// GUI strings table
		{ kGUIStrings, kLanguageList, "GUISTRINGS." },
		{ kConfigStrings, kLanguageList, "CONFIGSTRINGS." },

		// ROOM table/filenames
		{ Kyra::kRoomList, StaticResource::kRoomList, "ROOM-TABLE.ROOM" },
		{ kRoomFilenames, kStringList, "ROOM-FILENAMES.TXT" },

		// SHAPE tables
		{ kDefaultShapes, kShapeList, "SHAPES-DEFAULT.SHP" },
		{ kHealing1Shapes, kShapeList, "HEALING.SHP" },
		{ kHealing2Shapes, kShapeList, "HEALING2.SHP" },
		{ kPoisonDeathShapes, kShapeList, "POISONDEATH.SHP" },
		{ kFluteShapes, kShapeList, "FLUTE.SHP" },
		{ kWinter1Shapes, kShapeList, "WINTER1.SHP" },
		{ kWinter2Shapes, kShapeList, "WINTER2.SHP" },
		{ kWinter3Shapes, kShapeList, "WINTER3.SHP" },
		{ kDrinkShapes, kShapeList, "DRINK.SHP" },
		{ kWispShapes, kShapeList, "WISP.SHP" },
		{ kMagicAnimShapes, kShapeList, "MAGICANIM.SHP" },
		{ kBranStoneShapes, kShapeList, "BRANSTONE.SHP" },

		// IMAGE filename table
		{ kCharacterImageFilenames, kStringList, "CHAR-IMAGE.TXT" },

		// AMULET anim
		{ kAmuleteAnimSeq, kRawData, "AMULETEANIM.SEQ" },

		// PALETTE table
		{ kPaletteList, kPaletteTable, "1 33 PALTABLE" },

		// AUDIO files
		{ kAudioTracks, kStringList, "TRACKS.TXT" },
		{ kAudioTracksIntro, kStringList, "TRACKSINT.TXT" },

		// FM-TOWNS specific
		{ kKyra1TownsSFXwdTable, kRawData, "SFXWDTABLE" },
		{ kKyra1TownsSFXbtTable, kRawData, "SFXBTTABLE" },
		{ kKyra1TownsCDATable, kRawData, "CDATABLE" },
		{ kCreditsStrings, kRawData, "CREDITS" },

		{ 0, 0, 0 }
	};

	static const FilenameTable kyra2StaticRes[] = {
		// Sequence Player
		{ k2SeqplayPakFiles, kStringList, "S_PAKFILES.TXT" },
		{ k2SeqplayCredits, kRawData, "S_CREDITS.TXT" },
		{ k2SeqplayCreditsSpecial, kStringList, "S_CREDITS2.TXT" },
		{ k2SeqplayStrings, kLanguageList, "S_STRINGS." },
		{ k2SeqplaySfxFiles, kStringList, "S_SFXFILES.TXT" },
		{ k2SeqplayTlkFiles, kLanguageList, "S_TLKFILES." },
		{ k2SeqplaySeqData, k2SeqData, "S_DATA.SEQ" },
		{ k2SeqplayIntroTracks, kStringList, "S_INTRO.TRA" },
		{ k2SeqplayFinaleTracks, kStringList, "S_FINALE.TRA" },
		{ k2SeqplayIntroCDA, kRawData, "S_INTRO.CDA" },
		{ k2SeqplayFinaleCDA, kRawData, "S_FINALE.CDA" },
		{ k2SeqplayShapeAnimData, k2ShpAnimDataV1, "S_DEMO.SHP" },

		// Ingame
		{ k2IngamePakFiles, kStringList, "I_PAKFILES.TXT" },
		{ k2IngameSfxFiles, kStringList, "I_SFXFILES.TRA" },
		{ k2IngameSfxIndex, kRawData, "I_SFXINDEX.MAP" },
		{ k2IngameTracks, kStringList, "I_TRACKS.TRA" },
		{ k2IngameCDA, kRawData, "I_TRACKS.CDA" },
		{ k2IngameTalkObjIndex, kRawData, "I_TALKOBJECTS.MAP" },
		{ k2IngameTimJpStrings, kStringList, "I_TIMJPSTR.TXT" },
		{ k2IngameShapeAnimData, k2ShpAnimDataV2, "I_INVANIM.SHP" },
		{ k2IngameTlkDemoStrings, kLanguageList, "I_TLKDEMO.TXT." },		

		{ 0, 0, 0 }
	};

	if (_vm->game() == GI_KYRA1) {
		_builtIn = 0;
		_filenameTable = kyra1StaticRes;
	} else if (_vm->game() == GI_KYRA2) {
		_builtIn = 0;
		_filenameTable = kyra2StaticRes;
	} else if (_vm->game() == GI_KYRA3) {
		return true;
	} else {
		error("unknown game ID");
	}

	int tempSize = 0;
	uint8 *temp = getFile("INDEX", tempSize);
	if (!temp) {
		warning("No matching INDEX file found ('%s')", getFilename("INDEX"));
		outputError();
		return false;
	}

	if (tempSize != 3*4) {
		delete [] temp;
		warning("'%s' has illegal filesize %d", getFilename("INDEX"), tempSize);
		outputError();
		return false;
	}

	uint32 version = READ_BE_UINT32(temp);
	uint32 gameID = READ_BE_UINT32((temp+4));
	uint32 featuresValue = READ_BE_UINT32((temp+8));

	delete [] temp;
	temp = 0;

	if (version != RESFILE_VERSION) {
		warning("Invalid KYRA.DAT file version (%u, required %d)", version, RESFILE_VERSION);
		outputError();
		return false;
	}

	if (gameID != _vm->game()) {
		warning("Invalid game id (%u)", gameID);
		outputError();
		return false;
	}

	uint32 gameFeatures = createFeatures(_vm->gameFlags());
	if ((featuresValue & GAME_FLAGS) != gameFeatures) {
		warning("Your data file has a different game flags (0x%.08X has the data and your version has 0x%.08X)", (featuresValue & GAME_FLAGS), gameFeatures);
		outputError();
		return false;
	}

	// load all tables for now
	if (!prefetchId(-1)) {
		warning("Couldn't load all needed resources from 'KYRA.DAT'");
		outputError();
		return false;
	}
	return true;
}

void StaticResource::deinit() {
	unloadId(-1);
}

void StaticResource::outputError() {
	Common::String errorMessage = "Your '" + StaticResource::staticDataFilename() + "' file is outdated, reget it from the ScummVM website";
	::GUI::MessageDialog errorMsg(errorMessage);
	errorMsg.runModal();
	error(errorMessage.c_str());
}

const char * const*StaticResource::loadStrings(int id, int &strings) {
	const char * const*temp = (const char* const*)getData(id, kStringList, strings);
	if (temp)
		return temp;
	return (const char* const*)getData(id, kLanguageList, strings);
}

const uint8 *StaticResource::loadRawData(int id, int &size) {
	return (const uint8*)getData(id, kRawData, size);
}

const Shape *StaticResource::loadShapeTable(int id, int &entries) {
	return (const Shape*)getData(id, kShapeList, entries);
}

const Room *StaticResource::loadRoomTable(int id, int &entries) {
	return (const Room*)getData(id, StaticResource::kRoomList, entries);
}

const uint8 * const*StaticResource::loadPaletteTable(int id, int &entries) {
	return (const uint8* const*)getData(id, kPaletteTable, entries);
}

const HofSeqData *StaticResource::loadHofSequenceData(int id, int &entries) {
	return (const HofSeqData*)getData(id, k2SeqData, entries);
}

const ItemAnimData_v1 *StaticResource::loadHofShapeAnimDataV1(int id, int &entries) {
	return (const ItemAnimData_v1*)getData(id, k2ShpAnimDataV1, entries);
}

const ItemAnimData_v2 *StaticResource::loadHofShapeAnimDataV2(int id, int &entries) {
	return (const ItemAnimData_v2*)getData(id, k2ShpAnimDataV2, entries);
}

bool StaticResource::prefetchId(int id) {
	if (id == -1) {
		for (int i = 0; _filenameTable[i].filename; ++i)
			prefetchId(_filenameTable[i].id);
		return true;
	}
	const void *ptr = 0;
	int type = -1, size = -1;

	if (checkResList(id, type, ptr, size))
		return true;

	if (checkForBuiltin(id, type, size))
		return true;

	const FilenameTable *filename = searchFile(id);
	if (!filename)
		return false;
	const FileType *filetype = getFiletype(filename->type);
	if (!filetype)
		return false;

	ResData data;
	data.id = id;
	data.type = filetype->type;
	if (!(this->*(filetype->load))(filename->filename, data.data, data.size))
		return false;
	_resList.push_back(data);

	return true;
}

void StaticResource::unloadId(int id) {
	Common::List<ResData>::iterator pos = _resList.begin();
	for (; pos != _resList.end(); ++pos) {
		if (pos->id == id || id == -1) {
			const FileType *filetype = getFiletype(pos->type);
			(this->*(filetype->free))(pos->data, pos->size);
			if (id != -1)
				break;
		}
	}
}

bool StaticResource::checkResList(int id, int &type, const void *&ptr, int &size) {
	Common::List<ResData>::iterator pos = _resList.begin();
	for (; pos != _resList.end(); ++pos) {
		if (pos->id == id) {
			size = pos->size;
			type = pos->type;
			ptr = pos->data;
			return true;
		}
	}
	return false;
}

const void *StaticResource::checkForBuiltin(int id, int &type, int &size) {
	if (!_builtIn)
		return 0;

	for (int i = 0; _builtIn[i].data; ++i) {
		if (_builtIn[i].id == id) {
			size = _builtIn[i].size;
			type = _builtIn[i].type;
			return _builtIn[i].data;
		}
	}

	return 0;
}

const StaticResource::FilenameTable *StaticResource::searchFile(int id) {
	if (!_filenameTable)
		return 0;

	for (int i = 0; _filenameTable[i].filename; ++i) {
		if (_filenameTable[i].id == id)
			return &_filenameTable[i];
	}

	return 0;
}

const StaticResource::FileType *StaticResource::getFiletype(int type) {
	if (!_fileLoader)
		return 0;

	for (int i = 0; _fileLoader[i].load; ++i) {
		if (_fileLoader[i].type == type)
			return &_fileLoader[i];
	}

	return 0;
}

const void *StaticResource::getData(int id, int requesttype, int &size) {
	const void *ptr = 0;
	int type = -1;
	size = 0;

	if (checkResList(id, type, ptr, size)) {
		if (type == requesttype)
			return ptr;
		return 0;
	}

	ptr = checkForBuiltin(id, type, size);
	if (ptr) {
		if (type == requesttype)
			return ptr;
		return 0;
	}

	if (!prefetchId(id))
		return 0;

	if (checkResList(id, type, ptr, size)) {
		if (type == requesttype)
			return ptr;
	}

	return 0;
}

bool StaticResource::loadLanguageTable(const char *filename, void *&ptr, int &size) {
	static Common::String file;
	for (int i = 0; languages[i].ext; ++i) {
		if (languages[i].flags != createLanguage(_vm->gameFlags()))
			continue;

		file = filename;
		file += languages[i].ext;
		if (loadStringTable(file.c_str(), ptr, size))
			return true;
	}

	file = filename;
	file += languages[0].ext;
	if (loadStringTable(file.c_str(), ptr, size)) {
		static bool warned = false;
		if (!warned) {
			warned = true;
			warning("couldn't find specific language table for your version, using English now");
		}
		return true;
	}

	return false;
}

bool StaticResource::loadStringTable(const char *filename, void *&ptr, int &size) {
	uint8 *filePtr = getFile(filename, size);
	if (!filePtr)
		return false;
	uint8 *src = filePtr;

	uint32 count = READ_BE_UINT32(src); src += 4;
	size = count;
	char **output = new char*[count];
	assert(output);

	const char *curPos = (const char*)src;
	for (uint32 i = 0; i < count; ++i) {
		int strLen = strlen(curPos);
		output[i] = new char[strLen+1];
		assert(output[i]);
		memcpy(output[i], curPos, strLen+1);
		curPos += strLen+1;
	}

	delete [] filePtr;
	ptr = output;

	return true;
}

bool StaticResource::loadRawData(const char *filename, void *&ptr, int &size) {
	ptr = getFile(filename, size);
	if (!ptr)
		return false;
	return true;
}

bool StaticResource::loadShapeTable(const char *filename, void *&ptr, int &size) {
	uint8 *filePtr = getFile(filename, size);
	if (!filePtr)
		return false;
	uint8 *src = filePtr;

	uint32 count = READ_BE_UINT32(src); src += 4;
	size = count;
	Shape *loadTo = new Shape[count];
	assert(loadTo);

	for (uint32 i = 0; i < count; ++i) {
		loadTo[i].imageIndex = *src++;
		loadTo[i].x = *src++;
		loadTo[i].y = *src++;
		loadTo[i].w = *src++;
		loadTo[i].h = *src++;
		loadTo[i].xOffset = *src++;
		loadTo[i].yOffset = *src++;
	}

	delete [] filePtr;
	ptr = loadTo;

	return true;
}

bool StaticResource::loadRoomTable(const char *filename, void *&ptr, int &size) {
	uint8 *filePtr = getFile(filename, size);
	if (!filePtr)
		return false;
	uint8 *src = filePtr;

	uint32 count = READ_BE_UINT32(src); src += 4;
	size = count;
	Room *loadTo = new Room[count];
	assert(loadTo);

	for (uint32 i = 0; i < count; ++i) {
		loadTo[i].nameIndex = *src++;
		loadTo[i].northExit = READ_BE_UINT16(src); src += 2;
		loadTo[i].eastExit = READ_BE_UINT16(src); src += 2;
		loadTo[i].southExit = READ_BE_UINT16(src); src += 2;
		loadTo[i].westExit = READ_BE_UINT16(src); src += 2;
		memset(&loadTo[i].itemsTable[0], 0xFF, sizeof(byte)*6);
		memset(&loadTo[i].itemsTable[6], 0, sizeof(byte)*6);
		memset(loadTo[i].itemsXPos, 0, sizeof(uint16)*12);
		memset(loadTo[i].itemsYPos, 0, sizeof(uint8)*12);
		memset(loadTo[i].needInit, 0, sizeof(loadTo[i].needInit));
	}

	delete [] filePtr;
	ptr = loadTo;

	return true;
}

bool StaticResource::loadPaletteTable(const char *filename, void *&ptr, int &size) {
	const char *temp = filename;
	int start = atoi(temp);
	temp = strstr(temp, " ");
	if (temp == NULL)
		return false;
	++temp;
	int end = atoi(temp);

	char **table = new char*[end-start+1];
	assert(table);

	char file[64];
	temp = filename;
	temp = strstr(temp, " ");
	++temp;
	temp = strstr(temp, " ");
	if (temp == NULL)
		return false;
	++temp;
	strncpy(file, temp, 64);

	char name[64];
	for (int i = start; i <= end; ++i) {
		snprintf(name, 64, "%s%d.PAL", file, i);
		table[(start != 0) ? (i-start) : i] = (char*)getFile(name, size);
		if (!table[(start != 0) ? (i-start) : i]) {
			delete [] table;
			return false;
		}
	}

	ptr = table;
	size = end - start + 1;
	return true;
}

bool StaticResource::loadHofSequenceData(const char *filename, void *&ptr, int &size) {
	int filesize;
	uint8 *filePtr = getFile(filename, filesize);

	if (!filePtr)
		return false;

	uint16 *hdr = (uint16 *) filePtr;
	int numSeq = READ_BE_UINT16(hdr++);
	Sequence *tmp_s = new Sequence[numSeq];
	char *tmp_c = 0;

	size = sizeof(HofSeqData) + numSeq * (sizeof(Sequence) + 28);

	for (int i = 0; i < numSeq; i++) {
		const uint8 *offset = (const uint8 *)(filePtr + READ_BE_UINT16(hdr++));
		tmp_s[i].flags = READ_BE_UINT16(offset);
		offset += 2;
		tmp_c = new char[14];
		memcpy(tmp_c, offset, 14);
		tmp_s[i].wsaFile = tmp_c;
		offset += 14;
		tmp_c = new char[14];
		memcpy(tmp_c, offset, 14);
		tmp_s[i].cpsFile = tmp_c;
		offset += 14;
		tmp_s[i].startupCommand = *offset++;
		tmp_s[i].finalCommand = *offset++;
		tmp_s[i].stringIndex1 = READ_BE_UINT16(offset);
		offset += 2;
		tmp_s[i].stringIndex2 = READ_BE_UINT16(offset);
		offset += 2;
		tmp_s[i].startFrame = READ_BE_UINT16(offset);
		offset += 2;
		tmp_s[i].numFrames = READ_BE_UINT16(offset);
		offset += 2;
		tmp_s[i].frameDelay = READ_BE_UINT16(offset);
		offset += 2;
		tmp_s[i].xPos = READ_BE_UINT16(offset);
		offset += 2;
		tmp_s[i].yPos = READ_BE_UINT16(offset);
		offset += 2;
		tmp_s[i].duration = READ_BE_UINT16(offset);
	}

	int numSeqN = READ_BE_UINT16(hdr++);
	NestedSequence *tmp_n = new NestedSequence[numSeqN];
	size += (numSeqN * (sizeof(NestedSequence) + 14));

	for (int i = 0; i < numSeqN; i++) {
		const uint8 *offset = (const uint8 *)(filePtr + READ_BE_UINT16(hdr++));
		tmp_n[i].flags = READ_BE_UINT16(offset);
		offset += 2;
		tmp_c = new char[14];
		memcpy(tmp_c, offset, 14);
		tmp_n[i].wsaFile = tmp_c;
		offset += 14;
		tmp_n[i].startframe = READ_BE_UINT16(offset);
		offset += 2;
		tmp_n[i].endFrame = READ_BE_UINT16(offset);
		offset += 2;
		tmp_n[i].frameDelay = READ_BE_UINT16(offset);
		offset += 2;
		tmp_n[i].x = READ_BE_UINT16(offset);
		offset += 2;
		tmp_n[i].y = READ_BE_UINT16(offset);
		offset += 2;
		uint16 ctrlOffs = READ_BE_UINT16(offset);
		offset += 2;
		tmp_n[i].startupCommand = READ_BE_UINT16(offset);
		offset += 2;
		tmp_n[i].finalCommand = READ_BE_UINT16(offset);

		if (ctrlOffs) {
			int num_c = *(filePtr + ctrlOffs);
			const uint16 *in_c = (uint16*) (filePtr + ctrlOffs + 1);
			FrameControl *tmp_f = new FrameControl[num_c];

			for (int ii = 0; ii < num_c; ii++) {
				tmp_f[ii].index = READ_BE_UINT16(in_c++);
				tmp_f[ii].delay = READ_BE_UINT16(in_c++);
			}
			
			tmp_n[i].wsaControl = (const FrameControl*) tmp_f;
			size += (num_c * sizeof(FrameControl));

		} else {
			tmp_n[i].wsaControl = 0;
		}
	}

	delete [] filePtr;

	HofSeqData *loadTo = new HofSeqData;
	assert(loadTo);

	loadTo->seq = tmp_s;
	loadTo->seqn = tmp_n;
	loadTo->numSeq = numSeq;
	loadTo->numSeqn = numSeqN;

	ptr = loadTo;

	return true;
}

bool StaticResource::loadHofShapeAnimDataV1(const char *filename, void *&ptr, int &size) {
	int filesize;
	uint8 *filePtr = getFile(filename, filesize);
	uint8 *src = filePtr;

	if (!filePtr)
		return false;

	size = *src++;
	ItemAnimData_v1 *loadTo = new ItemAnimData_v1[size];
	assert(loadTo);

	for (int i = 0; i < size; i++) {
		loadTo[i].itemIndex = (int16) READ_BE_UINT16(src);
		src += 2;
		loadTo[i].y = READ_BE_UINT16(src);
		src += 2;
		uint16 *tmp_f = new uint16[20];
		for (int ii = 0; ii < 20; ii++) {
			tmp_f[ii] = READ_BE_UINT16(src);
			src += 2;
		}
		loadTo[i].frames = tmp_f;
	}

	delete [] filePtr;
	ptr = loadTo;

	return true;
}

bool StaticResource::loadHofShapeAnimDataV2(const char *filename, void *&ptr, int &size) {
	int filesize;
	uint8 *filePtr = getFile(filename, filesize);
	uint8 *src = filePtr;

	if (!filePtr)
		return false;

	size = *src++;
	ItemAnimData_v2 *loadTo = new ItemAnimData_v2[size];
	assert(loadTo);

	for (int i = 0; i < size; i++) {
		loadTo[i].itemIndex = (int16) READ_BE_UINT16(src);
		src += 2;
		loadTo[i].numFrames = *src++;
		FrameControl *tmp_f = new FrameControl[loadTo[i].numFrames];
		for (int ii = 0; ii < loadTo[i].numFrames; ii++) {
			tmp_f[ii].index = READ_BE_UINT16(src);
			src += 2;
			tmp_f[ii].delay = READ_BE_UINT16(src);
			src += 2;
		}
		loadTo[i].frames = tmp_f;
	}

	delete [] filePtr;
	ptr = loadTo;

	return true;
}

void StaticResource::freeRawData(void *&ptr, int &size) {
	uint8 *data = (uint8*)ptr;
	delete [] data;
	ptr = 0;
	size = 0;
}

void StaticResource::freeStringTable(void *&ptr, int &size) {
	char **data = (char**)ptr;
	while (size--)
		delete [] data[size];
	ptr = 0;
	size = 0;
}

void StaticResource::freeShapeTable(void *&ptr, int &size) {
	Shape *data = (Shape*)ptr;
	delete [] data;
	ptr = 0;
	size = 0;
}

void StaticResource::freeRoomTable(void *&ptr, int &size) {
	Room *data = (Room*)ptr;
	delete [] data;
	ptr = 0;
	size = 0;
}

void StaticResource::freeHofSequenceData(void *&ptr, int &size) {
	HofSeqData *h = (HofSeqData*) ptr;

	for (int i = 0; i < h->numSeq; i++) {
		delete [] h->seq[i].wsaFile;
		delete [] h->seq[i].cpsFile;
	}
	delete [] h->seq;

	for (int i = 0; i < h->numSeqn; i++) {
		delete [] h->seqn[i].wsaFile;
		delete [] h->seqn[i].wsaControl;
	}
	delete [] h->seqn;

	delete h;
	ptr = 0;
	size = 0;
}

void StaticResource::freeHofShapeAnimDataV1(void *&ptr, int &size) {
	ItemAnimData_v1 *d= (ItemAnimData_v1*) ptr;
	for (int i = 0; i < size; i++)
		delete [] d[i].frames;
	delete [] d;
	ptr = 0;
	size = 0;
}

void StaticResource::freeHofShapeAnimDataV2(void *&ptr, int &size) {
	ItemAnimData_v2 *d= (ItemAnimData_v2*) ptr;
	for (int i = 0; i < size; i++)
		delete [] d[i].frames;
	delete [] d;
	ptr = 0;
	size = 0;
}

void StaticResource::freePaletteTable(void *&ptr, int &size) {
	uint8 **data = (uint8**)ptr;
	while (size--)
		delete [] data[size];
	ptr = 0;
	size = 0;
}

const char *StaticResource::getFilename(const char *name) {
	static Common::String filename;

	filename = name;

	if (_vm->gameFlags().gameID == GI_KYRA2)
		filename += ".K2";

	if (_vm->gameFlags().isTalkie)
		filename += ".CD";
	else if (_vm->gameFlags().isDemo)
		filename += ".DEM";
	else if (_vm->gameFlags().platform == Common::kPlatformFMTowns || _vm->gameFlags().platform == Common::kPlatformPC98)
		filename += ".TNS";
	else if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		filename += ".AMG";

	return filename.c_str();
}

uint8 *StaticResource::getFile(const char *name, int &size) {
	uint32 tempSize = 0;
	uint8 *data = _vm->resource()->fileData(getFilename(name), &tempSize);
	size = tempSize;
	return data;
}

#pragma mark -

void KyraEngine_v1::initStaticResource() {
	int temp = 0;
	_seq_Forest = _staticres->loadRawData(kForestSeq, temp);
	_seq_KallakWriting = _staticres->loadRawData(kKallakWritingSeq, temp);
	_seq_KyrandiaLogo = _staticres->loadRawData(kKyrandiaLogoSeq, temp);
	_seq_KallakMalcolm = _staticres->loadRawData(kKallakMalcolmSeq, temp);
	_seq_MalcolmTree = _staticres->loadRawData(kMalcolmTreeSeq, temp);
	_seq_WestwoodLogo = _staticres->loadRawData(kWestwoodLogoSeq, temp);
	_seq_Demo1 = _staticres->loadRawData(kDemo1Seq, temp);
	_seq_Demo2 = _staticres->loadRawData(kDemo2Seq, temp);
	_seq_Demo3 = _staticres->loadRawData(kDemo3Seq, temp);
	_seq_Demo4 = _staticres->loadRawData(kDemo4Seq, temp);
	_seq_Reunion = _staticres->loadRawData(kOutroReunionSeq, temp);

	_seq_WSATable = _staticres->loadStrings(kIntroWSAStrings, _seq_WSATable_Size);
	_seq_CPSTable = _staticres->loadStrings(kIntroCPSStrings, _seq_CPSTable_Size);
	_seq_COLTable = _staticres->loadStrings(kIntroCOLStrings, _seq_COLTable_Size);
	_seq_textsTable = _staticres->loadStrings(kIntroStrings, _seq_textsTable_Size);

	_itemList = _staticres->loadStrings(kItemNames, _itemList_Size);
	_takenList = _staticres->loadStrings(kTakenStrings, _takenList_Size);
	_placedList = _staticres->loadStrings(kPlacedStrings, _placedList_Size);
	_droppedList = _staticres->loadStrings(kDroppedStrings, _droppedList_Size);
	_noDropList = _staticres->loadStrings(kNoDropStrings, _noDropList_Size);
	_putDownFirst = _staticres->loadStrings(kPutDownString, _putDownFirst_Size);
	_waitForAmulet = _staticres->loadStrings(kWaitAmuletString, _waitForAmulet_Size);
	_blackJewel = _staticres->loadStrings(kBlackJewelString, _blackJewel_Size);
	_poisonGone = _staticres->loadStrings(kPoisonGoneString, _poisonGone_Size);
	_healingTip = _staticres->loadStrings(kHealingTipString, _healingTip_Size);
	_thePoison = _staticres->loadStrings(kThePoisonStrings, _thePoison_Size);
	_fluteString = _staticres->loadStrings(kFluteStrings, _fluteString_Size);
	_wispJewelStrings = _staticres->loadStrings(kWispJewelStrings, _wispJewelStrings_Size);
	_magicJewelString = _staticres->loadStrings(kMagicJewelStrings, _magicJewelString_Size);
	_flaskFull = _staticres->loadStrings(kFlaskFullString, _flaskFull_Size);
	_fullFlask = _staticres->loadStrings(kFullFlaskString, _fullFlask_Size);
	_veryClever = _staticres->loadStrings(kVeryCleverString, _veryClever_Size);
	_homeString = _staticres->loadStrings(kOutroHomeString, _homeString_Size);
	_newGameString = _staticres->loadStrings(kNewGameString, _newGameString_Size);

	_healingShapeTable = _staticres->loadShapeTable(kHealing1Shapes, _healingShapeTableSize);
	_healingShape2Table = _staticres->loadShapeTable(kHealing2Shapes, _healingShape2TableSize);
	_posionDeathShapeTable = _staticres->loadShapeTable(kPoisonDeathShapes, _posionDeathShapeTableSize);
	_fluteAnimShapeTable = _staticres->loadShapeTable(kFluteShapes, _fluteAnimShapeTableSize);
	_winterScrollTable = _staticres->loadShapeTable(kWinter1Shapes, _winterScrollTableSize);
	_winterScroll1Table = _staticres->loadShapeTable(kWinter2Shapes, _winterScroll1TableSize);
	_winterScroll2Table = _staticres->loadShapeTable(kWinter3Shapes, _winterScroll2TableSize);
	_drinkAnimationTable = _staticres->loadShapeTable(kDrinkShapes, _drinkAnimationTableSize);
	_brandonToWispTable = _staticres->loadShapeTable(kWispShapes, _brandonToWispTableSize);
	_magicAnimationTable = _staticres->loadShapeTable(kMagicAnimShapes, _magicAnimationTableSize);
	_brandonStoneTable = _staticres->loadShapeTable(kBranStoneShapes, _brandonStoneTableSize);

	_characterImageTable = _staticres->loadStrings(kCharacterImageFilenames, _characterImageTableSize);

	_roomFilenameTable = _staticres->loadStrings(kRoomFilenames, _roomFilenameTableSize);

	_amuleteAnim = _staticres->loadRawData(kAmuleteAnimSeq, temp);

	_specialPalettes = _staticres->loadPaletteTable(kPaletteList, temp);

	_guiStrings = _staticres->loadStrings(kGUIStrings, _guiStringsSize);
	_configStrings = _staticres->loadStrings(kConfigStrings, _configStringsSize);

	_soundFiles = _staticres->loadStrings(kAudioTracks, _soundFilesSize);
	_soundFilesIntro = _staticres->loadStrings(kAudioTracksIntro, _soundFilesIntroSize);
	_cdaTrackTable = (const int32*) _staticres->loadRawData(kKyra1TownsCDATable, _cdaTrackTableSize);

	// copied static res

	// room list
	const Room *tempRoomList = _staticres->loadRoomTable(kRoomList, _roomTableSize);

	if (_roomTableSize > 0) {
		_roomTable = new Room[_roomTableSize];
		assert(_roomTable);

		memcpy(_roomTable, tempRoomList, _roomTableSize*sizeof(Room));
		tempRoomList = 0;

		_staticres->unloadId(kRoomList);
	}

	// default shape table
	const Shape *tempShapeTable = _staticres->loadShapeTable(kDefaultShapes, _defaultShapeTableSize);

	if (_defaultShapeTableSize > 0) {
		_defaultShapeTable = new Shape[_defaultShapeTableSize];
		assert(_defaultShapeTable);

		memcpy(_defaultShapeTable, tempShapeTable, _defaultShapeTableSize*sizeof(Shape));
		tempShapeTable = 0;

		_staticres->unloadId(kDefaultShapes);
	}

	// audio data tables
	static const AudioDataStruct soundData_PC[] = {
		{ _soundFilesIntro, _soundFilesIntroSize, 0, 0 },
		{ _soundFiles, _soundFilesSize, 0, 0 },
		{ 0, 0, 0, 0}
	};

	static const AudioDataStruct soundData_TOWNS[] = {
		{ _soundFiles, _soundFilesSize, _cdaTrackTable, _cdaTrackTableSize },
		{ _soundFiles, _soundFilesSize, _cdaTrackTable, _cdaTrackTableSize },
		{ 0, 0, 0, 0}
	};
	_soundData = (_flags.platform == Common::kPlatformPC) ? soundData_PC : soundData_TOWNS;
}

void KyraEngine_v1::loadMouseShapes() {
	_screen->loadBitmap("MOUSE.CPS", 3, 3, 0);
	_screen->_curPage = 2;
	_shapes[0] = _screen->encodeShape(0, 0, 8, 10, 0);
	_shapes[1] = _screen->encodeShape(0, 0x17, 0x20, 7, 0);
	_shapes[2] = _screen->encodeShape(0x50, 0x12, 0x10, 9, 0);
	_shapes[3] = _screen->encodeShape(0x60, 0x12, 0x10, 11, 0);
	_shapes[4] = _screen->encodeShape(0x70, 0x12, 0x10, 9, 0);
	_shapes[5] = _screen->encodeShape(0x80, 0x12, 0x10, 11, 0);
	_shapes[6] = _screen->encodeShape(0x90, 0x12, 0x10, 10, 0);
	_shapes[360] = _screen->encodeShape(0x28, 0, 0x10, 13, 0);
	_screen->setMouseCursor(1, 1, 0);
	_screen->setMouseCursor(1, 1, _shapes[0]);
	_screen->setShapePages(5, 3);
}

void KyraEngine_v1::loadCharacterShapes() {
	int curImage = 0xFF;
	int videoPage = _screen->_curPage;
	_screen->_curPage = 2;
	for (int i = 0; i < 115; ++i) {
		assert(i < _defaultShapeTableSize);
		Shape *shape = &_defaultShapeTable[i];
		if (shape->imageIndex == 0xFF) {
			_shapes[i+7] = 0;
			continue;
		}
		if (shape->imageIndex != curImage) {
			assert(shape->imageIndex < _characterImageTableSize);
			_screen->loadBitmap(_characterImageTable[shape->imageIndex], 3, 3, 0);
			curImage = shape->imageIndex;
		}
		_shapes[i+7] = _screen->encodeShape(shape->x<<3, shape->y, shape->w<<3, shape->h, 1);
	}
	_screen->_curPage = videoPage;
}

void KyraEngine_v1::loadSpecialEffectShapes() {
	_screen->loadBitmap("EFFECTS.CPS", 3, 3, 0);
	_screen->_curPage = 2;

	int currShape;
	for (currShape = 173; currShape < 183; currShape++)
		_shapes[currShape] = _screen->encodeShape((currShape-173) * 24, 0, 24, 24, 1);

	for (currShape = 183; currShape < 190; currShape++)
		_shapes[currShape] = _screen->encodeShape((currShape-183) * 24, 24, 24, 24, 1);

	for (currShape = 190; currShape < 201; currShape++)
		_shapes[currShape] = _screen->encodeShape((currShape-190) * 24, 48, 24, 24, 1);

	for (currShape = 201; currShape < 206; currShape++)
		_shapes[currShape] = _screen->encodeShape((currShape-201) * 16, 106, 16, 16, 1);
}

void KyraEngine_v1::loadItems() {
	int shape;

	_screen->loadBitmap("JEWELS3.CPS", 3, 3, 0);
	_screen->_curPage = 2;

	_shapes[323] = 0;

	for (shape = 1; shape < 6; shape++ )
		_shapes[323 + shape] = _screen->encodeShape((shape - 1) * 32, 0, 32, 17, 0);

	for (shape = 330; shape <= 334; shape++)
		_shapes[shape] = _screen->encodeShape((shape-330) * 32, 102, 32, 17, 0);

	for (shape = 335; shape <= 339; shape++)
		_shapes[shape] = _screen->encodeShape((shape-335) * 32, 17,  32, 17, 0);

	for (shape = 340; shape <= 344; shape++)
		_shapes[shape] = _screen->encodeShape((shape-340) * 32, 34,  32, 17, 0);

	for (shape = 345; shape <= 349; shape++)
		_shapes[shape] = _screen->encodeShape((shape-345) * 32, 51,  32, 17, 0);

	for (shape = 350; shape <= 354; shape++)
		_shapes[shape] = _screen->encodeShape((shape-350) * 32, 68,  32, 17, 0);

	for (shape = 355; shape <= 359; shape++)
		_shapes[shape] = _screen->encodeShape((shape-355) * 32, 85,  32, 17, 0);


	_screen->loadBitmap("ITEMS.CPS", 3, 3, 0);
	_screen->_curPage = 2;

	for (int i = 0; i < 107; i++) {
		shape = findDuplicateItemShape(i);

		if (shape != -1)
			_shapes[216 + i] = _shapes[216 + shape];
		else
			_shapes[216 + i] = _screen->encodeShape( (i % 20) * 16, i/20 * 16, 16, 16, 0);
	}

	uint32 size;
	uint8 *fileData = _res->fileData("_ITEM_HT.DAT", &size);
	assert(fileData);

	for (int i = 0; i < 107; i++) {
		_itemTable[i].height = fileData[i];
		_itemTable[i].unk1 = _itemTable[i].unk2 = 0;
	}

	delete[] fileData;
}

void KyraEngine_v1::loadButtonShapes() {
	_screen->loadBitmap("BUTTONS2.CPS", 3, 3, 0);
	_screen->_curPage = 2;
	_gui->_scrollUpButton.data0ShapePtr = _screen->encodeShape(0, 0, 24, 14, 1);
	_gui->_scrollUpButton.data1ShapePtr = _screen->encodeShape(24, 0, 24, 14, 1);
	_gui->_scrollUpButton.data2ShapePtr = _screen->encodeShape(48, 0, 24, 14, 1);
	_gui->_scrollDownButton.data0ShapePtr = _screen->encodeShape(0, 15, 24, 14, 1);
	_gui->_scrollDownButton.data1ShapePtr = _screen->encodeShape(24, 15, 24, 14, 1);
	_gui->_scrollDownButton.data2ShapePtr = _screen->encodeShape(48, 15, 24, 14, 1);
	_screen->_curPage = 0;
}

void KyraEngine_v1::loadMainScreen(int page) {
	_screen->clearPage(page);

	if (_flags.lang == Common::EN_ANY && !_flags.isTalkie && (_flags.platform == Common::kPlatformPC || _flags.platform == Common::kPlatformAmiga))
		_screen->loadBitmap("MAIN15.CPS", page, page, _screen->getPalette(0));
	else if (_flags.lang == Common::EN_ANY || _flags.lang == Common::JA_JPN || (_flags.isTalkie && _flags.lang == Common::IT_ITA))
		_screen->loadBitmap("MAIN_ENG.CPS", page, page, 0);
	else if (_flags.lang == Common::FR_FRA)
		_screen->loadBitmap("MAIN_FRE.CPS", page, page, 0);
	else if (_flags.lang == Common::DE_DEU)
		_screen->loadBitmap("MAIN_GER.CPS", page, page, 0);
	else if (_flags.lang == Common::ES_ESP)
		_screen->loadBitmap("MAIN_SPA.CPS", page, page, 0);
	else if (_flags.lang == Common::IT_ITA)
		_screen->loadBitmap("MAIN_ITA.CPS", page, page, 0);
	else
		warning("no main graphics file found");

	if (_flags.platform == Common::kPlatformAmiga)
		memcpy(_screen->getPalette(1), _screen->getPalette(0), 32*3);

	_screen->copyRegion(0, 0, 0, 0, 320, 200, page, 0);
}

void KyraEngine_v2::initStaticResource() {
	int tmpSize = 0;

	_sequencePakList = _staticres->loadStrings(k2SeqplayPakFiles, _sequencePakListSize);
	_ingamePakList = _staticres->loadStrings(k2IngamePakFiles, _ingamePakListSize);
	_sequenceStrings = _staticres->loadStrings(k2SeqplayStrings, _sequenceStringsSize);	
	_ingameSoundList = _staticres->loadStrings(k2IngameSfxFiles, _ingameSoundListSize);
	_ingameSoundIndex = (const uint16*) _staticres->loadRawData(k2IngameSfxIndex, _ingameSoundIndexSize);
	_musicFileListIntro = _staticres->loadStrings(k2SeqplayIntroTracks, _musicFileListIntroSize);
	_musicFileListIngame = _staticres->loadStrings(k2IngameTracks, _musicFileListIngameSize);
	_musicFileListFinale = _staticres->loadStrings(k2SeqplayFinaleTracks, _musicFileListFinaleSize);
	_cdaTrackTableIntro = _staticres->loadRawData(k2SeqplayIntroCDA, _cdaTrackTableIntroSize);
	_cdaTrackTableIngame = _staticres->loadRawData(k2IngameCDA, _cdaTrackTableIngameSize);
	_cdaTrackTableFinale = _staticres->loadRawData(k2SeqplayFinaleCDA, _cdaTrackTableFinaleSize);
	_ingameTalkObjIndex = (const uint16*) _staticres->loadRawData(k2IngameTalkObjIndex, _ingameTalkObjIndexSize);
	_ingameTimJpStr = _staticres->loadStrings(k2IngameTimJpStrings, _ingameTimJpStrSize);
	_itemAnimData = _staticres->loadHofShapeAnimDataV2(k2IngameShapeAnimData, _itemAnimDataSize);

	// replace sequence talkie files with localized versions and cut off .voc
	// suffix from voc files so as to allow compression specific file extensions
	const char* const* seqSoundList = _staticres->loadStrings(k2SeqplaySfxFiles, _sequenceSoundListSize);
	const char* const* tlkfiles = _staticres->loadStrings(k2SeqplayTlkFiles, tmpSize);
	char ** tmpSndLst = new char*[_sequenceSoundListSize];

	for (int i = 0; i < _sequenceSoundListSize; i++) {
		int len = strlen(seqSoundList[i]);

		tmpSndLst[i] = new char[len + 1];
		tmpSndLst[i][0] = 0;

		if (_flags.platform == Common::kPlatformPC)
			len -= 4;

		if (tlkfiles) {
			for (int ii = 0; ii < tmpSize; ii++) {
				if (!scumm_stricmp(&seqSoundList[i][1], &tlkfiles[ii][1]))
					strcpy(tmpSndLst[i], tlkfiles[ii]);
			}
		}

		if (tmpSndLst[i][0] == 0)
			strcpy(tmpSndLst[i], seqSoundList[i]);

		tmpSndLst[i][len] = 0;
	}

	tlkfiles = seqSoundList = 0;
	_staticres->unloadId(k2SeqplayTlkFiles);
	_staticres->unloadId(k2SeqplaySfxFiles);
	_sequenceSoundList = tmpSndLst;

	// assign music data
	static const char *fmtMusicFileListIntro[] = { "intro%d.twn" };
	static const char *fmtMusicFileListFinale[] = { "finale%d.twn" };
	static const char *fmtMusicFileListIngame[] = { "km%02d.twn" };

	static const AudioDataStruct soundData_PC[] = {
		{ _musicFileListIntro, _musicFileListIntroSize, 0, 0 },
		{ _musicFileListIngame, _musicFileListIngameSize, 0, 0},
		{ _musicFileListFinale, _musicFileListIntroSize, 0, 0 }
	};

	static const AudioDataStruct soundData_TOWNS[] = {
		{ fmtMusicFileListIntro, 1, _cdaTrackTableIntro, _cdaTrackTableIntroSize >> 1 },
		{ fmtMusicFileListIngame, 1, _cdaTrackTableIngame, _cdaTrackTableIngameSize >> 1 },
		{ fmtMusicFileListFinale, 1, _cdaTrackTableFinale, _cdaTrackTableFinaleSize >> 1 }
	};
	_soundData = (_flags.platform == Common::kPlatformPC) ? soundData_PC : soundData_TOWNS;

	// setup sequence data
	_sequences = _staticres->loadHofSequenceData(k2SeqplaySeqData, tmpSize);

	static const SeqProc hofSequenceCallbacks[] = { 0,
		&KyraEngine_v2::seq_introWestwood,
		&KyraEngine_v2::seq_introTitle, &KyraEngine_v2::seq_introOverview,
		&KyraEngine_v2::seq_introLibrary, &KyraEngine_v2::seq_introHand,
		&KyraEngine_v2::seq_introPoint, &KyraEngine_v2::seq_introZanfaun,
		&KyraEngine_v2::seq_finaleFunters, &KyraEngine_v2::seq_finaleFerb,
		&KyraEngine_v2::seq_finaleFish, &KyraEngine_v2::seq_finaleFheep,
		&KyraEngine_v2::seq_finaleFarmer, &KyraEngine_v2::seq_finaleFuards,
		&KyraEngine_v2::seq_finaleFirates, &KyraEngine_v2::seq_finaleFrash
	};

	static const SeqProc hofNestedSequenceCallbacks[] = {
		&KyraEngine_v2::seq_finaleFiggle, &KyraEngine_v2::seq_introOver1,
		&KyraEngine_v2::seq_introOver2, &KyraEngine_v2::seq_introForest,
		&KyraEngine_v2::seq_introDragon, &KyraEngine_v2::seq_introDarm,
		&KyraEngine_v2::seq_introLibrary2, &KyraEngine_v2::seq_introLibrary2,
		&KyraEngine_v2::seq_introMarco, &KyraEngine_v2::seq_introHand1a,
		&KyraEngine_v2::seq_introHand1b, &KyraEngine_v2::seq_introHand1c,
		&KyraEngine_v2::seq_introHand2,	&KyraEngine_v2::seq_introHand3, 0
	};

	static const SeqProc hofDemoSequenceCallbacks[] = {
		&KyraEngine_v2::seq_demoVirgin, &KyraEngine_v2::seq_demoWestwood,
		&KyraEngine_v2::seq_demoTitle, &KyraEngine_v2::seq_demoHill,
		&KyraEngine_v2::seq_demoOuthome, &KyraEngine_v2::seq_demoWharf,
		&KyraEngine_v2::seq_demoDinob, &KyraEngine_v2::seq_demoFisher, 0
	};

	static const SeqProc hofDemoNestedSequenceCallbacks[] = {
		&KyraEngine_v2::seq_demoWharf2, &KyraEngine_v2::seq_demoDinob2,
		&KyraEngine_v2::seq_demoWater, &KyraEngine_v2::seq_demoBail,
		&KyraEngine_v2::seq_demoDig, 0
	};

	_callbackS = (_flags.isDemo && !_flags.isTalkie) ? hofDemoSequenceCallbacks : hofSequenceCallbacks;
	_callbackN = (_flags.isDemo && !_flags.isTalkie) ? hofDemoNestedSequenceCallbacks : hofNestedSequenceCallbacks;
}

const ScreenDim Screen_v1::_screenDimTable[] = {
	{ 0x00, 0x00, 0x28, 0xC8, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x08, 0x48, 0x18, 0x38, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x01, 0x08, 0x26, 0x80, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x00, 0xC2, 0x28, 0x06, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x00, 0x90, 0x28, 0x38, 0x04, 0x0C, 0x00, 0x00 },
	{ 0x01, 0x94, 0x26, 0x30, 0x04, 0x1B, 0x00, 0x00 },
	{ 0x00, 0x90, 0x28, 0x38, 0x0F, 0x0D, 0x00, 0x00 },
	{ 0x01, 0x96, 0x26, 0x32, 0x0F, 0x0D, 0x00, 0x00 },
	{ 0x00, 0x00, 0x28, 0x88, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x01, 0x20, 0x26, 0x80, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x03, 0x28, 0x22, 0x46, 0x0F, 0x0D, 0x00, 0x00 }
};

const int Screen_v1::_screenDimTableCount = ARRAYSIZE(Screen_v1::_screenDimTable);

const ScreenDim Screen_v2::_screenDimTable[] = {
	{ 0x00, 0x00, 0x28, 0xC8, 0xC7, 0xCF, 0x00, 0x00 },
	{ 0x08, 0x48, 0x18, 0x38, 0xC7, 0xCF, 0x00, 0x00 },
	{ 0x00, 0x00, 0x28, 0x90, 0xC7, 0xCF, 0x00, 0x00 },
	{ 0x00, 0xC2, 0x28, 0x06, 0xC7, 0xCF, 0x00, 0x00 },
	{ 0x00, 0x90, 0x28, 0x38, 0x96, 0xCF, 0x00, 0x00 },
	{ 0x01, 0x94, 0x26, 0x30, 0x96, 0x1B, 0x00, 0x00 },
	{ 0x00, 0x90, 0x28, 0x38, 0xC7, 0xCC, 0x00, 0x00 },
	{ 0x01, 0x96, 0x26, 0x32, 0xC7, 0xCC, 0x00, 0x00 },
	{ 0x00, 0x00, 0x28, 0x88, 0xC7, 0xCF, 0x00, 0x00 },
	{ 0x00, 0x08, 0x28, 0xB8, 0xC7, 0xCF, 0x00, 0x00 },
	{ 0x01, 0x28, 0x26, 0x46, 0xC7, 0xCC, 0x00, 0x00 },
	{ 0x0A, 0x96, 0x14, 0x30, 0x19, 0xF0, 0x00, 0x00 }	// menu, just present for current menu code
};

const int Screen_v2::_screenDimTableCount = ARRAYSIZE(Screen_v2::_screenDimTable);

const ScreenDim Screen_v3::_screenDimTable[] = {
	{ 0x00, 0x00, 0x28, 0xC8, 0xFF, 0xF0, 0x00, 0x00 },
	{ 0x08, 0x48, 0x18, 0x38, 0xFF, 0xF0, 0x00, 0x00 },
	{ 0x00, 0x00, 0x28, 0xBC, 0xFF, 0xF0, 0x00, 0x00 },
	{ 0x0A, 0x96, 0x14, 0x30, 0x19, 0xF0, 0x00, 0x00 }
};

const int Screen_v3::_screenDimTableCount = ARRAYSIZE(Screen_v3::_screenDimTable);

const int8 KyraEngine::_addXPosTable[] = {
	 4,  4,  0, -4, -4, -4,  0,  4
};

const int8 KyraEngine::_addYPosTable[] = {
	 0, -2, -2, -2,  0,  2,  2,  2
};

const int8 KyraEngine_v1::_charXPosTable[] = {
	 0,  4,  4,  4,  0, -4, -4, -4
};

const int8 KyraEngine_v1::_charYPosTable[] = {
	-2, -2,  0,  2,  2,  2,  0, -2
};

const uint16 KyraEngine_v1::_itemPosX[] = {
	95, 115, 135, 155, 175, 95, 115, 135, 155, 175
};

const uint8 KyraEngine_v1::_itemPosY[] = {
	160, 160, 160, 160, 160, 181, 181, 181, 181, 181
};

void GUI_v1::initStaticResource() {
	GUI_V1_BUTTON(_scrollUpButton, 0x12, 1, 1, 1, 0x483, 0, 0, 0, 0x18, 0x0f, 0);
	GUI_V1_BUTTON(_scrollDownButton, 0x13, 1, 1, 1, 0x483, 0, 0, 0, 0x18, 0x0f, 0);

	GUI_V1_BUTTON(_menuButtonData[0], 0x0c, 1, 1, 1, 0x487, 0, 0, 0, 0, 0, 0);
	GUI_V1_BUTTON(_menuButtonData[1], 0x0d, 1, 1, 1, 0x487, 0, 0, 0, 0, 0, 0);
	GUI_V1_BUTTON(_menuButtonData[2], 0x0e, 1, 1, 1, 0x487, 0, 0, 0, 0, 0, 0);
	GUI_V1_BUTTON(_menuButtonData[3], 0x0f, 1, 1, 1, 0x487, 0, 0, 0, 0, 0, 0);
	GUI_V1_BUTTON(_menuButtonData[4], 0x10, 1, 1, 1, 0x487, 0, 0, 0, 0, 0, 0);
	GUI_V1_BUTTON(_menuButtonData[5], 0x11, 1, 1, 1, 0x487, 0, 0, 0, 0, 0, 0);

	delete [] _menu;
	_menu = new Menu[6];
	assert(_menu);

	Button::Callback quitPlayingFunctor = BUTTON_FUNCTOR(GUI_v1, this, &GUI_v1::quitPlaying);
	Button::Callback loadGameMenuFunctor = BUTTON_FUNCTOR(GUI_v1, this, &GUI_v1::loadGameMenu);
	Button::Callback cancelSubMenuFunctor = BUTTON_FUNCTOR(GUI_v1, this, &GUI_v1::cancelSubMenu);

	GUI_V1_MENU(_menu[0], -1, -1, 0x100, 0x8B, 248, 249, 250, 0, 251, -1, 8, 0, 5, -1, -1, -1, -1);
	GUI_V1_MENU_ITEM(_menu[0].item[0], 1, 0, 0, 0, -1, -1, 0x1E, 0xDC, 0x0F, 252, 253, -1, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[0].item[1], 1, 0, 0, 0, -1, -1, 0x2F, 0xDC, 0x0F, 252, 253, -1, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[0].item[2], 1, 0, 0, 0, -1, -1, 0x40, 0xDC, 0x0F, 252, 253, -1, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[0].item[3], 1, 0, 0, 0, -1, -1, 0x51, 0xDC, 0x0F, 252, 253, -1, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[0].item[4], 1, 0, 0, 0, -1,  0, 0x6E, 0xDC, 0x0F, 252, 253, -1, 255, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	_menu[0].item[0].callback = loadGameMenuFunctor;
	_menu[0].item[1].callback = BUTTON_FUNCTOR(GUI_v1, this, &GUI_v1::saveGameMenu);
	_menu[0].item[2].callback = BUTTON_FUNCTOR(GUI_v1, this, &GUI_v1::gameControlsMenu);
	_menu[0].item[3].callback = quitPlayingFunctor;
	_menu[0].item[4].callback = BUTTON_FUNCTOR(GUI_v1, this, &GUI_v1::resumeGame);
	
	GUI_V1_MENU(_menu[1], -1, -1, 0x140, 0x38, 248, 249, 250, 0, 254,-1, 8, 0, 2, -1, -1, -1, -1);
	GUI_V1_MENU_ITEM(_menu[1].item[0], 1, 0, 0, 0, 0x18, 0, 0x1E, 0x48, 0x0F, 252, 253, -1, 255, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[1].item[1], 1, 0, 0, 0, 0xD8, 0, 0x1E, 0x48, 0x0F, 252, 253, -1, 255, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	_menu[1].item[0].callback = BUTTON_FUNCTOR(GUI_v1, this, &GUI_v1::quitConfirmYes);
	_menu[1].item[1].callback = BUTTON_FUNCTOR(GUI_v1, this, &GUI_v1::quitConfirmNo);
	
	GUI_V1_MENU(_menu[2], -1, -1, 0x120, 0xA0, 248, 249, 250, 0, 251, -1, 8, 0, 6, 132, 22, 132, 124);
	GUI_V1_MENU_ITEM(_menu[2].item[0], 1, 0, 0, 0, -1, 255, 0x27, 0x100, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[2].item[1], 1, 0, 0, 0, -1, 255, 0x38, 0x100, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[2].item[2], 1, 0, 0, 0, -1, 255, 0x49, 0x100, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[2].item[3], 1, 0, 0, 0, -1, 255, 0x5A, 0x100, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[2].item[4], 1, 0, 0, 0, -1, 255, 0x6B, 0x100, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[2].item[5], 1, 0, 0, 0, 0xB8, 0, 0x86, 0x58, 0x0F, 252, 253, -1, 255, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	_menu[2].item[5].callback = cancelSubMenuFunctor;
	
	GUI_V1_MENU(_menu[3], -1, -1, 288, 67, 248, 249, 250, 0, 251, -1, 8, 0, 2, -1, -1, -1, -1);
	GUI_V1_MENU_ITEM(_menu[3].item[0], 1, 0, 0, 0, 24, 0, 44, 85, 15, 252, 253, -1, 255, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[3].item[1], 1, 0, 0, 0, 179, 0, 44, 85, 15, 252, 253, -1, 255, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	_menu[3].item[0].callback = BUTTON_FUNCTOR(GUI_v1, this, &GUI_v1::savegameConfirm);
	_menu[3].item[1].callback = cancelSubMenuFunctor;
	
	GUI_V1_MENU(_menu[4], -1, -1, 0xD0, 0x4C, 248, 249, 250, 0, 251, -1, 8, 0, 2, -1, -1, -1, -1);
	GUI_V1_MENU_ITEM(_menu[4].item[0], 1, 0, 0, 0, -1, -1, 0x1E, 0xB4, 0x0F, 252, 253, -1, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[4].item[1], 1, 0, 0, 0, -1, -1, 0x2F, 0xB4, 0x0F, 252, 253, -1, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	_menu[4].item[0].callback = loadGameMenuFunctor;
	_menu[4].item[1].callback = quitPlayingFunctor;

	GUI_V1_MENU(_menu[5], -1, -1, 0x130, 0x99, 248, 249, 250, 0, 251, -1, 8, 0, 6, -1, -1, -1, -1);
	GUI_V1_MENU_ITEM(_menu[5].item[0], 1, 0, 0, 0, 0xA5, 0, 0x1E, 0x80, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0x10, 0x20, 0, 0);
	GUI_V1_MENU_ITEM(_menu[5].item[1], 1, 0, 0, 0, 0xA5, 0, 0x2F, 0x80, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0x10, 0x31, 0, 0);
	GUI_V1_MENU_ITEM(_menu[5].item[2], 1, 0, 0, 0, 0xA5, 0, 0x40, 0x80, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0x10, 0x42, 0, 0);
	GUI_V1_MENU_ITEM(_menu[5].item[3], 1, 0, 0, 0, 0xA5, 0, 0x51, 0x80, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0x10, 0x53, 0, 0);
	GUI_V1_MENU_ITEM(_menu[5].item[4], 1, 0, 0, 0, 0xA5, 0, 0x62, 0x80, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0x10, 0x65, 0, 0);
	GUI_V1_MENU_ITEM(_menu[5].item[5], 1, 0, 0, 0,   -1, 0, 0x7F, 0x6C, 0x0F, 252, 253, -1, 255, 248, 249, 250, -1, -0, 0, 0, 0, 0);
	_menu[5].item[0].callback = BUTTON_FUNCTOR(GUI_v1, this, &GUI_v1::controlsChangeMusic);
	_menu[5].item[1].callback = BUTTON_FUNCTOR(GUI_v1, this, &GUI_v1::controlsChangeSounds);
	_menu[5].item[2].callback = BUTTON_FUNCTOR(GUI_v1, this, &GUI_v1::controlsChangeWalk);
	_menu[5].item[4].callback = BUTTON_FUNCTOR(GUI_v1, this, &GUI_v1::controlsChangeText);
	_menu[5].item[5].callback = BUTTON_FUNCTOR(GUI_v1, this, &GUI_v1::controlsApply);
}

void KyraEngine_v1::setupButtonData() {
	delete [] _buttonData;
	delete [] _buttonDataListPtr;

	_buttonData = new Button[15];
	assert(_buttonData);
	_buttonDataListPtr = new Button*[15];
	assert(_buttonDataListPtr);

	GUI_V1_BUTTON(_buttonData[1], 0x01, 1, 1, 1, 0x0487, 0, 0x009, 0xA4, 0x36, 0x1E, 0);
	_buttonData[1].buttonCallback = BUTTON_FUNCTOR(GUI_v1, _gui, &GUI_v1::buttonMenuCallback);

	Button::Callback inventoryFunctor = BUTTON_FUNCTOR(KyraEngine_v1, this, &KyraEngine_v1::buttonInventoryCallback);
	for (int i = 2; i <= 10; ++i)
		_buttonData[i].buttonCallback = inventoryFunctor;
	_buttonData[0].buttonCallback = inventoryFunctor;
	GUI_V1_BUTTON(_buttonData[0], 0x02, 0, 0, 0, 0x0400, 0, 0x05D, 0x9E, 0x13, 0x13, 0);
	GUI_V1_BUTTON(_buttonData[2], 0x03, 0, 0, 0, 0x0400, 0, 0x071, 0x9E, 0x13, 0x14, 0);
	GUI_V1_BUTTON(_buttonData[3], 0x04, 0, 0, 0, 0x0400, 0, 0x085, 0x9E, 0x13, 0x14, 0);
	GUI_V1_BUTTON(_buttonData[4], 0x05, 0, 0, 0, 0x0400, 0, 0x099, 0x9E, 0x13, 0x14, 0);
	GUI_V1_BUTTON(_buttonData[5], 0x06, 0, 0, 0, 0x0400, 0, 0x0AD, 0x9E, 0x13, 0x14, 0);
	GUI_V1_BUTTON(_buttonData[6], 0x07, 0, 0, 0, 0x0400, 0, 0x05D, 0xB3, 0x13, 0x14, 0);
	GUI_V1_BUTTON(_buttonData[7], 0x08, 0, 0, 0, 0x0400, 0, 0x071, 0xB3, 0x13, 0x14, 0);
	GUI_V1_BUTTON(_buttonData[8], 0x09, 0, 0, 0, 0x0400, 0, 0x085, 0xB3, 0x13, 0x14, 0);
	GUI_V1_BUTTON(_buttonData[9], 0x0A, 0, 0, 0, 0x0400, 0, 0x099, 0xB3, 0x13, 0x14, 0);
	GUI_V1_BUTTON(_buttonData[10], 0x0B, 0, 0, 0, 0x0400, 0, 0x0AD, 0xB3, 0x13, 0x14, 0);

	Button::Callback amuletFunctor = BUTTON_FUNCTOR(KyraEngine_v1, this, &KyraEngine_v1::buttonAmuletCallback);
	GUI_V1_BUTTON(_buttonData[11], 0x15, 1, 1, 1, 0x0487, 0, 0x0FD, 0x9C, 0x1A, 0x12, 0);
	GUI_V1_BUTTON(_buttonData[12], 0x16, 1, 1, 1, 0x0487, 0, 0x0E7, 0xAA, 0x1A, 0x12, 0);
	GUI_V1_BUTTON(_buttonData[13], 0x17, 1, 1, 1, 0x0487, 0, 0x0FD, 0xB5, 0x1A, 0x12, 0);
	GUI_V1_BUTTON(_buttonData[14], 0x18, 1, 1, 1, 0x0487, 0, 0x113, 0xAA, 0x1A, 0x12, 0);
	for (int i = 11; i <= 14; ++i)
		_buttonData[i].buttonCallback = amuletFunctor;

	for (int i = 1; i < 15; ++i)
		_buttonDataListPtr[i-1] = &_buttonData[i];
	_buttonDataListPtr[14] = 0;
}

const uint8 KyraEngine_v1::_magicMouseItemStartFrame[] = {
	0xAD, 0xB7, 0xBE, 0x00
};

const uint8 KyraEngine_v1::_magicMouseItemEndFrame[] = {
	0xB1, 0xB9, 0xC2, 0x00
};

const uint8 KyraEngine_v1::_magicMouseItemStartFrame2[] = {
	0xB2, 0xBA, 0xC3, 0x00
};

const uint8 KyraEngine_v1::_magicMouseItemEndFrame2[] = {
	0xB6, 0xBD, 0xC8, 0x00
};

const uint16 KyraEngine_v1::_amuletX[] = { 231, 275, 253, 253 };
const uint16 KyraEngine_v1::_amuletY[] = { 170, 170, 159, 181 };

const uint16 KyraEngine_v1::_amuletX2[] = { 0x000, 0x0FD, 0x0E7, 0x0FD, 0x113, 0x000 };
const uint16 KyraEngine_v1::_amuletY2[] = { 0x000, 0x09F, 0x0AA, 0x0B5, 0x0AA, 0x000 };

const int8 KyraEngine_v1::_dosTrackMap[] = {
	-1,   0,  -1,   1,   0,   3,   0,   2,
	 0,   4,   1,   2,   1,   3,   1,   4,
	 1,  92,   1,   6,   1,   7,   2,   2,
	 2,   3,   2,   4,   2,   5,   2,   6,
	 2,   7,   3,   3,   3,   4,   1,   8,
	 1,   9,   4,   2,   4,   3,   4,   4,
	 4,   5,   4,   6,   4,   7,   4,   8,
	 1,  11,   1,  12,   1,  14,   1,  13,
	 4,   9,   5,  12,   6,   2,   6,   6,
	 6,   7,   6,   8,   6,   9,   6,   3,
	 6,   4,   6,   5,   7,   2,   7,   3,
	 7,   4,   7,   5,   7,   6,   7,   7,
	 7,   8,   7,   9,   8,   2,   8,   3,
	 8,   4,   8,   5,   6,  11,   5,  11
};

const int KyraEngine_v1::_dosTrackMapSize = ARRAYSIZE(KyraEngine_v1::_dosTrackMap);

// Kyra 2 and 3 main menu

const char *KyraEngine_v3::_mainMenuStrings[] = {
	"Start a new game",
	"Introduction",
	"Load a game",
	"Exit the game",
	"Nouvelle Partie",
	"Introduction",
	"Charger une partie",
	"Quitter le jeu",
	"Neues Spiel starten",
	"Intro",
	"Spielstand laden",
	"Spiel beenden",
	0
};

// kyra 2 static res

const uint8 KyraEngine_v2::_seqTextColorPresets[] = { 0x01, 0x01, 0x00, 0x3f, 0x3f, 0x3f };

const char *KyraEngine_v2::_languageExtension[] = {
	"ENG",
	"FRE",
	"GER",/*,
	"ITA",		Italian and Spanish was never included
	"SPA"*/
	"JPN"
};

const char *KyraEngine_v2::_scriptLangExt[] = {
	"EMC",
	"FMC",
	"GMC",/*,
	"IMC",		Italian and Spanish was never included
	"SMC"*/
	"JMC"
};

const int KyraEngine_v2::_characterFrameTable[] = {
	0x19, 0x09, 0x09, 0x12, 0x12, 0x12, 0x09, 0x09
};

const int KyraEngine_v2::_inventoryX[] = {
	0x4F, 0x63, 0x77, 0x8B, 0x9F, 0x4F, 0x63, 0x77, 0x8B, 0x9F
};

const int KyraEngine_v2::_inventoryY[] = {
	0x95, 0x95, 0x95, 0x95, 0x95, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA
};

const byte KyraEngine_v2::_itemStringMap[] = {
	2,    2,    0,    0,    2,    2,    2,    0,
	2,    2,    0,    0,    0,    2,    0,    0,
	0,    0,    0,    0,    2,    0,    0,    0,
	0,    1,    0,    2,    2,    2,    2,    0,
	3,    0,    3,    2,    2,    2,    3,    2,
	2,    2,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    2,    0,    0,    0,
	0,    0,    0,    0,    0,    2,    0,    0,
	2,    0,    0,    0,    0,    0,    0,    2,
	2,    0,    0,    0,    2,    2,    2,    2,
	2,    2,    2,    2,    2,    2,    2,    2,
	2,    2,    2,    2,    2,    2,    2,    0,
	2,    2,    2,    0,    0,    1,    3,    2,
	2,    2,    2,    2,    2,    0,    0,    0,
	0,    2,    2,    1,    0,    1,    2,    0,
	0,    0,    0,    0,    0,    2,    2,    2,
	2,    2,    2,    2,    0,    2,    2,    2,
	2,    3,    2,    0,    0,    0,    0,    1,
	2,    0,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    0,    0,    0,    0,
	2,    2,    0,    0,    0,    0,    0,    2,
	0,    2,    0,    0,    0,    0,    0,    0
};

const int KyraEngine_v2::_itemStringMapSize = ARRAYSIZE(KyraEngine_v2::_itemStringMap);

const int8 KyraEngine_v2::_dosTrackMap[] = {
	-1,    0,   -1,    1,    9,    6,    5,    4,
	 8,    3,   -2,    0,   -2,    0,    2,    3,
	-2,    0,   -2,    0,   -2,    0,   -2,    0,
	 0,    2,    0,    3,    1,    2,    1,    3,
	 2,    2,    2,    0,    3,    2,    3,    3,
	 3,    4,    4,    2,    5,    2,    5,    3,
	 5,    4,    6,    2,    6,    3,    6,    4,
	 6,    5,    6,    6,    6,    7,    6,    8,
	 6,    0,    6,    9,    7,    2,    7,    3,
	 7,    4,    7,    5,    8,    6,    7,    6,
	 7,    7,    7,    8,    7,    9,    8,    2,
	14,    2,    8,    4,    8,    7,    8,    8,
	 8,    9,    9,    2,    9,    3,    9,    4,
	 9,    5,    9,    7,    9,    8,    9,    9,
	10,    2,   10,    3,   10,    4,   10,    5,
	10,    6,   10,    7,   11,    2,   11,    3,
	11,    4,   11,    5,   11,    6,   11,    7,
	11,    8,   11,    9,   12,    2,   12,    3,
	12,    4,   12,    5,   12,    6,   12,    7,
	12,    8,   12,    9,   13,    2,    4,    7,
	14,    3,   14,    4,   14,    5,    4,    2,
	 4,    3,    4,    4,    4,    5,    4,    6
};

const int KyraEngine_v2::_dosTrackMapSize = ARRAYSIZE(KyraEngine_v2::_dosTrackMap);

void KyraEngine_v2::initInventoryButtonList() {
	delete [] _inventoryButtons;

	_inventoryButtons = new Button[15];
	assert(_inventoryButtons);

	GUI_V2_BUTTON(_inventoryButtons[0], 0x1, 0x4F, 0, 1, 1, 1, 0x4487, 0, 0x00A, 0x95, 0x39, 0x1D, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	_inventoryButtons[0].buttonCallback = BUTTON_FUNCTOR(GUI_v2, _gui, &GUI_v2::optionsButton);

	GUI_V2_BUTTON(_inventoryButtons[1], 0x2, 0x00, 0, 1, 1, 1, 0x4487, 0, 0x104, 0x90, 0x3C, 0x2C, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	_inventoryButtons[1].buttonCallback = BUTTON_FUNCTOR(KyraEngine_v2, this, &KyraEngine_v2::cauldronButton);

	GUI_V2_BUTTON(_inventoryButtons[2],	0x5, 0x00, 0, 1, 1, 1, 0x4487, 0, 0x0FA, 0x90, 0x0A, 0x2C, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	_inventoryButtons[2].buttonCallback = BUTTON_FUNCTOR(KyraEngine_v2, this, &KyraEngine_v2::cauldronClearButton);

	GUI_V2_BUTTON(_inventoryButtons[3], 0x3, 0x00, 0, 1, 1, 1, 0x4487, 0, 0x0CE, 0x90, 0x2C, 0x2C, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	_inventoryButtons[3].buttonCallback = BUTTON_FUNCTOR(KyraEngine_v2, this, &KyraEngine_v2::bookButton);

	GUI_V2_BUTTON(_inventoryButtons[4], 0x4, 0x00, 0, 1, 1, 1, 0x4487, 0, 0x0B6, 0x9D, 0x18, 0x1E, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	_inventoryButtons[4].buttonCallback = BUTTON_FUNCTOR(KyraEngine_v2, this, &KyraEngine_v2::scrollInventory);

	Button::Callback inventoryCallback = BUTTON_FUNCTOR(KyraEngine_v2, this, &KyraEngine_v2::buttonInventory);
	GUI_V2_BUTTON(_inventoryButtons[5], 0x6, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x04D, 0x92, 0x13, 0x15, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[6], 0x7, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x061, 0x92, 0x13, 0x15, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[7], 0x8, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x075, 0x92, 0x13, 0x15, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[8], 0x9, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x089, 0x92, 0x13, 0x15, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[9], 0xA, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x09D, 0x92, 0x13, 0x15, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[10], 0xB, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x04D, 0xA8, 0x13, 0x14, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[11], 0xC, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x061, 0xA8, 0x13, 0x14, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[12], 0xD, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x075, 0xA8, 0x13, 0x14, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[13], 0xE, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x089, 0xA8, 0x13, 0x14, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[14], 0xF, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x09D, 0xA8, 0x13, 0x14, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);

	for (int i = 5; i <= 14; ++i)
		_inventoryButtons[i].buttonCallback = inventoryCallback;

	_buttonList = &_inventoryButtons[0];
	for (size_t i = 1; i < 15; ++i)
		_buttonList = _gui->addButtonToList(_buttonList, &_inventoryButtons[i]);
}

void GUI_v2::initStaticData() {
	GUI_V2_BUTTON(_scrollUpButton, 0x17, 0, 0, 1, 1, 1, 0x4487, 0, 0, 0, 0x18, 0x0F, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_scrollDownButton, 0x18, 0, 0, 1, 1, 1, 0x4487, 0, 0, 0, 0x18, 0x0F, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);

	for (int i = 0; i < 4; ++i) {
		GUI_V2_BUTTON(_sliderButtons[0][i], 0x18+i, 0, 0, 1, 1, 1, 0x4487, 0, 0, 0, 0x0A, 0x0E, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	}
	for (int i = 0; i < 4; ++i) {
		GUI_V2_BUTTON(_sliderButtons[1][i], 0x1C+i, 0, 0, 1, 1, 1, 0x4487, 0, 0, 0, 0x0A, 0x0E, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	}
	for (int i = 0; i < 4; ++i) {
		GUI_V2_BUTTON(_sliderButtons[2][i], 0x20+i, 0, 0, 0, 0, 0, 0x2200, 0, 0, 0, 0x6E, 0x0E, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	}

	for (uint i = 0; i < ARRAYSIZE(_menuButtons); ++i) {
		GUI_V2_BUTTON(_menuButtons[i], 0x10+i, 0, 0, 1, 1, 1, 0x4487, 0, 0, 0, 0, 0, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	}

	Button::Callback clickLoadSlotFunctor = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::clickLoadSlot);
	Button::Callback clickSaveSlotFunctor = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::clickSaveSlot);
	Button::Callback clickLoadMenuFunctor = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::loadMenu);
	Button::Callback clickQuitGameFunctor = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::quitGame);
	Button::Callback clickQuitOptionsFunctor = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::quitOptionsMenu);

	const uint16 *menuStr = _vm->gameFlags().isTalkie ? _menuStringsTalkie : _menuStringsOther;

	GUI_V2_MENU(_mainMenu, -1, -1, 0x100, 0xAC, 0xF8, 0xF9, 0xFA, menuStr[0 * 8], 0xFB, -1, 8, 0, 7, -1, -1, -1, -1);
	GUI_V2_MENU_ITEM(_mainMenu.item[0], 1, 0x02, -1, 0x1E, 0xDC, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_mainMenu.item[0].callback = clickLoadMenuFunctor;
	GUI_V2_MENU_ITEM(_mainMenu.item[1], 1, 0x03, -1, 0x2F, 0xDC, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_mainMenu.item[1].callback = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::saveMenu);
	GUI_V2_MENU_ITEM(_mainMenu.item[2], 1, 0x23, -1, 0x40, 0xDC, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_mainMenu.item[2].callback = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::deleteMenu);
	GUI_V2_MENU_ITEM(_mainMenu.item[3], 1, 0x04, -1, 0x51, 0xDC, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_mainMenu.item[3].callback = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::gameOptionsTalkie);
	GUI_V2_MENU_ITEM(_mainMenu.item[4], 1, 0x25, -1, 0x62, 0xDC, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_mainMenu.item[4].callback = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::audioOptions);
	GUI_V2_MENU_ITEM(_mainMenu.item[5], 1, 0x05, -1, 0x73, 0xDC, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_mainMenu.item[5].callback = clickQuitGameFunctor;
	GUI_V2_MENU_ITEM(_mainMenu.item[6], 1, 0x06, -1, 0x90, 0xDC, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_mainMenu.item[6].callback = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::resumeGame);
	for (int i = 0; i < 7; ++i)
		_mainMenu.item[i].itemId = menuStr[0 * 8 + i + 1];

	if (!_vm->gameFlags().isTalkie) {
		_mainMenu.height = 0x9C;
		_mainMenu.numberOfItems = 6;
		_mainMenu.item[6].enabled = false;
		for (int i = 4; i < 6; ++i)
			_mainMenu.item[i].callback = _mainMenu.item[i+1].callback;
		_mainMenu.item[3].callback = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::gameOptions);
		_mainMenu.item[6].callback = Button::Callback();
		_mainMenu.item[5].y = 0x7F;
	}

	GUI_V2_MENU(_gameOptions, -1, -1, 0x120, 0x88, 0xF8, 0xF9, 0xFA, menuStr[1 * 8], 0xFB, -1, 8, 4, 4, -1, -1, -1, -1);
	if (_vm->gameFlags().isTalkie) {
		GUI_V2_MENU_ITEM(_gameOptions.item[0], 1, 0, 0xA0, 0x1E, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x15, 8, 0x20, 0);
		_gameOptions.item[0].callback = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::toggleWalkspeed);
		GUI_V2_MENU_ITEM(_gameOptions.item[1], 1, 0, 0xA0, 0x2F, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x26, 8, 0x31, 0);
		_gameOptions.item[1].callback = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::changeLanguage);
		GUI_V2_MENU_ITEM(_gameOptions.item[2], 1, 0, 0xA0, 0x40, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x16, 8, 0x42, 0);
		_gameOptions.item[2].callback = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::toggleText);
		GUI_V2_MENU_ITEM(_gameOptions.item[3], 1, 0x10, -1, 0x6E, 0x6C, 0x0F, 0xFD, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
		_gameOptions.item[3].callback = clickQuitOptionsFunctor;
	} else {
		_gameOptions.numberOfItems = 5;
		GUI_V2_MENU_ITEM(_gameOptions.item[0], 0, 0x2B, 0xA0, 0x1E, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x1F, 0x10, 0x20, 0);
		GUI_V2_MENU_ITEM(_gameOptions.item[1], 0, 0x2C, 0xA0, 0x2F, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x20, 0x10, 0x31, 0);
		GUI_V2_MENU_ITEM(_gameOptions.item[2], 0, 0x2D, 0xA0, 0x40, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x1D, 0x10, 0x42, 0);
		GUI_V2_MENU_ITEM(_gameOptions.item[3], 0, 0x2E, 0xA0, 0x51, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x1E, 0x10, 0x53, 0);
		GUI_V2_MENU_ITEM(_gameOptions.item[4], 1, 0x18, 0xA0, 0x6E, 0x6C, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
		_gameOptions.item[4].callback = clickQuitOptionsFunctor;
	}

	for (int i = _gameOptions.numberOfItems; i <= 6; ++i)
		_gameOptions.item[i].enabled = false;
	for (int i = 0; i < 7; ++i)
		_gameOptions.item[i].itemId = menuStr[1 * 8 + i + 1];

	GUI_V2_MENU(_audioOptions, -1, -1, 0x120, 0x88, 0xF8, 0xF9, 0xFA, menuStr[2 * 8], 0xFB, -1, 8, 3, 4, -1, -1, -1, -1);
	GUI_V2_MENU_ITEM(_audioOptions.item[0], 0, 0, 0xA0, 0x1E, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x17, 8, 0x20, 0);
	GUI_V2_MENU_ITEM(_audioOptions.item[1], 0, 0, 0xA0, 0x2F, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x18, 8, 0x31, 0);
	GUI_V2_MENU_ITEM(_audioOptions.item[2], 0, 0, 0xA0, 0x40, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x27, 8, 0x42, 0);
	GUI_V2_MENU_ITEM(_audioOptions.item[3], 1, 0x10, -1, 0x6E, 0x5C, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_audioOptions.item[3].callback = clickQuitOptionsFunctor;
	for (int i = 4; i <= 6; ++i)
		_audioOptions.item[i].enabled = false;
	for (int i = 0; i < 7; ++i)
		_audioOptions.item[i].itemId = menuStr[2 * 8 + i + 1];

	GUI_V2_MENU(_choiceMenu, -1, -1, 0x140, 0x38, 0xF8, 0xF9, 0xFA, 0, 0xFE, -1, 8, 0, 2, -1, -1, -1, -1);
	GUI_V2_MENU_ITEM(_choiceMenu.item[0], 1, 0x14, 0x18, 0x1E, 0x48, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_choiceMenu.item[0].callback = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::choiceYes);
	GUI_V2_MENU_ITEM(_choiceMenu.item[1], 1, 0x13, 0xD8, 0x1E, 0x48, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_choiceMenu.item[1].callback = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::choiceNo);
	for (int i = 2; i <= 6; ++i)
		_choiceMenu.item[i].enabled = false;
	for (int i = 0; i < 7; ++i)
		_choiceMenu.item[i].itemId = menuStr[3 * 8 + i + 1];
	
	GUI_V2_MENU(_loadMenu, -1, -1, 0x120, 0xA0, 0xF8, 0xF9, 0xFA, menuStr[4 * 8], 0xFB, -1, 8, 0, 6, 0x84, 0x16, 0x84, 0x7C);
	GUI_V2_MENU_ITEM(_loadMenu.item[0], 1, 0x29, -1, 0x27, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	GUI_V2_MENU_ITEM(_loadMenu.item[1], 1, 0x2A, -1, 0x38, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	GUI_V2_MENU_ITEM(_loadMenu.item[2], 1, 0x2B, -1, 0x49, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	GUI_V2_MENU_ITEM(_loadMenu.item[3], 1, 0x2C, -1, 0x5A, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	GUI_V2_MENU_ITEM(_loadMenu.item[4], 1, 0x2D, -1, 0x6B, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	for (int i = 0; i <= 4; ++i)
		_loadMenu.item[i].callback = clickLoadSlotFunctor;
	GUI_V2_MENU_ITEM(_loadMenu.item[5], 1, 0x0B, 0xB8, 0x86, 0x58, 0xF, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_loadMenu.item[5].callback = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::cancelLoadMenu);
	_loadMenu.item[6].enabled = false;
	for (int i = 0; i < 7; ++i)
		_loadMenu.item[i].itemId = menuStr[4 * 8 + i + 1];

	GUI_V2_MENU(_saveMenu, -1, -1, 0x120, 0xA0, 0xF8, 0xF9, 0xFA, menuStr[5 * 8], 0xFB, -1, 8, 0, 6, 0x84, 0x16, 0x84, 0x7C);
	GUI_V2_MENU_ITEM(_saveMenu.item[0], 1, 0x29, -1, 0x27, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	GUI_V2_MENU_ITEM(_saveMenu.item[1], 1, 0x2A, -1, 0x38, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	GUI_V2_MENU_ITEM(_saveMenu.item[2], 1, 0x2B, -1, 0x49, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	GUI_V2_MENU_ITEM(_saveMenu.item[3], 1, 0x2C, -1, 0x5A, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	GUI_V2_MENU_ITEM(_saveMenu.item[4], 1, 0x2D, -1, 0x6B, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	for (int i = 0; i <= 4; ++i)
		_saveMenu.item[i].callback = clickSaveSlotFunctor;
	GUI_V2_MENU_ITEM(_saveMenu.item[5], 1, 0x0B, 0xB8, 0x86, 0x58, 0xF, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_saveMenu.item[5].callback = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::cancelSaveMenu);
	_saveMenu.item[6].enabled = false;
	for (int i = 0; i < 7; ++i)
		_saveMenu.item[i].itemId = menuStr[5 * 8 + i + 1];

	GUI_V2_MENU(_savenameMenu, -1, -1, 0x140, 0x43, 0xF8, 0xF9, 0xFA, menuStr[6 * 8], 0xFB, -1, 8, 0, 2, -1, -1, -1, -1);
	GUI_V2_MENU_ITEM(_savenameMenu.item[0], 1, 0xD, 0x18, 0x2C, 0x58, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_savenameMenu.item[0].callback = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::finishSavename);
	GUI_V2_MENU_ITEM(_savenameMenu.item[1], 1, 0xB, 0xD0, 0x2C, 0x58, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_savenameMenu.item[1].callback = BUTTON_FUNCTOR(GUI_v2, this, &GUI_v2::cancelSavename);
	for (int i = 2; i <= 6; ++i)
		_savenameMenu.item[i].enabled = false;
	for (int i = 0; i < 7; ++i)
		_savenameMenu.item[i].itemId = menuStr[6 * 8 + i + 1];

	GUI_V2_MENU(_deathMenu, -1, -1, 0xD0, 0x4C, 0xF8, 0xF9, 0xFA, menuStr[7 * 8], 0xFB, -1, 8, 0, 2, -1, -1, -1, -1);
	GUI_V2_MENU_ITEM(_deathMenu.item[0], 1, 2, -1, 0x1E, 0xB4, 0x0F, 0xFC, 0xFD, 8, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_deathMenu.item[0].callback = clickLoadMenuFunctor;
	GUI_V2_MENU_ITEM(_deathMenu.item[1], 1, 5, -1, 0x2F, 0xB4, 0x0F, 0xFC, 0xFD, 8, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_deathMenu.item[1].callback = clickQuitGameFunctor;
	for (int i = 2; i <= 6; ++i)
		_deathMenu.item[i].enabled = false;
	for (int i = 0; i < 2; ++i)
		_deathMenu.item[i].itemId = menuStr[7 * 8 + i + 1];
}

const uint16 GUI_v2::_menuStringsTalkie[] = {
	0x001, 0x002, 0x003, 0x023, 0x004, 0x025, 0x005, 0x006,	// Main Menu String IDs
	0x025, 0x000, 0x000, 0x000, 0x010, 0x000, 0x000, 0x000,	// Options Menu String IDs
	0x007, 0x000, 0x000, 0x000, 0x010, 0x000, 0x000, 0x000,	// Audio Menu String IDs
	0x000, 0x014, 0x013, 0x000, 0x000, 0x000, 0x000, 0x000,	// Menu3 Menu String IDs
	0x008, 0x029, 0x02A, 0x02B, 0x02C, 0x02D, 0x00B, 0x000,	// Load Menu String IDs
	0x009, 0x029, 0x02A, 0x02B, 0x02C, 0x02D, 0x00B, 0x000,	// Save Menu String IDs
	0x00C, 0x00D, 0x00B, 0x000, 0x000, 0x000, 0x000, 0x000,	// Menu6 Menu String IDs
	0x00E, 0x002, 0x005, 0x000, 0x000, 0x000, 0x000, 0x000	// Death Menu String IDs
};

const uint16 GUI_v2::_menuStringsOther[] = {
	0x009, 0x00A, 0x00B, 0x001, 0x00C, 0x00D, 0x00E, 0x000,	// Main Menu String IDs
	0x00F, 0x02B, 0x02C, 0x02D, 0x02E, 0x018, 0x000, 0x000,	// Options Menu String IDs
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,	// Dummy
	0x000, 0x01C, 0x01B, 0x000, 0x000, 0x000, 0x000, 0x000,	// Menu3 Menu String IDs
	0x010, 0x02F, 0x030, 0x031, 0x032, 0x033, 0x013, 0x000,	// Load Menu String IDs
	0x011, 0x02F, 0x030, 0x031, 0x032, 0x033, 0x013, 0x000,	// Save Menu String IDs
	0x014, 0x015, 0x013, 0x3E8, 0x000, 0x000, 0x000, 0x000,	// Menu6 String IDs
	0x016, 0x00A, 0x00D, 0x000, 0x000, 0x000, 0x000, 0x000	// Death Menu String IDs
};

const int GUI_v2::_sliderBarsPosition[] = {
	0x92, 0x1F, 0x92, 0x30, 0x92, 0x41, 0x92, 0x52
};

const uint16 KyraEngine_v2::_itemMagicTable[] = {
	0x0D,  0x0A,  0x0B,    0,
	0x0D,  0x0B,  0x0A,    0,
	0x0D,  0x38,  0x37,    0,
	0x0D,  0x37,  0x38,    0,
	0x0D,  0x35,  0x36,    0,
	0x0D,  0x36,  0x35,    0,
	0x34,  0x27,  0x33,    0,
	0x41,  0x29,  0x49,    0,
	0x45,  0x29,  0x4A,    1,
	0x46,  0x29,  0x4A,    1,
	0x3C,  0x29,  0x4B,    1,
	0x34,  0x29,  0x4C,    0,
	0x3C,  0x49,  0x3B,    1,
	0x41,  0x4B,  0x3B,    0,
	0x3C,  0x4A,  0x3B,    1,
	0x34,  0x49,  0x3B,    0,
	0x41,  0x4C,  0x3B,    0,
	0x45,  0x4C,  0x3B,    1,
	0x46,  0x4C,  0x3B,    1,
	0x34,  0x4A,  0x3B,    0,
	0x0D,  0x67,  0x68,    0,
	0x0D,  0x68,  0x67,    0,
	0x0D,  0x69,  0x6A,    0,
	0x0D,  0x6A,  0x69,    0,
	0x0D,  0x6B,  0x6C,    0,
	0x0D,  0x6C,  0x6B,    0,
	0x0D,  0x88,  0x87,    0,
	0x0D,  0x87,  0x88,    0,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
};

const int KyraEngine_v2::_bookPageYOffset[] = {
	0, 0, 2, 2,
	0, 0, 2, 2,
	0, 0, 2, 2
};

const byte KyraEngine_v2::_bookTextColorMap[] = {
	0x00, 0xC7, 0xCF, 0x00
};

const int16 KyraEngine_v2::_cauldronProtectedItems[] = {
	0x07, 0x0D, 0x47, 0x48,
	0x29, 0x1A, 0x1C, 0x6D,
	0x4D, 0x3A, 0x0E, 0x0F,
	0x10, 0x11, 0x26, 0x3E,
	0x35, 0x40, 0x42, 0xA6,
	0xA4, 0xA5, 0x91, 0x95,
	0x99, 0xAC, 0xAE, 0xAF,
	0x8A, 0x79, 0x61, -1
};

const int16 KyraEngine_v2::_cauldronBowlTable[] = {
	0x0027, 0x0029,
	0x0028, 0x0029,
	0x0033, 0x0029,
	0x0049, 0x0029,
	0x004A, 0x0029,
	0x004B, 0x0029,
	0x004C, 0x0029,
	0x003B, 0x0029,
	0x0034, 0x0034,
	-1, -1
};

const int16 KyraEngine_v2::_cauldronMagicTable[] = {
	0x0, 0x16, 0x2, 0x1A,
	0x7, 0xA4, 0x5, 0x4D,
	0x1, 0xA5, 0x3, 0xA6,
	0x6, 0x6D, 0x4, 0x91,
	0xA, 0x99, 0xC, 0x95,
	0x9, 0xAC, -1, -1
};

const int16 KyraEngine_v2::_cauldronMagicTableScene77[] = {
	0x0, 0x16, 0x2, 0x1A,
	0x7, 0xAB, 0x5, 0x4D,
	0x1, 0xAE, 0x3, 0xAF,
	0x6, 0x6D, 0x4, 0x91,
	0xA, 0x99, 0xC, 0x95,
	0x9, 0xAC, -1, -1
};

const uint8 KyraEngine_v2::_cauldronStateTable[] = {
	3, 1, 3, 1, 1, 4, 4, 2,
	3, 1, 1, 3, 1, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3
};

const int16 KyraEngine_v2::_flaskTable[] = {
	0x19, 0x14, 0x15, 0x16, 0x17, 0x18, 0x34,
	0x1B, 0x39, 0x1A, 0x3A, 0x4D, 0x72, -1
};

const uint8 KyraEngine_v2::_rainbowRoomData[] = {
	0x02, 0xA9, 0x9E, 0x75, 0x73, 0x17, 0x00, 0xA0,
	0x08, 0x01, 0x19, 0x9F, 0x66, 0x05, 0x22, 0x7D,
	0x20, 0x25, 0x1D, 0x64, 0xA0, 0x78, 0x85, 0x3B,
	0x3C, 0x5E, 0x38, 0x45, 0x8F, 0x61, 0xA1, 0x71,
	0x47, 0x77, 0x86, 0x41, 0xA2, 0x5F, 0x03, 0x72,
	0x83, 0x9E, 0x84, 0x8E, 0xAD, 0xA8, 0x04, 0x79,
	0xAA, 0xA3, 0x06, 0x27, 0x8F, 0x9F, 0x0A, 0x76,
	0x46, 0x1E, 0x24, 0x63, 0x18, 0x69, 0x39, 0x1F,
	0x7E, 0xAD, 0x28, 0x60, 0x67, 0x21, 0x84, 0x34
};

// kyra 3 static res

const char *KyraEngine_v3::_soundList[] = {
	"ARREST1.AUD",
	"BATH1.AUD",
	"OCEAN1.AUD",
	"CLOWN1.AUD",
	"DARM2.AUD",
	"FALL1M.AUD",
	"FALL2.AUD",
	"FISH1.AUD",
	"FISHWNDR.AUD",
	"HERMAN1.AUD",
	"JAIL1.AUD",
	"JUNGLE1.AUD",
	"KATHY1.AUD",
	"NICESINE.AUD",
	"PEGASUS1.AUD",
	"PIRATE1.AUD",
	"PIRATE2.AUD",
	"PIRATE3.AUD",
	"POP3.AUD",
	"PORT1.AUD",
	"QUEEN1.AUD",
	"RUINS1.AUD",
	"SNAKES1.AUD",
	"SPRING1.AUD",
	"STATUE1.AUD",
	"STATUE2.AUD",
	"TITLE1.AUD",
	"UNDER1.AUD",
	"WALKCHP1.AUD",
	"YANK1.AUD",
	"ZAN2.AUD",
	"GROOVE2.AUD",
	"GROOVE3.AUD",
	"KING1.AUD",
	"KING2.AUD",
	"GROOVE1.AUD",
	"JAIL2.AUD",
	"SPIRIT1.AUD",
	"SPRING1A.AUD",
	"POP1.AUD",
	"POP2.AUD",
	"SQUIRL1.AUD"
};

const int KyraEngine_v3::_soundListSize = ARRAYSIZE(KyraEngine_v3::_soundList);

const char *KyraEngine_v3::_languageExtension[] = {
	"TRE",
	"TRF",
	"TRG"/*,
	"TRI",		Italian and Spanish was never included
	"TRS"*/
};

const int KyraEngine_v3::_languageExtensionSize = ARRAYSIZE(KyraEngine_v3::_languageExtension);

const KyraEngine_v3::ShapeDesc KyraEngine_v3::_shapeDescs[] = {
	{ 57, 91, -31, -82 },
	{ 57, 91, -31, -82 },
	{ 57, 91, -31, -82 },
	{ 57, 91, -31, -82 },
	{ 57, 91, -31, -82 },
	{ 82, 96, -43, -86 },
	{ 57, 91, -31, -82 },
	{ 57, 91, -31, -82 },
	{ 57, 91, -31, -82 },
	{ 69, 91, -31, -82 },
	{ 57, 91, -31, -82 },
	{ 57, 91, -31, -82 }
};

const int KyraEngine_v3::_shapeDescsSize = ARRAYSIZE(KyraEngine_v3::_shapeDescs);

const int8 KyraEngine_v3::_updateCharPosXTable[] = {
	0, 4, 4, 4, 0, -4, -4, -4
};

const int8 KyraEngine_v3::_updateCharPosYTable[] = {
	-2, -2, 0, 2, 2, 2, 0, -2
};

const uint8 KyraEngine_v3::_characterFrameTable[] = {
	0x36, 0x35, 0x35, 0x33, 0x32, 0x32, 0x34, 0x34
};

const uint8 KyraEngine_v3::_sfxFileMap[] = {
	0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
	0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
	0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x99, 0x00,
	0x46, 0x00, 0xA9, 0x00, 0x33, 0x00, 0x65, 0x00,
	0x9B, 0x00, 0x17, 0x00, 0xBB, 0x00, 0x64, 0x00,
	0x55, 0x00, 0xD5, 0x00, 0x66, 0x00, 0xB9, 0x00,
	0x9A, 0x00, 0xFF, 0x00, 0xCC, 0x00, 0x67, 0x00,
	0x2E, 0x00, 0xA1, 0x00, 0xD0, 0x00, 0x63, 0x00,
	0x89, 0x00, 0xBE, 0x00, 0x80, 0x00, 0x1D, 0x00,
	0x02, 0x00, 0x28, 0x00, 0x91, 0x00, 0x29, 0x00,
	0xCE, 0x00, 0x8F, 0x00, 0x49, 0x00, 0x2B, 0x00,
	0x2D, 0x00, 0x2C, 0x00, 0x3E, 0x00, 0x22, 0x00,
	0x80, 0x00, 0x9C, 0x00, 0x2E, 0x00, 0x04, 0x00,
	0x47, 0x00, 0xA8, 0x00, 0x51, 0x00, 0x52, 0x00,
	0x80, 0x00, 0x48, 0x00, 0x38, 0x0A, 0x0C, 0x00,
	0xD8, 0x00, 0xD1, 0x00, 0xD2, 0x00, 0xD3, 0x00,
	0xD1, 0x00, 0x6A, 0x00, 0x8A, 0x00, 0xC0, 0x00,
	0xC1, 0x00, 0xC2, 0x00, 0x9F, 0x00, 0xA3, 0x00,
	0x90, 0x00, 0xB6, 0x00, 0x37, 0x00, 0x71, 0x00,
	0x13, 0x00, 0x50, 0x00, 0x5A, 0x00, 0x6E, 0x00,
	0x70, 0x00, 0x11, 0x00, 0x16, 0x00, 0x14, 0x00,
	0x43, 0x00, 0xCD, 0x00, 0xAA, 0x00, 0x15, 0x00,
	0x83, 0x00, 0x19, 0x00, 0xB3, 0x00, 0x6F, 0x00,
	0x26, 0x00, 0xC8, 0x00, 0xA7, 0x00, 0x98, 0x00,
	0x87, 0x00, 0xC7, 0x00, 0xA2, 0x00, 0xB0, 0x00,
	0x12, 0x00, 0xD7, 0x00, 0x56, 0x00, 0x45, 0x00,
	0x4B, 0x00, 0xAF, 0x00, 0x3B, 0x00, 0x6C, 0x00,
	0x8E, 0x00, 0x39, 0x00, 0x38, 0x00, 0x92, 0x00,
	0x4B, 0x00, 0xD0, 0x00, 0x4A, 0x00, 0x9D, 0x00,
	0x7F, 0x00, 0x6D, 0x00, 0xFF, 0x00, 0xFF, 0x00,
	0x3D, 0x00, 0x72, 0x00, 0x40, 0x00, 0x66, 0x00,
	0x01, 0x00, 0xA5, 0x00, 0x00, 0x00, 0x3C, 0x00,
	0xAC, 0x00, 0x38, 0x00, 0x8B, 0x00, 0xDF, 0x00,
	0x0E, 0x00, 0x54, 0x00, 0xFF, 0x00, 0xFF, 0x00,
	0x94, 0x00, 0xAB, 0x00, 0x76, 0x00, 0x58, 0x00,
	0x6B, 0x00, 0x27, 0x00, 0xFF, 0x00, 0x77, 0x00,
	0xA6, 0x00, 0x63, 0x00, 0x9E, 0x00, 0xDE, 0x00,
	0x84, 0x00, 0x85, 0x00, 0x86, 0x00, 0x3F, 0x00,
	0xCC, 0x00, 0xCC, 0x00, 0xCC, 0x00, 0x93, 0x00,
	0x9D, 0x00, 0x75, 0x00, 0x75, 0x00, 0x75, 0x00,
	0x75, 0x00, 0x3A, 0x00, 0xFF, 0x00, 0xFF, 0x00,
	0xFF, 0x00, 0xAE, 0x00, 0x8C, 0x00, 0x20, 0x00,
	0xFF, 0x00, 0x32, 0x00, 0x32, 0x00, 0xFF, 0x00,
	0x4D, 0x00, 0xD9, 0x00, 0x88, 0x00, 0x4D, 0x00,
	0x4D, 0x00, 0x4D, 0x00, 0x4D, 0x00, 0xA0, 0x00,
	0x4C, 0x00, 0x8C, 0x00, 0x4C, 0x00, 0x4C, 0x00,
	0x8C, 0x00, 0x8C, 0x00, 0x5C, 0x00, 0x5D, 0x00,
	0x60, 0x00, 0x5F, 0x00, 0xC5, 0x00, 0xBF, 0x00,
	0xFF, 0x00, 0x4F, 0x00, 0x16, 0x00, 0x59, 0x00,
	0xFF, 0x00, 0x24, 0x00, 0xA4, 0x00, 0xCF, 0x00,
	0xFF, 0x00, 0x47, 0x00, 0x95, 0x00, 0x96, 0x00,
	0x7B, 0x00, 0xBD, 0x00, 0xFF, 0x00, 0x34, 0x00,
	0x35, 0x00, 0x36, 0x00, 0xDE, 0x00, 0xFF, 0x00,
	0x4B, 0x00, 0xD6, 0x00, 0xFF, 0x00, 0x61, 0x00,
	0x62, 0x00, 0xFF, 0x00, 0x78, 0x00, 0xFF, 0x00,
	0x44, 0x00, 0xB4, 0x00, 0xB5, 0x00, 0x42, 0x00,
	0x27, 0x00, 0xA2, 0x00, 0x27, 0x00, 0x5D, 0x00,
	0x7A, 0x00, 0x89, 0x00, 0x1A, 0x00, 0x0E, 0x00,
	0x82, 0x00, 0xFF, 0x00, 0x79, 0x00, 0x2A, 0x00,
	0x81, 0x00, 0xFF, 0x00, 0x74, 0x00, 0x4E, 0x00,
	0xB1, 0x00, 0x1B, 0x00, 0x2F, 0x00, 0xBA, 0x00,
	0xBB, 0x00, 0xBC, 0x00, 0xDA, 0x00, 0xDB, 0x00,
	0x18, 0x00, 0x5E, 0x00, 0x0D, 0x0A, 0x88, 0x00,
	0x1E, 0x00, 0x1F, 0x00, 0x20, 0x00, 0x21, 0x00,
	0x69, 0x00, 0x1C, 0x00, 0x7C, 0x00, 0x30, 0x00,
	0xC3, 0x00, 0xC4, 0x00, 0xAD, 0x00, 0x25, 0x00,
	0x53, 0x00, 0xB7, 0x00, 0xB8, 0x00, 0xDC, 0x00,
	0x8D, 0x00, 0xCB, 0x00, 0xD4, 0x00, 0xB2, 0x00,
	0xDD, 0x00, 0x57, 0x00, 0x41, 0x00, 0x10, 0x00,
	0x4C, 0x00, 0xC9, 0x00, 0xFF, 0x00, 0xFF, 0x00,
	0x7D, 0x00, 0x7E, 0x00, 0xCA, 0x00, 0x03, 0x00,
	0x04, 0x00, 0x05, 0x00, 0x06, 0x00, 0x07, 0x00,
	0x08, 0x00, 0x09, 0x00, 0x0A, 0x00, 0x0B, 0x00,
	0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
	0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
	0x23, 0x00, 0x97, 0x00, 0x73, 0x00
};

const int KyraEngine_v3::_sfxFileMapSize = ARRAYSIZE(KyraEngine_v3::_sfxFileMap);

const char *KyraEngine_v3::_sfxFileList[] = {
	"ALARM1",
	"ARMOIRE1",
	"ARROW1",
	"AUDLAFF1",
	"AUDLAFF2",
	"AUDLAFF3",
	"AUDLAFF4",
	"AUDLAFF5",
	"AUDLAFF6",
	"AUDLAFF7",
	"AUDLAFF8",
	"AUDLAFF9",
	"BARK22A",
	"BEAM1",
	"BEDSQK1",
	"BEDSQK2",
	"BIGCLOK1",
	"BIGDOR2",
	"BIRD4",
	"BIRD122",
	"BIRD222",
	"BIRD322",
	"BLAST22D",
	"BLINK1",
	"BOATMIX1",
	"BODYFAL1",
	"BOTLBLOW",
	"BOUNCE3",
	"BOUNCE5",
	"BOW2",
	"BUBL1",
	"BUBL2",
	"BUBL3",
	"BUBL4",
	"BUTTON1",
	"BUTTON2",
	"CANNON1",
	"CASHREG1",
	"CATHY1",
	"CHAIN1",
	"CHATTER1",
	"CHATTER2",
	"CHEESE1",
	"CHICHIC2",
	"CHIPLAF1",
	"CHIPROR1",
	"CLANG1",
	"CLDOOR1",
	"CLEAT1",
	"CLOTHES1",
	"COIN2",
	"COUNTER1",
	"CREAK1",
	"CREAK2",
	"CREAK3",
	"CRIKT22A",
	"CRMAD1",
	"CRNORM1",
	"CRUMBLE1",
	"CRUNCH1",
	"CRYSTAL1",
	"DFLY1",
	"DIAL1",
	"DIGDIRT1",
	"DIZZY1",
	"DODO1",
	"DOORBELL",
	"DOORCL1",
	"DOOROP1",
	"DRIP1",
	"DROPITM1",
	"EAT22A",
	"EATNUT1",
	"ELEC1",
	"EXPLODE2",
	"FALL1",
	"FALLM2",
	"FALLM3",
	"FESTRE1",
	"FISHLAF2",
	"FLAG22A",
	"FLAG22B",
	"FLAG22C",
	"FLPOOF1",
	"FOLDER1",
	"FROG1",
	"FROGJMP1",
	"FSHBUBL1",
	"FUNNEL1",
	"FUSE1",
	"GATE22A",
	"GEM1",
	"GEMFIRE1",
	"GEMLIT1",
	"GEMPUT1",
	"GEMRAIN1",
	"GEMWND1",
	"GIRLLAF1",
	"GIRLLAF2",
	"GLASBRK1",
	"GLOWY1",
	"GOODK33",
	"GROWTWIG",
	"GUNTHER3",
	"H2ODROP2",
	"H2OFALL1",
	"HAMMER1",
	"HAYFALL2",
	"HERMMAG1",
	"HIPRES1",
	"HITHED22",
	"HOWL1",
	"HUM1",
	"HYPNO1",
	"HYPNO2",
	"IMPACT1",
	"JOHAN1",
	"JUNGAMB2",
	"KISS1",
	"KISS2",
	"KNIFE",
	"KNIFHIT1",
	"KNIFSTAB",
	"KNOCK",
	"LAND1",
	"LEVIBAB1",
	"LEVIMAN1",
	"LID",
	"MACHMIX1",
	"MALCFALL",
	"MALCYAWN",
	"MJUMP1",
	"MOO1",
	"MOO2",
	"MOO3",
	"MORPH1",
	"MORPH2",
	"MORPH3",
	"MORPH4",
	"MOTHS1",
	"MSPLASH1",
	"MTLSLAM1",
	"MUDBATH1",
	"NAIL1",
	"NEIGH1",
	"NETCATCH",
	"NETMAL1",
	"NETRIP1",
	"OPDOOR1",
	"OWL1",
	"OWL2",
	"PEDAL3",
	"PEGWING1",
	"PICKUP1",
	"PLUCK3",
	"POLGULP1",
	"POOF1",
	"PORTAL1",
	"POURH2O1",
	"PRIMOR1",
	"PUMP1",
	"PUNCTRE1",
	"RATTLE1",
	"REV2",
	"RING",
	"ROAR3",
	"ROWBOAT1",
	"RUCKUS1",
	"RUMBLE1",
	"SCOLD1",
	"SCRATCH1",
	"SHOVEL1",
	"SHOWER2",
	"SLOTPUL1",
	"SNAKKILL",
	"SNAP1",
	"SNIFF1",
	"SNIFF2",
	"SNIFFM1",
	"SNIP22B",
	"SNORIN1",
	"SNOROUT1",
	"SNORT1",
	"SPITBAL1",
	"SPITBAL2",
	"SPLASH1",
	"SQUEAK1",
	"SQUEAK2",
	"SQUEAK3",
	"STATUE",
	"STAMPED1",
	"STARS1",
	"STONE1",
	"STONE2",
	"STONE3",
	"STRETCH1",
	"STRETCH2",
	"SUNRISE1",
	"SWALLOW1",
	"SWALLOW2",
	"SWAV22B",
	"TELBEL1",
	"TELBEL2",
	"TENNIS1",
	"THROW1",
	"THUMP1",
	"TOILET1",
	"TRAPDOR1",
	"TRICKLE",
	"TROLGRNT",
	"TROLYEL1",
	"TROLYEL2",
	"TUBEDOR1",
	"TWIGSNAP",
	"UMBRLA1",
	"UNLOK22A",
	"VACUUM",
	"WAVELT1",
	"WHIP1",
	"WHIP2",
	"WOODHIT1",
	"YAWN1",
	"ZING",
	"ZIPPER1"
};

const int KyraEngine_v3::_sfxFileListSize = ARRAYSIZE(KyraEngine_v3::_sfxFileList);

const uint8 KyraEngine_v3::_badConscienceFrameTable[] = {
	0x13, 0x13, 0x13, 0x18, 0x13, 0x13, 0x13, 0x13,
	0x13, 0x13, 0x13, 0x10, 0x13, 0x13, 0x13, 0x13,
	0x13, 0x13, 0x13, 0x18, 0x13, 0x13, 0x13, 0x13,
	0x15, 0x15, 0x14, 0x18, 0x14, 0x14, 0x14, 0x14,
	0x24, 0x24, 0x24, 0x24, 0x24, 0x1D, 0x1D, 0x1D
};

const uint8 KyraEngine_v3::_chapterLowestScene[] = {
	0x00, 0x00, 0x19, 0x2B, 0x33, 0x3B
};

const uint8 KyraEngine_v3::_vocHighTable[] = {
	0x64, 0x76, 0x82, 0x83, 0x92
};

const uint8 KyraEngine_v3::_inventoryX[] = {
	0x45, 0x61, 0x7D, 0x99, 0xB5,
	0x45, 0x61, 0x7D, 0x99, 0xB5
};

const uint8 KyraEngine_v3::_inventoryY[] = {
	0x9C, 0x9C, 0x9C, 0x9C, 0x9C,
	0xB2, 0xB2, 0xB2, 0xB2, 0xB2
};

const uint8 KyraEngine_v3::_trashItemList[] = {
	0x1E, 0x1D, 0x1C, 0x1F, 0x0F, 0x05, 0x04, 0x00,
	0x03, 0x22, 0x0B, 0x20, 0x21, 0x10, 0x11, 0x3A,
	0x39, 0x40, 0x3E, 0x3D, 0x3C, 0x3F, 0xFF
};

const uint8 KyraEngine_v3::_itemMagicTable[] = {
	0x06, 0x05, 0x07, 0xFE, 0x05, 0x06, 0x07, 0xFE,
	0x03, 0x00, 0x22, 0xFE, 0x00, 0x03, 0x22, 0xFE,
	0x10, 0x00, 0x20, 0x0F, 0x00, 0x10, 0x0F, 0x20,
	0x10, 0x22, 0x21, 0x0F, 0x22, 0x10, 0x0F, 0x21,
	0xFF, 0xFF, 0xFF, 0xFF
};

const uint8 KyraEngine_v3::_itemStringMap[] = {
	1, 0, 2, 0, 2, 2, 0, 0,
	2, 2, 2, 2, 2, 2, 2, 0,
	0, 0, 0, 0, 0, 0, 3, 1,
	2, 0, 2, 2, 0, 0, 0, 0,
	0, 0, 1, 2, 0, 2, 0, 2,
	0, 0, 2, 0, 0, 0, 0, 1,
	1, 0, 2, 2, 0, 0, 2, 0,
	0, 2, 0, 2, 2, 0, 0, 2,
	0, 0, 0, 0, 2, 0, 0, 2
};

const uint KyraEngine_v3::_itemStringMapSize = ARRAYSIZE(KyraEngine_v3::_itemStringMap);

const uint8 KyraEngine_v3::_itemStringPickUp[] = {
	0x4, 0x7, 0x0, 0xA
};

const uint8 KyraEngine_v3::_itemStringDrop[] = {
	0x5, 0x8, 0x1, 0xB
};

const uint8 KyraEngine_v3::_itemStringInv[] = {
	0x6, 0x9, 0x2, 0xC
};

const int8 KyraEngine_v3::_scoreTable[] = {
	10,  8,  5,  9, 10, 10,  7,  8,
	 9,  9,  8,  8,  7,  8,  5,  9,
	 6, 10,  7,  8,  5,  9,  6,  6,
	 7,  8,  5,  9,  6,  8,  7,  8,
	 5,  9,  6, 10,  7,  8,  5,  5,
	 5,  7,  5,  7, 10,  5, 10,  5,
	 5,  8,  6,  8,  7,  5,  5,  8,
	 6,  9,  5,  7,  6,  5,  5,  7,
	 7,  7,  6,  5,  8,  6, 10,  5,
	 7,  5, 10,  5,  5,  5,  5,  7,
	 5,  8,  9,  7,  7,  6, 10,  6,
	 5, 10,  8,  5,  8,  6, 10,  5,
	 5,  8,  8,  5,  7,  7,  7,  6,
	 8,  9,  8,  8,  6,  5,  7,  6,
	 5,  8, 15,  7,  9,  6,  6,  8,
	 5,  8, 15, 15,  5, 15,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,	 0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0 
}; 

const int KyraEngine_v3::_scoreTableSize = ARRAYSIZE(KyraEngine_v3::_scoreTable);

void KyraEngine_v3::initMainButtonList(bool disable) {
	if (!_mainButtonListInitialized) {
		_mainButtonData = new Button[14];
		assert(_mainButtonData);

		GUI_V3_BUTTON(_mainButtonData[0], 1, 0, 0, 4, 4, 4, 0x4487, 0,   5, 162, 50, 25, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
		GUI_V3_BUTTON(_mainButtonData[1], 2, 0, 0, 1, 1, 1, 0x4487, 0, 245, 156, 69, 33, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
		GUI_V3_BUTTON(_mainButtonData[2], 3, 0, 0, 1, 1, 1, 0x4487, 0, 215, 191, 24,  9, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
		GUI_V3_BUTTON(_mainButtonData[3], 4, 0, 0, 1, 1, 1, 0x4487, 0, 215, 155, 25, 36, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);

		Button::Callback buttonInventoryFunctor = BUTTON_FUNCTOR(KyraEngine_v3, this, &KyraEngine_v3::buttonInventory);
		for (int i = 0; i < 5; ++i) {
			GUI_V3_BUTTON(_mainButtonData[i+4], i+5, 0, 0, 0, 0, 0, 0x1100, 0, 67+i*28, 155, 27, 21, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
			_mainButtonData[i+4].buttonCallback = buttonInventoryFunctor;
		}

		for (int i = 0; i < 5; ++i) {
			GUI_V3_BUTTON(_mainButtonData[i+9], i+10, 0, 0, 0, 0, 0, 0x1100, 0, 67+i*28, 177, 27, 21, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
			_mainButtonData[i+9].buttonCallback = buttonInventoryFunctor;
		}

		for (int i = 0; i < 14; ++i)
			_mainButtonList = _gui->addButtonToList(_mainButtonList, &_mainButtonData[i]);

		_mainButtonListInitialized = true;
	}

	for (int i = 0; i < 14; ++i) {
		if (disable)
			_gui->flagButtonDisable(&_mainButtonData[i]);
		else
			_gui->flagButtonEnable(&_mainButtonData[i]);
	}
}

} // End of namespace Kyra

