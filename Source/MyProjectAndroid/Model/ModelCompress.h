
#pragma once

#include "ModelFile.h"

namespace CompressUtil
{
	void CompressModel(FModel *InModel);
	bool CompressTexture(FTextureSourceInfo *TexInfo, ETexSlot slot);
	bool CompressTexture(FModelTexture *Tex);
	FName GetTextureCompressFormat(FTextureSourceInfo *TexInfo);
	bool BuildTexture(const TArray<FImage>& SourceMips, const FTextureBuildSettings& BuildSettings, TArray<FCompressedImage2D>& OutTextureMips);
}

