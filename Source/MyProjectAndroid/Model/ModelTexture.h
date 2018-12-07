
#pragma once

#include "TextureCompressorModule.h"

enum ETexSlot
{
	eDiffuse,
	eSpecular,
	eEmissive,
	eRoughness,
	eMetallic,
	eOpacity,
	eNormal,
	eMax
};

extern FString  GetTexSlotName(ETexSlot Slot);
extern FString  GetTexSlotKey(ETexSlot Slot);
extern ETexSlot GetSlotType(const FName &Name);
class FTextureSourceInfo
{
public:
	FTextureSourceInfo()
	{
		Reset();
	}

	void Reset()
	{
		Data.Reset();
		CompressionSettings = TC_Default;
		SizeX = SizeY = 0;
		CompressionNoAlpha = false;
		bUseLegacyGamma = false;
		bDitherMipMapAlpha = false;
		SRGB = true;
	}

	void Init(int32 Width, int32 Height, ETextureSourceFormat InFormat, bool bSRGB, const uint8 *InData, int32 NumBytes, TextureCompressionSettings = TC_Default)
	{
		SizeX = Width;
		SizeY = Height;
		SourceFormat = InFormat;
		SRGB = bSRGB;
		Data.SetNum(NumBytes);
		FMemory::Memcpy(Data.GetData(), InData, NumBytes);
	}

	void Serialize(FArchive &Ar, uint32 Ver);

	void SetNum(int32 Num) { Data.SetNum(Num); }

	TArray<FCompressedImage2D>	CompressedImages;
	ETextureSourceFormat		SourceFormat;
	TextureCompressionSettings	CompressionSettings;
	int32						SizeX, SizeY;
	TArray<uint8>				Data;
	bool						SRGB;
	bool						CompressionNoAlpha;
	bool						bDitherMipMapAlpha;
	bool						bUseLegacyGamma;
};

struct FModelTexture
{
	FModelTexture();
	~FModelTexture();
	UTexture2D *GetTexture();
	void CopyFrom(FModelTexture &oth);
	void SetData(FTextureSourceInfo &SourceInfo);
	void Serialize(FArchive &Ar, uint32 Ver);
	void Decompress();
	unsigned long PackRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
	void BlockDecompressImageDXT1(unsigned long width, unsigned long height, const unsigned char *blockStorage, unsigned long *image);
	void DecompressBlockDXT1(unsigned long x, unsigned long y, unsigned long width, const unsigned char *blockStorage, unsigned long *image);
	void DecompressBlockDXT5(unsigned long x, unsigned long y, unsigned long width, const unsigned char *blockStorage, unsigned long *image);
	void BlockDecompressImageDXT5(unsigned long width, unsigned long height, const unsigned char *blockStorage, unsigned long *image);
	FName					Name;
	FTextureSourceInfo		Source;
protected:
	TWeakObjectPtr<UTexture2D> TexPtr;
};

