/**
 * Error implementation.
 * エラーの実装ね。
 *
 * Author: KleaSCM
 * Email: KleaSCM@gmail.com
 */
#include <TohruPhysics/Error.h>

const char *ErrorCodeToString(ErrorCode Code) {
	switch (Code) {
	case ErrorCode_Ok:               return "Ok";
	case ErrorCode_OutOfMemory:      return "OutOfMemory";
	case ErrorCode_InvalidParameter: return "InvalidParameter";
	case ErrorCode_InitFailed:       return "InitFailed";
	default:                         return "Unknown";
	}
}
