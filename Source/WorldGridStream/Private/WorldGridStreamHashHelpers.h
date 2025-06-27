// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreTypes.h"
#include "HAL/PlatformString.h"
#include "Misc/AssertionMacros.h"
#include "Misc/CString.h"
#include "Misc/Char.h"
#include "Templates/UnrealTypeTraits.h"
#include "Traits/IsCharType.h"

template <typename... Types>
struct TTuple;

/** 
 * CRC hash generation for different types of input data
 **/
struct FWorldGridStreamHashHelpers
{
	/** lookup table with precalculated CRC values - slicing by 8 implementation */
	static uint32 CRCTablesSB8[8][256];

	/** generates CRC hash of the memory area */
	typedef uint32 (*MemCrc32Functor)( const void* Data, int32 Length, uint32 CRC );
	static MemCrc32Functor MemCrc32Func;
	[[nodiscard]] static FORCEINLINE uint32 MemCrc32(const void* Data, int32 Length, uint32 CRC = 0)
	{
		return MemCrc32Func(Data, Length, CRC);
	}

	/** generates CRC hash of the element */
	template <typename T>
	[[nodiscard]] static uint32 TypeCrc32( const T& Data, uint32 CRC=0 )
	{
		return MemCrc32(&Data, sizeof(T), CRC);
	}

	/** String CRC. */
	template <typename CharType>
	[[nodiscard]] static uint32 StrCrc32(const CharType* Data, uint32 CRC = 0)
	{
		// We ensure that we never try to do a StrCrc32 with a CharType of more than 4 bytes.  This is because
		// we always want to treat every CRC as if it was based on 4 byte chars, even if it's less, because we
		// want consistency between equivalent strings with different character types.
		static_assert(sizeof(CharType) <= 4, "StrCrc32 only works with CharType up to 32 bits.");

		if constexpr (sizeof(CharType) != 1)
		{
			CRC = ~CRC;
			while (CharType Ch = *Data++)
			{
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
				Ch >>= 8;
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
				Ch >>= 8;
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
				Ch >>= 8;
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
			}
			return ~CRC;
		}
		else
		{
			/* Special case for when CharType is a byte, which causes warnings when right-shifting by 8 */
			CRC = ~CRC;
			while (CharType Ch = *Data++)
			{
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC     ) & 0xFF];
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC     ) & 0xFF];
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC     ) & 0xFF];
			}
			return ~CRC;
		}
	}

	/** String CRC. */
	template <typename CharType>
	[[nodiscard]] static uint32 StrCrc32Len(const CharType* Data, int32 Length, uint32 CRC = 0)
	{
		// We ensure that we never try to do a StrCrc32 with a CharType of more than 4 bytes.  This is because
		// we always want to treat every CRC as if it was based on 4 byte chars, even if it's less, because we
		// want consistency between equivalent strings with different character types.
		static_assert(sizeof(CharType) <= 4, "StrCrc32Len only works with CharType up to 32 bits.");

		if constexpr (sizeof(CharType) != 1)
		{
			CRC = ~CRC;
			for (int32 Idx = 0; Idx < Length; ++Idx)
			{
				CharType Ch = Data[Idx];
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
				Ch >>= 8;
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
				Ch >>= 8;
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
				Ch >>= 8;
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
			}
			return ~CRC;
		}
		else
		{
			/* Special case for when CharType is a byte, which causes warnings when right-shifting by 8 */
			CRC = ~CRC;
			for (int32 Idx = 0; Idx < Length; ++Idx)
			{
				CharType Ch = Data[Idx];
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC ^ Ch) & 0xFF];
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC     ) & 0xFF];
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC     ) & 0xFF];
				CRC = (CRC >> 8) ^ CRCTablesSB8[0][(CRC     ) & 0xFF];
			}
			return ~CRC;
		}
	}

	FORCEINLINE const TCHAR* ToCStr(const TCHAR* Ptr)
	{
		return Ptr;
	}
};

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_5
#include "Templates/EnableIf.h"
#endif
