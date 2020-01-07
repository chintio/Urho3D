//
// Copyright (c) 2008-2019 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../Precompiled.h"

#include "../Graphics/Graphics.h"
#include "../Graphics/GraphicsImpl.h"
#include "../Graphics/Material.h"
#include "../IO/FileSystem.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/XMLFile.h"

#include "../DebugNew.h"

namespace Urho3D
{

static const char* addressModeNames[] =
{
    "wrap",
    "mirror",
    "clamp",
    "border",
    nullptr
};

static const char* filterModeNames[] =
{
    "nearest",
    "bilinear",
    "trilinear",
    "anisotropic",
    "nearestanisotropic",
    "default",
    nullptr
};

Texture::Texture(Context* context) :
    ResourceWithMetadata(context),
    GPUObject(GetSubsystem<Graphics>())
{
}

Texture::~Texture() = default;

// 设置请求的mip级别数。要在设置大小之前调用。
// 默认值（0）根据需要分配尽可能多的mip级别以达到1x1大小。将值1设置为禁用mipmapping。
// 请注意，rendertargets需要在渲染后动态重新生成mips，这可能会降低性能。
// 屏幕缓冲区和渲染器分配的阴影贴图将禁用mipmaps。
void Texture::SetNumLevels(unsigned levels)
{
    if (usage_ > TEXTURE_RENDERTARGET)
        requestedLevels_ = 1;
    else
        requestedLevels_ = levels;
}

// 过滤器模式，包括最近点采样（D3DTEXF_POINT）、线性（D3DTEXF_LINEAR）、各向异性（D3DTEXF_ANISOTROPIC）
void Texture::SetFilterMode(TextureFilterMode mode)
{
    filterMode_ = mode;
    parametersDirty_ = true;
}

// 寻址模式，包括重复（D3DADDRESS_WRAP）、边界颜色（D3DADDRESS_BORDER）、箝位（D3DADDRESS_CLAMP）、镜像（D3DADDRESS_MIRROR）
void Texture::SetAddressMode(TextureCoordinate coord, TextureAddressMode mode)
{
    addressModes_[coord] = mode;
    parametersDirty_ = true;
}

// 各向异性等级SetSamplerState(index, D3DSAMP_MAXANISOTROPY, level)
void Texture::SetAnisotropy(unsigned level)
{
    anisotropy_ = level;
    parametersDirty_ = true;
}

void Texture::SetShadowCompare(bool enable)
{
    shadowCompare_ = enable;
    parametersDirty_ = true;
}

void Texture::SetBorderColor(const Color& color)
{
    borderColor_ = color;
    parametersDirty_ = true;
}

void Texture::SetBackupTexture(Texture* texture)
{
    backupTexture_ = texture;
}

// toSkip以下将不采用
void Texture::SetMipsToSkip(MaterialQuality quality, int toSkip)
{
    if (quality >= QUALITY_LOW && quality < MAX_TEXTURE_QUALITY_LEVELS)
    {
        mipsToSkip_[quality] = (unsigned)toSkip;

        // Make sure a higher quality level does not actually skip more mips
		// 越高质量级的显示效果，会要求mipmap的渐进层级越精细。因此要求mipsToSkip_[i] <= mipsToSkip_[i - 1]
        for (int i = 1; i < MAX_TEXTURE_QUALITY_LEVELS; ++i)
        {
            if (mipsToSkip_[i] > mipsToSkip_[i - 1])
                mipsToSkip_[i] = mipsToSkip_[i - 1];
        }
    }
}

int Texture::GetMipsToSkip(MaterialQuality quality) const
{
    return (quality >= QUALITY_LOW && quality < MAX_TEXTURE_QUALITY_LEVELS) ? mipsToSkip_[quality] : 0;
}

int Texture::GetLevelWidth(unsigned level) const
{
    if (level > levels_)
        return 0;
    return Max(width_ >> level, 1);
}

int Texture::GetLevelHeight(unsigned level) const
{
    if (level > levels_)
        return 0;
    return Max(height_ >> level, 1);
}

int Texture::GetLevelDepth(unsigned level) const
{
    if (level > levels_)
        return 0;
    return Max(depth_ >> level, 1);
}

unsigned Texture::GetDataSize(int width, int height) const
{
    if (IsCompressed())
        return GetRowDataSize(width) * ((height + 3) >> 2u);
    else
        return GetRowDataSize(width) * height;
}

unsigned Texture::GetDataSize(int width, int height, int depth) const
{
    return depth * GetDataSize(width, height);
}

unsigned Texture::GetComponents() const
{
    if (!width_ || IsCompressed())
        return 0;
    else
        return GetRowDataSize(width_) / width_;
}

void Texture::SetParameters(XMLFile* file)
{
    if (!file)
        return;

    XMLElement rootElem = file->GetRoot();
    SetParameters(rootElem);
}

// 读取纹理文件的配置
void Texture::SetParameters(const XMLElement& element)
{
    LoadMetadataFromXML(element);
    for (XMLElement paramElem = element.GetChild(); paramElem; paramElem = paramElem.GetNext())
    {
        String name = paramElem.GetName();

        if (name == "address")
        {
            String coord = paramElem.GetAttributeLower("coord");
            if (coord.Length() >= 1)
            {
                auto coordIndex = (TextureCoordinate)(coord[0] - 'u');
                String mode = paramElem.GetAttributeLower("mode");
                SetAddressMode(coordIndex, (TextureAddressMode)GetStringListIndex(mode.CString(), addressModeNames, ADDRESS_WRAP));
            }
        }

        if (name == "border")
            SetBorderColor(paramElem.GetColor("color"));

        if (name == "filter")
        {
            String mode = paramElem.GetAttributeLower("mode");
            SetFilterMode((TextureFilterMode)GetStringListIndex(mode.CString(), filterModeNames, FILTER_DEFAULT));
            if (paramElem.HasAttribute("anisotropy"))
                SetAnisotropy(paramElem.GetUInt("anisotropy"));
        }

        if (name == "mipmap")
            SetNumLevels(paramElem.GetBool("enable") ? 0 : 1);

        if (name == "quality")
        {
            if (paramElem.HasAttribute("low"))
                SetMipsToSkip(QUALITY_LOW, paramElem.GetInt("low"));
            if (paramElem.HasAttribute("med"))
                SetMipsToSkip(QUALITY_MEDIUM, paramElem.GetInt("med"));
            if (paramElem.HasAttribute("medium"))
                SetMipsToSkip(QUALITY_MEDIUM, paramElem.GetInt("medium"));
            if (paramElem.HasAttribute("high"))
                SetMipsToSkip(QUALITY_HIGH, paramElem.GetInt("high"));
        }

        if (name == "srgb")
            SetSRGB(paramElem.GetBool("enable"));
    }
}

void Texture::SetParametersDirty()
{
    parametersDirty_ = true;
}

void Texture::SetLevelsDirty()
{
    if (usage_ == TEXTURE_RENDERTARGET && levels_ > 1)
        levelsDirty_ = true;
}

// 检测最大mips层数
unsigned Texture::CheckMaxLevels(int width, int height, unsigned requestedLevels)
{
    unsigned maxLevels = 1;
    while (width > 1 || height > 1)
    {
        ++maxLevels;
        width = width > 1 ? (width >> 1u) : 1;
        height = height > 1 ? (height >> 1u) : 1;
    }

    if (!requestedLevels || maxLevels < requestedLevels)
        return maxLevels;
    else
        return requestedLevels;
}

unsigned Texture::CheckMaxLevels(int width, int height, int depth, unsigned requestedLevels)
{
    unsigned maxLevels = 1;
    while (width > 1 || height > 1 || depth > 1)
    {
        ++maxLevels;
        width = width > 1 ? (width >> 1u) : 1;
        height = height > 1 ? (height >> 1u) : 1;
        depth = depth > 1 ? (depth >> 1u) : 1;
    }

    if (!requestedLevels || maxLevels < requestedLevels)
        return maxLevels;
    else
        return requestedLevels;
}

// 检查是否已超出纹理内存预算。在这种情况下释放未使用的材质以释放纹理引用。
void Texture::CheckTextureBudget(StringHash type)
{
    auto* cache = GetSubsystem<ResourceCache>();
    unsigned long long textureBudget = cache->GetMemoryBudget(type);
    unsigned long long textureUse = cache->GetMemoryUse(type);
    if (!textureBudget)
        return;

    // If textures are over the budget, they likely can not be freed directly as materials still refer to them.
    // Therefore free unused materials first
    if (textureUse > textureBudget)
        cache->ReleaseResources(Material::GetTypeStatic());
}

}
