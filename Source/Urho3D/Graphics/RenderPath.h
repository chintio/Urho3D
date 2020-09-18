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

// 场景渲染和视口上的任何后处理都由其RenderPath对象定义，该对象既可以从XML文件中读取，也可以通过编程方式创建。
// 渲染路径由rendertarget定义和命令组成。执行这些命令以生成渲染结果。每个命令要么输出到目标rendertarget& viewport（如果省略输出定义，则默认），要么输出一个命名的rendertargets。MRT（Multiple Render Targets）也可以输出。如果rendertarget是立方体贴图，也可以指定要渲染到的面（0 - 5）。
// rendertarget的大小可以是绝对大小，也可以是目标视口大小的乘或除。乘数或除数不必是整数。此外，可以将rendertarget声明为“persistent”，以便它不会与相同大小和格式的其他rendertarget混合，并且可以假定其内容也可用于后续帧。
// 请注意，如果已经在代码中创建了命名的rendertarget纹理，并使用AddManualResource（）将其存储到资源缓存中，则可以直接将其用作输出（通过引用其名称），而无需为其定义rendertarget。

/* 可用命令包括：
- clear: 清除任何颜色、深度和模板。“颜色清除”可以选择使用远剪裁距离内可见区域的雾颜色。
- scenepass: 渲染其材质“material technology”包含指定过程（pass）的场景对象。将使用状态排序从前到后顺序，或使用无状态排序从后到前顺序。
            对于延迟渲染，可以选择将对象光照掩码(lightmasks)标记到模板缓冲区。如果顶点光具有必要的着色器组合，则可以在过程(pass)中对其进行处理。
            过程的全局纹理可以绑定到自由纹理单元；这些单元可以是视口、命名的rendertarget或用其路径名标识的纹理资源。
- quad: 使用指定的着色器渲染一个视口大小的四边形。可以选择指定混合模式（默认值为replace）。
- forwardlights: 为具有指定过程名称的不透明对象渲染每像素前向光照。阴影贴图也会根据需要进行渲染。
- lightvolumes: 使用指定的着色器渲染延迟的光体积。G-buffer纹理可以根据需要绑定。
- renderui: 将UI渲染到rendertarget中。使用此选项将跳过对backbuffer的默认UI渲染。
- sendevent: 使用指定的字符串参数（“event name”）发送事件。这可用于在renderpath执行过程中调用自定义代码，通常是自定义低级别渲染。
*/

// Scenepass、quad、forwardlights和lightvolumes命令都允许“命令全局”（command-global ）的着色器编译宏、着色器参数和纹理。例如，在延迟渲染中，lightvolumes命令将绑定G缓冲区纹理以能够计算照明。请注意，当绑定命令全局纹理时，这些纹理（为了优化）只在命令开头绑定一次。如果纹理绑定被对象的材质覆盖，它将“丢失”，直到命令结束。因此，“全局纹理”命令应使用材质不使用的单位。
// 请注意，renderpath中只存在一个forwardlights或一个lightvolumes命令是合法的。
// renderpath可以从主XML文件加载（调用Load（）），然后可以通过调用Append（）将其他XML文件（例如每个后处理效果（post-processing effect）对应一个）附加到该文件中。通过调用SetEnabled（）来打开或关闭后处理效果，可以启用或禁用Rendertargets和命令。为了帮助实现这一点，可以通过标记名来标识二者，例如bloom效果对其所有rendertargets和命令使用标记“bloom”。
// 在同一个命令期间，向目标视口（viewport）写入数据并从中采样（sample）是合法的：将自动生成其内容的乒乓副本（pingpong copies）。如果视口（viewport）启用了“硬件多重采样”，则在采样之前，多采样的backbuffer将被解析为纹理。
// 有关renderpath定义的示例，请参见bin/CoreData/renderpaths目录中的前向（forward）、延迟（deferred）和灯光预处理（light pre-pass）渲染路径，以及bin/Data/postprocess目录中的后处理renderpath定义。

// 深度模板处理和读取场景深度（Depth-stencil handling and reading scene depth）
// 通常需要的深度模具表面（depth-stencil surfaces）会在执行渲染路径时自动分配。
// 特殊的“lineardepth”（同义词“depth”）格式用于在延迟渲染中存储场景深度。它不是实际的硬件深度模具纹理，而是32位单通道（R）浮点rendertarget。（在OpenGL2上它是RGBA，因为所有颜色缓冲区都必须是相同格式的限制。着色器包含文件Samplers.glsl在bin/CoreData/Shaders/GLSL中，提供了将线性深度编码和解码为RGB的函数。）
// 手动将深度写入rendertarget，同时使用不可读的深度模板表面可确保最佳兼容性，并防止同时使用深度测试和手动深度采样的任何冲突。
// 还可以定义可读的硬件深度纹理，并指示渲染路径使用它。必须首先使用函数GetReadableDepthSupport（）检查此函数的可用性。在Direct3D9上，这将使用INTZ“hack”格式。若要定义可读深度模具纹理，请使用格式“readabledepth”（同义词“hwdepth”）并通过使用渲染路径命令中的“depthstencil”属性将其设置为深度模具。请注意，必须在每个要使用它的命令中设置它，否则将使用自动分配的深度模具。还请注意，模板通道的存在并不能保证，因此通常由渲染器和视图类（Renderer & View classes）使用的灯光的模板掩码优化将被禁用。
// 在仅深度渲染过程的特殊情况下，可以直接将可读深度纹理设置为“output”，而根本不需要指定“depthstencil”属性。
// 填充可读的深度纹理后，可以在任何后续命令中将其绑定到纹理单元。像素着色器应使用ReconstructDepth（）辅助函数从非线性硬件深度值重建0 - 1之间的线性深度值。当可读深度纹理被绑定进行采样时，深度写入将自动禁用，因为修改和采样深度都将是未定义的。
// 有一个可读硬件深度的渲染路径示例：bin/CoreData/RenderPaths/ForwardHWDepth.xml
//     渲染路径首先分配与目标视口大小相同的可读深度模具纹理，清除其深度，然后向其渲染仅深度的过程。接下来，目标颜色rendertarget将正常清除，而可读的深度纹理将用作该命令和所有后续命令的深度模板。深度渲染过程之后的任何命令现在都可以将深度纹理绑定到采样单元，例如平滑粒子或SSAO效果。
//     ForwardDepth.xml“渲染路径”（render path）执行相同的操作，但使用线性深度渲染目标而不是硬件深度纹理。其优点是兼容性更好（保证在不检查GetReadableDepthSupport（）的情况下工作），但性能较差，因为它将执行额外的完整场景渲染过程。

// 软粒子渲染（Soft particles rendering）
// 软粒子渲染是利用场景深度读取的一个实例。暴露可读深度的默认renderpaths在alpha过程中绑定深度纹理。当包含SOFTPARTICLES着色器编译宏定义时，UnlitParticle&LitParticle着色器将使用此选项。在Bin/CoreData/technologies中名称中包含“Soft”的粒子技术使用此定义。请注意，它们期望一个可读的深度，并且不能与普通的正向renderpath一起工作！
// 软粒子可以用两种截然不同的方法实现：“收缩”和“膨胀”（"shrinking" and "expanding"）。在“收缩方法”（默认）中，“深度测试”（depth test）处于启用状态，当粒子几何体接近实心几何体时，“软粒子着色器”（soft particle shader）开始降低粒子不透明度。在“膨胀方法”中，粒子应禁用“深度测试”（depth test），当粒子几何体超出实体几何体时，着色器将开始增加粒子不透明度。
// 有关展开模式，请参见“SoftExpand”粒子技术系列。它们的缺点是，由于不能使用硬件深度测试，性能可能会降低。
// 最后请注意SoftParticleFadeScale着色器参数，该参数用于控制渐变生效的距离。这在使用软粒子的示例材质中定义(SmokeSoft.xml& LitSmokeSoft.xml)

// 前向光照特殊注意事项（Forward lighting special considerations）
// 另外，可以指定完全自定义的场景渲染过程，但要记住与前向照明相关的一些事项：
//     不透明的基过程必须用元数据“base”标记。当正向照明逻辑执行lit base pass优化时，它将搜索前缀为“lit”的过程，即，如果自定义不透明基本过程称为“custombase”，则相应的lit基本过程将为“litcustombase”。
//     透明基过程必须用元数据“alpha”标记。对于照亮的透明对象，前向照明逻辑将查找前面带有单词“lit”的过程，即如果自定义alpha基过程称为“customalpha”，则相应的照亮过程为“litcustomalpha”。lit drawcalls将与透明的基本过程交错，scenepass命令应该启用从后到前的排序。
//     如果前向照明和延迟照明混合使用，则G-buffer写入过程必须使用元数据“gbuffer”进行标记，以防止几何体也使用前向灯光进行双重照明。
//     记住将照明模式（逐顶点/逐像素）标记到定义自定义过程的技术中，因为只有已知的默认过程才能自动猜测照明模式。
//     如果需要单独的不透明仅环境光基础过程，则forwardlights命令可以选择性地禁用lit base pass优化，而不必接触材质技术。默认情况下启用优化。

// 后处理效果特别注意事项（Post-processing effects special considerations）
// 后处理效果通常通过使用quad命令来实现。使用与视口rendertarget尺寸不同的中间rendertarget时，通常需要在着色器中引用其（inverse）大小和Direct3D9的半像素偏移。将自动尝试为命名rendertargets指定这些着色器统一值（uniforms）。例如，请查看bloom后处理着色器：因为有一个名为BlurH的rendertarget，renderpath中的每个quad命令都将尝试设置着色器uniforms cBlurHInvSize和cBlurHOffsets（两个Vector2）。请注意，设置着色器uniforms不区分大小写。
// 在OpenGL后处理着色器中，对采样rendertarget纹理和常规纹理资源的区分是很重要的，因为中间rendertarget（例如G-buffer）可能是垂直反转的。使用GetScreenPos（）或GetQuadtextCoord（）函数从裁剪（clip）坐标获取rendertarget UV坐标；这会自动考虑翻转。要对常规纹理进行采样，请使用GetQuadTexCoordNoFlip（）函数，该函数需要世界坐标而不是裁剪（clip）坐标。

// 多重采样rendertargets（Multisampled rendertargets）
// Texture2D和TextureCube支持多重采样。在定义维度和格式时，可以通过SetSize（）函数以编程方式启用多重采样。也可以在renderpath的rendertarget定义中设置多重采样。
// 通常的操作是，多重采样rendertarget将在作为纹理采样之前自动解析为1-sample。这由autoResolve参数表示，其默认值为true。在OpenGL（支持时）和Direct3D11上，还可以通过定义多重采样采样器和使用专用函数（OpenGL上的texelFetch，Direct3D11上的Texture2DMS.Load）访问着色器代码中Texture2D的单个采样。在这种情况下，“autoResolve”参数应该设置为false。请注意，对于立方体纹理或使用Direct3D9时，无法访问单个采样。
// 通过访问多重采样G-buffer纹理的单个采样，可以实现延迟MSAA渲染器。这有一些性能考虑因素 / 复杂性（当不在三角形边上时，应避免运行每个采样的照明计算），并且默认情况下不会实现。

}
