//
// Copyright (c) 2008-2016 the Urho3D project.
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

#include "../Core/Context.h"
#include "../IO/Log.h"
#include "../Resource/ResourceCache.h"
#include "../Resource/XMLFile.h"
#include "../UI/Button.h"
#include "../UI/MessageBox.h"
#include "../UI/Text.h"
#include "../UI/UI.h"
#include "../UI/UIEvents.h"
#include "../UI/Window.h"

namespace Urho3D
{

MessageBox::MessageBox(Context* context, const String& messageString, const String& titleString, XMLFile* layoutFile,
    XMLFile* styleFile) :
    UIElement(context),
    titleText_(0),
    messageText_(0),
    okButton_(0)
{
    // If layout file is not given, use the default message box layout
    if (!layoutFile)
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        layoutFile = cache->GetResource<XMLFile>("UI/MessageBox.xml");
        if (!layoutFile)    // Error is already logged
            return;         // Note: windowless MessageBox should not be used!
    }

    // MessageBox itself doesn't render anything. Add self to UI root for lifetime management
    UI* ui = GetSubsystem<UI>();
    window_ = ui->LoadLayout(layoutFile, styleFile);
    if (!window_)   // Error is already logged
        return;

    AddChild(window_);
    UIElement* root = ui->GetRoot();
    // Add self to UI hierarchy for lifetime management
    root->AddChild(this);

    // Set the title and message strings if they are given
    titleText_ = dynamic_cast<Text*>(window_->GetChild("TitleText", true));
    if (titleText_ && !titleString.Empty())
        titleText_->SetText(titleString);
    messageText_ = dynamic_cast<Text*>(window_->GetChild("MessageText", true));
    if (messageText_ && !messageString.Empty())
        messageText_->SetText(messageString);

    // Center window after the message is set
    Window* window = dynamic_cast<Window*>(window_.Get());
    if (window)
    {
        const IntVector2& size = window->GetSize();
        window->SetPosition((root->GetWidth() - size.x_) / 2, (root->GetHeight() - size.y_) / 2);
        window->SetModal(true);
        SubscribeToEvent(window, E_MODALCHANGED, URHO3D_HANDLER(MessageBox, HandleMessageAcknowledged));
    }

    // Bind the buttons (if any in the loaded UI layout) to event handlers
    okButton_ = dynamic_cast<Button*>(window_->GetChild("OkButton", true));
    if (okButton_)
    {
        ui->SetFocusElement(okButton_);
        SubscribeToEvent(okButton_, E_RELEASED, URHO3D_HANDLER(MessageBox, HandleMessageAcknowledged));
    }
    Button* cancelButton = dynamic_cast<Button*>(window_->GetChild("CancelButton", true));
    if (cancelButton)
        SubscribeToEvent(cancelButton, E_RELEASED, URHO3D_HANDLER(MessageBox, HandleMessageAcknowledged));
    Button* closeButton = dynamic_cast<Button*>(window_->GetChild("CloseButton", true));
    if (closeButton)
        SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(MessageBox, HandleMessageAcknowledged));
}

MessageBox::~MessageBox()
{
    RemoveWindow();
}

void MessageBox::RegisterObject(Context* context)
{
    context->RegisterFactory<MessageBox>();
}

void MessageBox::SetTitle(const String& text)
{
    if (titleText_)
        titleText_->SetText(text);
}

void MessageBox::SetMessage(const String& text)
{
    if (messageText_)
        messageText_->SetText(text);
}

const String& MessageBox::GetTitle() const
{
    return titleText_ ? titleText_->GetText() : String::EMPTY;
}

const String& MessageBox::GetMessage() const
{
    return messageText_ ? messageText_->GetText() : String::EMPTY;
}

void MessageBox::HandleMessageAcknowledged(StringHash eventType, VariantMap& eventData)
{
    using namespace MessageACK;

    VariantMap& newEventData = GetEventDataMap();
    newEventData[P_OK] = eventData[Released::P_ELEMENT] == okButton_;
    SendEvent(E_MESSAGEACK, newEventData);

    // Remove the modal window now
    RemoveWindow();

    // Remove self from UI hierarchy. This should cause destruction of self in case no other strong refs exist
    Remove();
}

void MessageBox::RemoveWindow()
{
    if (window_)
    {
        Window* window = dynamic_cast<Window*>(window_.Get());
        if (window)
            window->SetModal(false);
        RemoveChild(window_);
        window_.Reset();
    }
}

}
