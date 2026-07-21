/**
 * StringTable implementation for TohruPhysics GUI.
 * TohruPhysics GUI用の文字列テーブル実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include "StringTable.h"
#include "En.h"
#include "Ja.h"

const char *StringTableGet(Language Lang, StrKey Key) {
	if (Key < 0 || Key >= StrKey_Count) {
		return "";
	}
	if (Lang == Language_Japanese) {
		return GJaStrings[Key];
	}
	return GEnStrings[Key];
}
