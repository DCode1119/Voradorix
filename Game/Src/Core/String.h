#pragma once

#include <string>
#include <cstdint>
#include <SFML/System/String.hpp>

#include "Core/Common.h"

struct FVrdxString
{
public:
	FVrdxString() = default;
	FVrdxString(const FVrdxString&) = default;
	FVrdxString& operator=(const FVrdxString&) = default;
	FVrdxString(FVrdxString&&) = default;
	FVrdxString& operator=(FVrdxString&&) = default;

	FVrdxString(const char* Utf8Str);
	FVrdxString(const std::string& Utf8Str);
	explicit FVrdxString(const sf::String& SfStr);

	VRDX_NO_DISCARD std::string ToUtf8() const;
	VRDX_NO_DISCARD const char* ToCStr() const;

	VRDX_NO_DISCARD sf::String ToSfString() const;

	VRDX_NO_DISCARD size_t Length() const { return Buffer.size(); }
	VRDX_NO_DISCARD bool IsEmpty() const { return Buffer.empty(); }

	VRDX_NO_DISCARD FVrdxString Left(const size_t Count) const;
	VRDX_NO_DISCARD FVrdxString Right(const size_t Offset) const;
	VRDX_NO_DISCARD FVrdxString Substr(const size_t Offset, const size_t Count = npos) const;

	VRDX_NO_DISCARD char32_t operator[](const size_t Index) const;
	VRDX_NO_DISCARD char32_t CodePointAt(const size_t Index) const;

	VRDX_NO_DISCARD char32_t Front() const;
	VRDX_NO_DISCARD char32_t Back() const;

	FVrdxString operator+(const FVrdxString& Other) const;
	FVrdxString& operator+=(const FVrdxString& Other);
	FVrdxString& operator+=(const char32_t CodePoint);

	bool operator==(const FVrdxString& Other) const;
	bool operator!=(const FVrdxString& Other) const;
	bool operator<(const FVrdxString& Other) const;

	static const FVrdxString Empty;

	static constexpr size_t npos = static_cast<size_t>(-1);

private:
	std::u32string Buffer;
	mutable std::string Utf8Cache;
	mutable bool bCacheValid = false;

	static void AppendUtf8(std::string& Out, char32_t CodePoint);
	static char32_t DecodeUtf8(const char*& Input, const char* End);
};

// ============================================================================
// UTF-8 → UTF-32 생성자
// ============================================================================

inline FVrdxString::FVrdxString(const char* Utf8Str)
{
	if (!Utf8Str)
	{
		return;
	}

	const char* Ptr = Utf8Str;
	const char* End = Ptr + std::char_traits<char>::length(Utf8Str);

	while (Ptr < End)
	{
		Buffer += DecodeUtf8(Ptr, End);
	}
}

inline FVrdxString::FVrdxString(const std::string& Utf8Str)
	: FVrdxString(Utf8Str.c_str())
{
}

// ============================================================================
// sf::String → UTF-32 생성자
// ============================================================================

inline FVrdxString::FVrdxString(const sf::String& SfStr)
{
	Buffer.reserve(SfStr.getSize());
	for (size_t i = 0; i < SfStr.getSize(); ++i)
	{
		Buffer += static_cast<char32_t>(SfStr[i]);
	}
}

// ============================================================================
// UTF-8 출력
// ============================================================================

inline std::string FVrdxString::ToUtf8() const
{
	if (!bCacheValid)
	{
		Utf8Cache.clear();
		for (char32_t CP : Buffer)
		{
			AppendUtf8(Utf8Cache, CP);
		}
		bCacheValid = true;
	}
	return Utf8Cache;
}

inline const char* FVrdxString::ToCStr() const
{
	if (!bCacheValid)
	{
		Utf8Cache.clear();
		for (char32_t CP : Buffer)
		{
			AppendUtf8(Utf8Cache, CP);
		}
		bCacheValid = true;
	}
	return Utf8Cache.c_str();
}

// ============================================================================
// SFML 연동
// ============================================================================

inline sf::String FVrdxString::ToSfString() const
{
	return sf::String(Buffer);
}

// ============================================================================
// 코드 포인트 단위 부분 문자열
// ============================================================================

inline FVrdxString FVrdxString::Left(const size_t Count) const
{
	const size_t Len = Buffer.size();
	const size_t Actual = (Count < Len) ? Count : Len;

	FVrdxString Result;
	Result.Buffer.assign(Buffer.begin(), Buffer.begin() + Actual);
	return Result;
}

inline FVrdxString FVrdxString::Right(const size_t Offset) const
{
	if (Offset >= Buffer.size())
	{
		return FVrdxString();
	}

	FVrdxString Result;
	Result.Buffer.assign(Buffer.begin() + Offset, Buffer.end());
	return Result;
}

inline FVrdxString FVrdxString::Substr(const size_t Offset, const size_t Count) const
{
	const size_t Len = Buffer.size();

	if (Offset >= Len)
	{
		return FVrdxString();
	}

	const size_t End = (Count != npos && Offset + Count < Len) ? Offset + Count : Len;

	FVrdxString Result;
	Result.Buffer.assign(Buffer.begin() + Offset, Buffer.begin() + End);
	return Result;
}

// ============================================================================
// 인덱스 접근
// ============================================================================

inline char32_t FVrdxString::operator[](const size_t Index) const
{
	return Buffer[Index];
}

inline char32_t FVrdxString::CodePointAt(const size_t Index) const
{
	return Buffer.at(Index);
}

inline char32_t FVrdxString::Front() const
{
	return Buffer.front();
}

inline char32_t FVrdxString::Back() const
{
	return Buffer.back();
}

// ============================================================================
// 연결
// ============================================================================

inline FVrdxString FVrdxString::operator+(const FVrdxString& Other) const
{
	FVrdxString Result(*this);
	Result += Other;
	return Result;
}

inline FVrdxString& FVrdxString::operator+=(const FVrdxString& Other)
{
	Buffer += Other.Buffer;
	bCacheValid = false;
	return *this;
}

inline FVrdxString& FVrdxString::operator+=(const char32_t CodePoint)
{
	Buffer += CodePoint;
	bCacheValid = false;
	return *this;
}

// ============================================================================
// 비교
// ============================================================================

inline bool FVrdxString::operator==(const FVrdxString& Other) const
{
	return Buffer == Other.Buffer;
}

inline bool FVrdxString::operator!=(const FVrdxString& Other) const
{
	return Buffer != Other.Buffer;
}

inline bool FVrdxString::operator<(const FVrdxString& Other) const
{
	return Buffer < Other.Buffer;
}

// ============================================================================
// UTF-8 인코딩 / 디코딩
// ============================================================================

inline void FVrdxString::AppendUtf8(std::string& Out, const char32_t CodePoint)
{
	if (CodePoint <= 0x7F)
	{
		Out += static_cast<char>(CodePoint);
	}
	else if (CodePoint <= 0x7FF)
	{
		Out += static_cast<char>(0xC0 | (CodePoint >> 6));
		Out += static_cast<char>(0x80 | (CodePoint & 0x3F));
	}
	else if (CodePoint <= 0xFFFF)
	{
		if (CodePoint >= 0xD800 && CodePoint <= 0xDFFF)
		{
			AppendUtf8(Out, 0xFFFD);
			return;
		}
		Out += static_cast<char>(0xE0 | (CodePoint >> 12));
		Out += static_cast<char>(0x80 | ((CodePoint >> 6) & 0x3F));
		Out += static_cast<char>(0x80 | (CodePoint & 0x3F));
	}
	else if (CodePoint <= 0x10FFFF)
	{
		Out += static_cast<char>(0xF0 | (CodePoint >> 18));
		Out += static_cast<char>(0x80 | ((CodePoint >> 12) & 0x3F));
		Out += static_cast<char>(0x80 | ((CodePoint >> 6) & 0x3F));
		Out += static_cast<char>(0x80 | (CodePoint & 0x3F));
	}
	else
	{
		AppendUtf8(Out, 0xFFFD);
	}
}

inline char32_t FVrdxString::DecodeUtf8(const char*& Input, const char* End)
{
	const auto Byte = static_cast<uint8_t>(*Input);

	if (Byte <= 0x7F)
	{
		++Input;
		return Byte;
	}

	uint32_t CP;
	size_t ExtraBytes;
	char32_t MinCP;

	if ((Byte & 0xE0) == 0xC0)
	{
		CP = Byte & 0x1F;
		ExtraBytes = 1;
		MinCP = 0x80;
	}
	else if ((Byte & 0xF0) == 0xE0)
	{
		CP = Byte & 0x0F;
		ExtraBytes = 2;
		MinCP = 0x800;
	}
	else if ((Byte & 0xF8) == 0xF0)
	{
		CP = Byte & 0x07;
		ExtraBytes = 3;
		MinCP = 0x10000;
	}
	else
	{
		++Input;
		return 0xFFFD;
	}

	++Input;

	for (size_t i = 0; i < ExtraBytes; ++i)
	{
		if (Input >= End)
		{
			return 0xFFFD;
		}

		const auto CB = static_cast<uint8_t>(*Input);
		if ((CB & 0xC0) != 0x80)
		{
			return 0xFFFD;
		}

		CP = (CP << 6) | (CB & 0x3F);
		++Input;
	}

	if (CP < MinCP || (CP >= 0xD800 && CP <= 0xDFFF) || CP > 0x10FFFF)
	{
		return 0xFFFD;
	}

	return static_cast<char32_t>(CP);
}
