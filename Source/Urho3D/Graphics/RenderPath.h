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

/// \file

#pragma once

#include "../Container/Ptr.h"
#include "../Container/RefCounted.h"
#include "../Graphics/GraphicsDefs.h"
#include "../Math/Color.h"
#include "../Math/Vector4.h"

namespace Urho3D
{

class XMLElement;
class XMLFile;

/// Rendering path command types.
enum RenderCommandType
{
    CMD_NONE = 0,
    CMD_CLEAR,
    CMD_SCENEPASS,
    CMD_QUAD,
    CMD_FORWARDLIGHTS,
    CMD_LIGHTVOLUMES,
    CMD_RENDERUI,
    CMD_SENDEVENT
};

/// Rendering path sorting modes.
enum RenderCommandSortMode
{
    SORT_FRONTTOBACK = 0,
    SORT_BACKTOFRONT
};

/// Rendertarget size mode.
enum RenderTargetSizeMode
{
    SIZE_ABSOLUTE = 0,
    SIZE_VIEWPORTDIVISOR,
    SIZE_VIEWPORTMULTIPLIER
};

/// Rendertarget definition.
struct URHO3D_API RenderTargetInfo
{
    /// Read from an XML element.
    void Load(const XMLElement& element);

    /// Name.
    String name_;
    /// Tag name.
    String tag_;
    /// Texture format.
    unsigned format_{};
    /// Absolute size or multiplier.
    Vector2 size_;
    /// Size mode.
    RenderTargetSizeMode sizeMode_{SIZE_ABSOLUTE};
    /// Multisampling level (1 = no multisampling).
    int multiSample_{1};
    /// Multisampling autoresolve flag.
    bool autoResolve_{true};
    /// Enabled flag.
    bool enabled_{true};
    /// Cube map flag.
    bool cubemap_{};
    /// Filtering flag.
    bool filtered_{};
    /// sRGB sampling/writing mode flag.
    bool sRGB_{};
    /// Should be persistent and not shared/reused between other buffers of same size.
    bool persistent_{};
};

/// Rendering path command.
struct URHO3D_API RenderPathCommand
{
    /// Read from an XML element.
    void Load(const XMLElement& element);
    /// Set a texture resource name. Can also refer to a rendertarget defined in the rendering path.
    void SetTextureName(TextureUnit unit, const String& name);
    /// Set a shader parameter.
    void SetShaderParameter(const String& name, const Variant& value);
    /// Remove a shader parameter.
    void RemoveShaderParameter(const String& name);
    /// Set number of output rendertargets.
    void SetNumOutputs(unsigned num);
    /// Set output rendertarget name and face index for cube maps.
    void SetOutput(unsigned index, const String& name, CubeMapFace face = FACE_POSITIVE_X);
    /// Set output rendertarget name.
    void SetOutputName(unsigned index, const String& name);
    /// Set output rendertarget face index for cube maps.
    void SetOutputFace(unsigned index, CubeMapFace face);
    /// Set depth-stencil output name. When empty, will assign a depth-stencil buffer automatically.
    void SetDepthStencilName(const String& name);

    /// Return texture resource name.
    const String& GetTextureName(TextureUnit unit) const;
    /// Return shader parameter.
    const Variant& GetShaderParameter(const String& name) const;

    /// Return number of output rendertargets.
    unsigned GetNumOutputs() const { return outputs_.Size(); }

    /// Return output rendertarget name.
    const String& GetOutputName(unsigned index) const;
    /// Return output rendertarget face index.
    CubeMapFace GetOutputFace(unsigned index) const;

    /// Return depth-stencil output name.
    const String& GetDepthStencilName() const { return depthStencilName_; }

    /// Tag name.
    String tag_;
    /// Command type.
    RenderCommandType type_{};
    /// Sorting mode.
    RenderCommandSortMode sortMode_{}; // command type="scenepass"时的排序模式，缺省值SORT_FRONTTOBACK，通常在alpha相关的pass中指定SORT_BACKTOFRONT
    /// Scene pass name.
    String pass_;
    /// Scene pass index. Filled by View.
    unsigned passIndex_{};
    /// Command/pass metadata.
    String metadata_;
    /// Vertex shader name.
    String vertexShaderName_;
    /// Pixel shader name.
    String pixelShaderName_;
    /// Vertex shader defines.
    String vertexShaderDefines_;
    /// Pixel shader defines.
    String pixelShaderDefines_;
    /// Textures.
    String textureNames_[MAX_TEXTURE_UNITS]; // 指定使用的纹理，Graphics::SetTexture
    /// %Shader parameters.
    HashMap<StringHash, Variant> shaderParameters_;
    /// Output rendertarget names and faces.
    Vector<Pair<String, CubeMapFace> > outputs_; // 通过属性或者子项定义的渲染目标，缺省值<"viewport",FACE_POSITIVE_X>，Graphics::SetRenderTarget
    /// Depth-stencil output name.
    String depthStencilName_;
    /// Clear flags. Affects clear command only.
    ClearTargetFlags clearFlags_{};
    /// Clear color. Affects clear command only.
    Color clearColor_;
    /// Clear depth. Affects clear command only.
    float clearDepth_{};
    /// Clear stencil value. Affects clear command only.
    unsigned clearStencil_{};
    /// Blend mode. Affects quad command only.
    BlendMode blendMode_{BLEND_REPLACE};
    /// Enabled flag.
    bool enabled_{true};
    /// Use fog color for clearing.
    bool useFogColor_{};
    /// Mark to stencil flag.
    bool markToStencil_{}; // command type="scenepass"时是否标志到模板，默认值false，通常在pass="deferred"中指定为true
    /// Use lit base pass optimization for forward per-pixel lights.
    bool useLitBase_{true};
    /// Vertex lights flag.
    bool vertexLights_{}; // command type="scenepass"时的顶点光标志，默认值false，在需要时指定
    /// Event name.
    String eventName_;
};

/// Rendering path definition. A sequence of commands (e.g. clear screen, draw objects with specific pass) that yields the scene rendering result.
class URHO3D_API RenderPath : public RefCounted
{
public:
    /// Construct.
    RenderPath();
    /// Destruct.
    ~RenderPath() override;

    /// Clone the rendering path.
    SharedPtr<RenderPath> Clone();
    /// Clear existing data and load from an XML file. Return true if successful.
    bool Load(XMLFile* file);
    /// Append data from an XML file. Return true if successful.
    bool Append(XMLFile* file);
    /// Enable/disable commands and rendertargets by tag.
    void SetEnabled(const String& tag, bool active);
    /// Return true of any of render targets or commands with specified tag are enabled.
    bool IsEnabled(const String& tag) const;
    /// Return true if renderpath or command with given tag exists.
    bool IsAdded(const String& tag) const;
    /// Toggle enabled state of commands and rendertargets by tag.
    void ToggleEnabled(const String& tag);
    /// Assign rendertarget at index.
    void SetRenderTarget(unsigned index, const RenderTargetInfo& info);
    /// Add a rendertarget.
    void AddRenderTarget(const RenderTargetInfo& info);
    /// Remove a rendertarget by index.
    void RemoveRenderTarget(unsigned index);
    /// Remove a rendertarget by name.
    void RemoveRenderTarget(const String& name);
    /// Remove rendertargets by tag name.
    void RemoveRenderTargets(const String& tag);
    /// Assign command at index.
    void SetCommand(unsigned index, const RenderPathCommand& command);
    /// Add a command to the end of the list.
    void AddCommand(const RenderPathCommand& command);
    /// Insert a command at a position.
    void InsertCommand(unsigned index, const RenderPathCommand& command);
    /// Remove a command by index.
    void RemoveCommand(unsigned index);
    /// Remove commands by tag name.
    void RemoveCommands(const String& tag);
    /// Set a shader parameter in all commands that define it.
    void SetShaderParameter(const String& name, const Variant& value);

    /// Return number of rendertargets.
    unsigned GetNumRenderTargets() const { return renderTargets_.Size(); }

    /// Return number of commands.
    unsigned GetNumCommands() const { return commands_.Size(); }

    /// Return command at index, or null if does not exist.
    RenderPathCommand* GetCommand(unsigned index) { return index < commands_.Size() ? &commands_[index] : nullptr; }

    /// Return a shader parameter (first appearance in any command.)
    const Variant& GetShaderParameter(const String& name) const;

    /// Rendertargets.
    Vector<RenderTargetInfo> renderTargets_;
    /// Rendering commands.
    Vector<RenderPathCommand> commands_;
};

/*RenderPath
- clear: 清除任何颜色、深度和模板。“颜色清除”可以选择使用远剪裁距离内可见区域的雾颜色。
- scenepass: 渲染其材质“material technology”包含指定过程的场景对象。将使用状态排序从前到后顺序，或使用无状态排序从后到前顺序。
            对于延迟渲染，可以选择将对象光照掩码(lightmasks)标记到模板缓冲区。如果顶点光具有必要的着色器组合，则可以在过程(pass)中对其进行处理。
            过程的全局纹理可以绑定到自由纹理单元；这些单元可以是视口、命名的rendertarget或用其路径名标识的纹理资源。
- quad: 使用指定的着色器渲染一个视口大小的四边形。可以选择指定混合模式（默认值为replace）。
- forwardlights: 为具有指定过程名称的不透明对象渲染每像素前向光照。阴影贴图也会根据需要进行渲染。
- lightvolumes: 使用指定的着色器渲染延迟的光体积。G-buffer纹理可以根据需要绑定。
- renderui: 将UI渲染到rendertarget中。使用此选项将跳过对backbuffer的默认UI渲染。
- sendevent: 使用指定的字符串参数（“event name”）发送事件。这可用于在renderpath执行过程中调用自定义代码，通常是自定义低级别渲染。
*/

}
