
#include "ModelTexture.h"
#include "ResourceMgr.h"

FString GetTexSlotName(ETexSlot Slot)
{
	switch (Slot)
	{
	case ETexSlot::eDiffuse:   return "Diffuse";
	case ETexSlot::eSpecular:  return "Specular";
	case ETexSlot::eEmissive:  return "EMissive";
	case ETexSlot::eRoughness: return "Roughness";
	case ETexSlot::eMetallic:return "Metallic";
	case ETexSlot::eOpacity: return "Opacity";
	case ETexSlot::eNormal:  return "Normal";
	default: break;
	}
	return "";
}

FString GetTexSlotKey(ETexSlot Slot)
{
	switch (Slot)
	{
	case ETexSlot::eDiffuse:   return "D";
	case ETexSlot::eSpecular:  return "S";
	case ETexSlot::eEmissive:  return "E";
	case ETexSlot::eRoughness: return "R";
	case ETexSlot::eMetallic:return "M";
	case ETexSlot::eOpacity: return "A";
	case ETexSlot::eNormal:  return "N";
	default: break;
	}
	return "";
}

ETexSlot GetSlotType(const FName &Name)
{
	static FName Diffuse(TEXT("Diffuse"));
	static FName Normal(TEXT("Normal"));
	static FName Specular(TEXT("Specular"));
	static FName EMissive(TEXT("EMissive"));
	static FName Roughness(TEXT("Roughness"));
	static FName Metallic(TEXT("Metallic"));
	static FName Opacity(TEXT("Opacity"));

	if (Name == Diffuse)
	{
		return ETexSlot::eDiffuse;
	}
	else if (Name == Normal)
	{
		return ETexSlot::eNormal;
	}
	else if (Name == Specular)
	{
		return ETexSlot::eSpecular;
	}
	else if (Name == EMissive)
	{
		return ETexSlot::eEmissive;
	}
	else if (Name == Roughness)
	{
		return ETexSlot::eRoughness;
	}
	else if (Name == Metallic)
	{
		return ETexSlot::eMetallic;
	}
	else if (Name == Opacity)
	{
		return ETexSlot::eOpacity;
	}
	return ETexSlot::eMax;
}

FModelTexture::FModelTexture()
{
}

FModelTexture::~FModelTexture()
{
	if (TexPtr.IsValid())
	{
		//UE_LOG(LogResMgr, Log, TEXT("Release Texture %s"), *TexPtr->GetName());
		TexPtr->ReleaseResource();
		TexPtr->ConditionalBeginDestroy();
	}
}

void FModelTexture::CopyFrom(FModelTexture &oth)
{
	Name = oth.Name;
	Source = oth.Source;
}

UTexture2D *FModelTexture::GetTexture()
{
	UTexture2D *Texture = NULL;

	if (TexPtr.IsValid())
	{
		Texture = TexPtr.Get();
	}
	else
	{
		/*if (Source.CompressedImages.Num() > 0)
		{
			FCompressedImage2D &CompressedImage = Source.CompressedImages[0];

			EPixelFormat PixelFormat = (EPixelFormat)CompressedImage.PixelFormat;

			Texture = UTexture2D::CreateTransient(CompressedImage.SizeX, CompressedImage.SizeY, PixelFormat);

			FTexture2DMipMap &FirstMip = Texture->PlatformData->Mips[0];
			FirstMip.SizeX = CompressedImage.SizeX;
			FirstMip.SizeY = CompressedImage.SizeY;
			Texture->NeverStream = 0;
			Texture->SRGB = 1;
			Texture->PlatformData->NumSlices = 1;
			Texture->PlatformData->PixelFormat = PixelFormat;
			check(CompressedImage.RawData.GetTypeSize() == 1);

			if (PixelFormat == PF_BC5)
			{
				Texture->CompressionSettings = TextureCompressionSettings::TC_Normalmap;
				Texture->LODGroup = TextureGroup::TEXTUREGROUP_WorldNormalMap;
				Texture->SRGB = 0;
			}

			void* NewMipData = FirstMip.BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(NewMipData, CompressedImage.RawData.GetData(), CompressedImage.RawData.Num());
			FirstMip.BulkData.Unlock();

			for (int32 i = 1; i < Source.CompressedImages.Num(); ++i)
			{
				FCompressedImage2D &Image = Source.CompressedImages[i];
				FTexture2DMipMap &SubMip = *(new (Texture->PlatformData->Mips) FTexture2DMipMap());
				SubMip.SizeX = Image.SizeX;
				SubMip.SizeY = Image.SizeY;

				SubMip.BulkData.Lock(LOCK_READ_WRITE);
				NewMipData = SubMip.BulkData.Realloc(Image.RawData.Num());
				FMemory::Memcpy(NewMipData, Image.RawData.GetData(), Image.RawData.Num());
				SubMip.BulkData.Unlock();
			}

			Texture->UpdateResource();
		}
		else
		{
			Texture = UTexture2D::CreateTransient(Source.SizeX, Source.SizeX, EPixelFormat::PF_B8G8R8A8);
			Texture->SRGB = Source.SRGB;

			FTexture2DMipMap &Mip = Texture->PlatformData->Mips[0];
			void* NewMipData = Mip.BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(NewMipData, Source.Data.GetData(), Source.Data.Num());
			Mip.BulkData.Unlock();

			Texture->UpdateResource();
		}*/

		TexPtr = Texture;
	}

	return Texture;
}

void FModelTexture::SetData(FTextureSourceInfo &SourceInfo)
{
	TexPtr.Reset();
	Source = SourceInfo;
}

void FTextureSourceInfo::Serialize(FArchive &Ar, uint32 Ver)
{
	if (Ar.IsSaving() || Ar.IsLoading())
	{
		Ar << SizeX;
		Ar << SizeY;
		Ar << SRGB;
		Ar << CompressionNoAlpha;
		Ar << bDitherMipMapAlpha;
		Ar << bUseLegacyGamma;
	}

//	if (Ar.IsSaving())
//	{
//		uint8 ByteFormat = (uint8)SourceFormat;
//		Ar << ByteFormat;
//
//		ByteFormat = (uint8)CompressionSettings;
//		Ar << ByteFormat;
//
//		int32 NumBytes = Data.Num();
//		Ar << NumBytes;
//		Ar.Serialize(Data.GetData(), NumBytes * sizeof(uint8));
//
//		int32 NumImages = CompressedImages.Num();
//		Ar << NumImages;
//
//		for (int32 i = 0; i < CompressedImages.Num(); ++i)
//		{
//			FCompressedImage2D &Image = CompressedImages[i];
//			Ar << Image.SizeX;
//			Ar << Image.SizeY;
//			Ar << Image.PixelFormat;
//
//			NumBytes = Image.RawData.Num();
//			Ar << NumBytes;
//			Ar.Serialize(Image.RawData.GetData(), Image.RawData.Num());
//		}
//	}
//	else if (Ar.IsLoading())
//	{
//		uint8 ByteFormat = 0;
//		Ar << ByteFormat;
//		SourceFormat = (ETextureSourceFormat)ByteFormat;
//
//		Ar << ByteFormat;
//		CompressionSettings = (TextureCompressionSettings)ByteFormat;
//
//#if		UE_SERVER || UE_EDITOR //raw texture data only use in server used for ouput png file(iray image format) .
//		int32 NumBytes = 0;
//		Ar << NumBytes;
//		Data.SetNum(NumBytes);
//		Ar.Serialize(Data.GetData(), NumBytes * sizeof(uint8));
//#else
//		int32 NumBytes = 0;
//		Ar << NumBytes;
//		Ar.Seek(Ar.Tell() + NumBytes);
//#endif
//
//		int32 NumImages = 0;
//		Ar << NumImages;
//		CompressedImages.SetNum(NumImages);
//
//		for (int32 i = 0; i < CompressedImages.Num(); ++i)
//		{
//			FCompressedImage2D &Image = CompressedImages[i];
//			Ar << Image.SizeX;
//			Ar << Image.SizeY;
//			Ar << Image.PixelFormat;
//
//			NumBytes = 0;
//			Ar << NumBytes;
//			Image.RawData.SetNum(NumBytes);
//			Ar.Serialize(Image.RawData.GetData(), NumBytes);
//		}
//	}
}

void FModelTexture::Serialize(FArchive &Ar, uint32 Ver)
{
	Source.Serialize(Ar, Ver);

	Decompress();
}


void FModelTexture::Decompress()
{
	//if (Source.Data.Num() == 0 && Source.CompressedImages.Num() > 0)
	//{
	//	FCompressedImage2D &CompressedImage = Source.CompressedImages[0];
	//	int32 szData = CompressedImage.SizeX*CompressedImage.SizeY * 4;
	//	Source.Data.SetNum(szData);
	//	
	//	unsigned long *RGBAImage = (unsigned long *)Source.Data.GetData();
	//	EPixelFormat PixelFormat = (EPixelFormat)CompressedImage.PixelFormat;

	//	if (PixelFormat == PF_DXT1)
	//	{
	//		BlockDecompressImageDXT1(CompressedImage.SizeX, CompressedImage.SizeY, (unsigned char *)CompressedImage.RawData.GetData(), RGBAImage);
	//	}
	//	else if(PixelFormat==PF_DXT5)
	//	{
	//		BlockDecompressImageDXT5(CompressedImage.SizeX, CompressedImage.SizeY, (unsigned char *)CompressedImage.RawData.GetData(), RGBAImage);
	//	}
	//	//UTextureImporter::ExportTGA(CompressedImage.SizeX, CompressedImage.SizeY, (uint8 *)RGBAImage, false, TEXT("f:/123.tga"));
	//}
}

unsigned long FModelTexture::PackRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	return ((a << 24) | (r << 16) | (g << 8) | b);
}

// void DecompressBlockDXT5(): Decompresses one block of a DXT5 texture and stores the resulting pixels at the appropriate offset in 'image'.
//
// unsigned long x:						x-coordinate of the first pixel in the block.
// unsigned long y:						y-coordinate of the first pixel in the block.
// unsigned long width: 				width of the texture being decompressed.
// unsigned long height:				height of the texture being decompressed.
// const unsigned char *blockStorage:	pointer to the block to decompress.
// unsigned long *image:				pointer to image where the decompressed pixel data should be stored.

void FModelTexture::DecompressBlockDXT5(unsigned long x, unsigned long y, unsigned long width, const unsigned char *blockStorage, unsigned long *image)
{
	unsigned char alpha0 = *reinterpret_cast<const unsigned char *>(blockStorage);
	unsigned char alpha1 = *reinterpret_cast<const unsigned char *>(blockStorage + 1);

	const unsigned char *bits = blockStorage + 2;
	unsigned long alphaCode1 = bits[2] | (bits[3] << 8) | (bits[4] << 16) | (bits[5] << 24);
	unsigned short alphaCode2 = bits[0] | (bits[1] << 8);

	unsigned short color0 = *reinterpret_cast<const unsigned short *>(blockStorage + 8);
	unsigned short color1 = *reinterpret_cast<const unsigned short *>(blockStorage + 10);

	unsigned long temp;

	temp = (color0 >> 11) * 255 + 16;
	unsigned char r0 = (unsigned char)((temp / 32 + temp) / 32);
	temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
	unsigned char g0 = (unsigned char)((temp / 64 + temp) / 64);
	temp = (color0 & 0x001F) * 255 + 16;
	unsigned char b0 = (unsigned char)((temp / 32 + temp) / 32);

	temp = (color1 >> 11) * 255 + 16;
	unsigned char r1 = (unsigned char)((temp / 32 + temp) / 32);
	temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
	unsigned char g1 = (unsigned char)((temp / 64 + temp) / 64);
	temp = (color1 & 0x001F) * 255 + 16;
	unsigned char b1 = (unsigned char)((temp / 32 + temp) / 32);

	unsigned long code = *reinterpret_cast<const unsigned long *>(blockStorage + 12);

	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < 4; i++)
		{
			int alphaCodeIndex = 3 * (4 * j + i);
			int alphaCode;

			if (alphaCodeIndex <= 12)
			{
				alphaCode = (alphaCode2 >> alphaCodeIndex) & 0x07;
			}
			else if (alphaCodeIndex == 15)
			{
				alphaCode = (alphaCode2 >> 15) | ((alphaCode1 << 1) & 0x06);
			}
			else // alphaCodeIndex >= 18 && alphaCodeIndex <= 45
			{
				alphaCode = (alphaCode1 >> (alphaCodeIndex - 16)) & 0x07;
			}

			unsigned char finalAlpha;
			if (alphaCode == 0)
			{
				finalAlpha = alpha0;
			}
			else if (alphaCode == 1)
			{
				finalAlpha = alpha1;
			}
			else
			{
				if (alpha0 > alpha1)
				{
					finalAlpha = ((8 - alphaCode)*alpha0 + (alphaCode - 1)*alpha1) / 7;
				}
				else
				{
					if (alphaCode == 6)
						finalAlpha = 0;
					else if (alphaCode == 7)
						finalAlpha = 255;
					else
						finalAlpha = ((6 - alphaCode)*alpha0 + (alphaCode - 1)*alpha1) / 5;
				}
			}

			unsigned char colorCode = (code >> 2 * (4 * j + i)) & 0x03;

			unsigned long finalColor = 0;
			switch (colorCode)
			{
			case 0:
				finalColor = PackRGBA(r0, g0, b0, finalAlpha);
				break;
			case 1:
				finalColor = PackRGBA(r1, g1, b1, finalAlpha);
				break;
			case 2:
				finalColor = PackRGBA((2 * r0 + r1) / 3, (2 * g0 + g1) / 3, (2 * b0 + b1) / 3, finalAlpha);
				break;
			case 3:
				finalColor = PackRGBA((r0 + 2 * r1) / 3, (g0 + 2 * g1) / 3, (b0 + 2 * b1) / 3, finalAlpha);
				break;
			}

			if (x + i < width)
				image[(y + j)*width + (x + i)] = finalColor;
		}
	}
}

// void BlockDecompressImageDXT5(): Decompresses all the blocks of a DXT5 compressed texture and stores the resulting pixels in 'image'.
//
// unsigned long width:					Texture width.
// unsigned long height:				Texture height.
// const unsigned char *blockStorage:	pointer to compressed DXT5 blocks.
// unsigned long *image:				pointer to the image where the decompressed pixels will be stored.

void FModelTexture::BlockDecompressImageDXT5(unsigned long width, unsigned long height, const unsigned char *blockStorage, unsigned long *image)
{
	unsigned long blockCountX = (width + 3) / 4;
	unsigned long blockCountY = (height + 3) / 4;
	unsigned long blockWidth = (width < 4) ? width : 4;
	unsigned long blockHeight = (height < 4) ? height : 4;

	for (unsigned long j = 0; j < blockCountY; j++)
	{
		for (unsigned long i = 0; i < blockCountX; i++) DecompressBlockDXT5(i * 4, j * 4, width, blockStorage + i * 16, image);
		blockStorage += blockCountX * 16;
	}
}


// void DecompressBlockDXT1(): Decompresses one block of a DXT1 texture and stores the resulting pixels at the appropriate offset in 'image'.
//
// unsigned long x:						x-coordinate of the first pixel in the block.
// unsigned long y:						y-coordinate of the first pixel in the block.
// unsigned long width: 				width of the texture being decompressed.
// unsigned long height:				height of the texture being decompressed.
// const unsigned char *blockStorage:	pointer to the block to decompress.
// unsigned long *image:				pointer to image where the decompressed pixel data should be stored.

void FModelTexture::DecompressBlockDXT1(unsigned long x, unsigned long y, unsigned long width, const unsigned char *blockStorage, unsigned long *image)
{
	unsigned short color0 = *reinterpret_cast<const unsigned short *>(blockStorage);
	unsigned short color1 = *reinterpret_cast<const unsigned short *>(blockStorage + 2);

	unsigned long temp;

	temp = (color0 >> 11) * 255 + 16;
	unsigned char r0 = (unsigned char)((temp / 32 + temp) / 32);
	temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
	unsigned char g0 = (unsigned char)((temp / 64 + temp) / 64);
	temp = (color0 & 0x001F) * 255 + 16;
	unsigned char b0 = (unsigned char)((temp / 32 + temp) / 32);

	temp = (color1 >> 11) * 255 + 16;
	unsigned char r1 = (unsigned char)((temp / 32 + temp) / 32);
	temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
	unsigned char g1 = (unsigned char)((temp / 64 + temp) / 64);
	temp = (color1 & 0x001F) * 255 + 16;
	unsigned char b1 = (unsigned char)((temp / 32 + temp) / 32);

	unsigned long code = *reinterpret_cast<const unsigned long *>(blockStorage + 4);

	for (int j = 0; j < 4; j++)
	{
		for (int i = 0; i < 4; i++)
		{
			unsigned long finalColor = 0;
			unsigned char positionCode = (code >> 2 * (4 * j + i)) & 0x03;

			if (color0 > color1)
			{
				switch (positionCode)
				{
				case 0:
					finalColor = PackRGBA(r0, g0, b0, 255);
					break;
				case 1:
					finalColor = PackRGBA(r1, g1, b1, 255);
					break;
				case 2:
					finalColor = PackRGBA((2 * r0 + r1) / 3, (2 * g0 + g1) / 3, (2 * b0 + b1) / 3, 255);
					break;
				case 3:
					finalColor = PackRGBA((r0 + 2 * r1) / 3, (g0 + 2 * g1) / 3, (b0 + 2 * b1) / 3, 255);
					break;
				}
			}
			else
			{
				switch (positionCode)
				{
				case 0:
					finalColor = PackRGBA(r0, g0, b0, 255);
					break;
				case 1:
					finalColor = PackRGBA(r1, g1, b1, 255);
					break;
				case 2:
					finalColor = PackRGBA((r0 + r1) / 2, (g0 + g1) / 2, (b0 + b1) / 2, 255);
					break;
				case 3:
					finalColor = PackRGBA(0, 0, 0, 255);
					break;
				}
			}

			if (x + i < width)
				image[(y + j)*width + (x + i)] = finalColor;
		}
	}
}

// void BlockDecompressImageDXT1(): Decompresses all the blocks of a DXT1 compressed texture and stores the resulting pixels in 'image'.
//
// unsigned long width:					Texture width.
// unsigned long height:				Texture height.
// const unsigned char *blockStorage:	pointer to compressed DXT1 blocks.
// unsigned long *image:				pointer to the image where the decompressed pixels will be stored.

void FModelTexture::BlockDecompressImageDXT1(unsigned long width, unsigned long height, const unsigned char *blockStorage, unsigned long *image)
{
	unsigned long blockCountX = (width + 3) / 4;
	unsigned long blockCountY = (height + 3) / 4;
	unsigned long blockWidth = (width < 4) ? width : 4;
	unsigned long blockHeight = (height < 4) ? height : 4;

	for (unsigned long j = 0; j < blockCountY; j++)
	{
		for (unsigned long i = 0; i < blockCountX; i++) DecompressBlockDXT1(i * 4, j * 4, width, blockStorage + i * 8, image);
		blockStorage += blockCountX * 8;
	}
}



